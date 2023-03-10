/*****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005-2021                                           \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/

#ifndef MLSSURFACE_H
#define MLSSURFACE_H

#include "balltree.h"
#include <Eigen/Dense>
#include <iostream>
#include <vcg/math/matrix33.h>
#include <vcg/space/box3.h>
#include <vcg/complex/allocate.h>

namespace GaelMls {

template<typename MeshType>
void computeVertexRadius(MeshType& m, int nNeighbors = 16);

enum {
	MLS_OK,
	MLS_TOO_FAR,
	MLS_TOO_MANY_ITERS,
	MLS_NOT_SUPPORTED,

	MLS_DERIVATIVE_ACCURATE,
	MLS_DERIVATIVE_APPROX,
	MLS_DERIVATIVE_FINITEDIFF
};

template<typename MeshType>
class MlsSurface
{
public:
	typedef typename MeshType::ScalarType    Scalar;
	typedef vcg::Point3<Scalar>              VectorType;
	typedef vcg::Matrix33<Scalar>            MatrixType;
	typedef typename MeshType::VertContainer PointsType;

	MlsSurface(const MeshType& mesh) : mMesh(mesh)
	{
		mCachedQueryPointIsOK = false;

		mAABB = mesh.bbox;

		typename MeshType::template ConstPerVertexAttributeHandle<Scalar> h;
		h = vcg::tri::Allocator<MeshType>::template FindPerVertexAttribute<Scalar>(mMesh, "radius");
		assert(vcg::tri::Allocator<MeshType>::template IsValidHandle<Scalar>(mMesh, h));

		mFilterScale                = 4.0;
		mMaxNofProjectionIterations = 20;
		mProjectionAccuracy         = (Scalar) 1e-4;
		mBallTree                   = 0;
		mGradientHint               = MLS_DERIVATIVE_ACCURATE;
		mHessianHint                = MLS_DERIVATIVE_ACCURATE;

		mDomainMinNofNeighbors = 4;
		mDomainRadiusScale     = 2.;
		mDomainNormalScale     = 1.;
	}

	virtual ~MlsSurface() {}

	/** \returns the value of the reconstructed scalar field at point \a x */
	virtual Scalar potential(const VectorType& x, int* errorMask = 0) const = 0;

	/** \returns the gradient of the reconstructed scalar field at point \a x
	 *
	 * The method used to compute the gradient can be controlled with setGradientHint().
	 */
	virtual VectorType gradient(const VectorType& x, int* errorMask = 0) const = 0;

	/** \returns the hessian matrix of the reconstructed scalar field at point \a x
	 *
	 * The method used to compute the hessian matrix can be controlled with setHessianHint().
	 */
	virtual MatrixType hessian(const VectorType& x, int* errorMask = 0) const
	{
		if (errorMask)
			*errorMask = MLS_NOT_SUPPORTED;
		return MatrixType();
	}

	/** \returns the projection of point x onto the MLS surface, and optionally returns the normal
	 * in \a pNormal */
	virtual VectorType
	project(const VectorType& x, VectorType* pNormal = 0, int* errorMask = 0) const = 0;

	/** \returns whether \a x is inside the restricted surface definition domain */
	virtual bool isInDomain(const VectorType& x) const;

	/** \returns the mean curvature from the gradient vector and Hessian matrix.
	 */
	Scalar meanCurvature(const VectorType& gradient, const MatrixType& hessian) const;

	/** set the scale of the spatial filter */
	void setFilterScale(Scalar v);
	/** set the maximum number of iterations during the projection */
	void setMaxProjectionIters(int n);
	/** set the threshold factor to detect convergence of the iterations */
	void setProjectionAccuracy(Scalar v);

	/** set a hint on how to compute the gradient
	 *
	 * Possible values are MLS_DERIVATIVE_ACCURATE, MLS_DERIVATIVE_APPROX, MLS_DERIVATIVE_FINITEDIFF
	 */
	void setGradientHint(int h);

	/** set a hint on how to compute the hessian matrix
	 *
	 * Possible values are MLS_DERIVATIVE_ACCURATE, MLS_DERIVATIVE_APPROX, MLS_DERIVATIVE_FINITEDIFF
	 */
	void setHessianHint(int h);

	inline const MeshType& mesh() const { return mMesh; }
	/** a shortcut for mesh().vert */
	inline const PointsType& points() const { return mMesh.vert; }

	inline vcg::ConstDataWrapper<VectorType> positions() const
	{
		return vcg::ConstDataWrapper<VectorType>(
			&mMesh.vert[0].P(),
			mMesh.vert.size(),
			size_t(mMesh.vert[1].P().V()) - size_t(mMesh.vert[0].P().V()));
	}
	inline vcg::ConstDataWrapper<VectorType> normals() const
	{
		return vcg::ConstDataWrapper<VectorType>(
			&mMesh.vert[0].N(),
			mMesh.vert.size(),
			size_t(mMesh.vert[1].N().V()) - size_t(mMesh.vert[0].N().V()));
	}
	inline vcg::ConstDataWrapper<Scalar> radii() const
	{
		typename MeshType::template ConstPerVertexAttributeHandle<Scalar> h;
		h = vcg::tri::Allocator<MeshType>::template FindPerVertexAttribute<Scalar>(mMesh, "radius");
		assert(vcg::tri::Allocator<MeshType>::template IsValidHandle<Scalar>(mMesh, h));

		return vcg::ConstDataWrapper<Scalar>(
			&h[0],
			mMesh.vert.size(),
			size_t(&h[1]) - size_t(&h[0]));
	}
	const vcg::Box3<Scalar>& boundingBox() const { return mAABB; }

	static const Scalar InvalidValue() { return Scalar(12345679810.11121314151617); }

	//void computeVertexRaddi(const int nbNeighbors = 16);

protected:
	void computeNeighborhood(const VectorType& x, bool computeDerivatives) const;
	void requestSecondDerivatives() const;

	struct PointToPointSqDist
	{
		inline bool
		operator()(const VectorType& a, const VectorType& b, Scalar& refD2, VectorType& q) const
		{
			Scalar d2 = vcg::SquaredDistance(a, b);
			if (d2 > refD2)
				return false;

			refD2 = d2;
			q     = a;
			return true;
		}
	};

	class DummyObjectMarker
	{
	};

protected:
	const MeshType&   mMesh;
	vcg::Box3<Scalar> mAABB;
	int               mGradientHint;
	int               mHessianHint;

	BallTree<Scalar>* mBallTree;

	int    mMaxNofProjectionIterations;
	Scalar mFilterScale;
	Scalar mAveragePointSpacing;
	Scalar mProjectionAccuracy;

	int   mDomainMinNofNeighbors;
	float mDomainRadiusScale;
	float mDomainNormalScale;

	// cached values:
	mutable bool                    mCachedQueryPointIsOK;
	mutable VectorType              mCachedQueryPoint;
	mutable Neighborhood<Scalar>    mNeighborhood;
	mutable std::vector<Scalar>     mCachedWeights;
	mutable std::vector<Scalar>     mCachedWeightDerivatives;
	mutable std::vector<VectorType> mCachedWeightGradients;
	mutable std::vector<Scalar>     mCachedWeightSecondDerivatives;
};

} // namespace GaelMls

#include "mlssurface.tpp"

#endif // MLSSURFACE_H
