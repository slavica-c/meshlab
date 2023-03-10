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

#ifndef APSS_H
#define APSS_H

#include "mlssurface.h"

namespace GaelMls {

template<typename _MeshType>
class APSS : public MlsSurface<_MeshType>
{
	typedef MlsSurface<_MeshType> Base;

	typedef typename Base::Scalar     Scalar;
	typedef typename Base::VectorType VectorType;
	typedef typename Base::MatrixType MatrixType;
	typedef _MeshType                 MeshType;
	using Base::mAveragePointSpacing;
	using Base::mBallTree;
	using Base::mCachedQueryPoint;
	using Base::mCachedQueryPointIsOK;
	using Base::mCachedWeightDerivatives;
	using Base::mCachedWeightGradients;
	using Base::mCachedWeights;
	using Base::mCachedWeightSecondDerivatives;
	using Base::mFilterScale;
	using Base::mGradientHint;
	using Base::mMaxNofProjectionIterations;
	using Base::mMesh;
	using Base::mNeighborhood;
	using Base::mProjectionAccuracy;

	enum Status { ASS_SPHERE, ASS_PLANE, ASS_UNDETERMINED };

public:
	APSS(const MeshType& m) : Base(m) { mSphericalParameter = 1; }

	virtual Scalar     potential(const VectorType& x, int* errorMask = 0) const;
	virtual VectorType gradient(const VectorType& x, int* errorMask = 0) const;
	virtual MatrixType hessian(const VectorType& x, int* errorMask) const;
	virtual VectorType
	project(const VectorType& x, VectorType* pNormal = 0, int* errorMask = 0) const;

	/** \returns the approximation of the mean curvature obtained from the radius of the fitted
	 * sphere */
	virtual Scalar approxMeanCurvature(const VectorType& x, int* errorMask = 0) const;

	void setSphericalParameter(Scalar v);

protected:
	bool fit(const VectorType& x) const;
	bool mlsGradient(const VectorType& x, VectorType& grad) const;
	bool mlsHessian(const VectorType& x, MatrixType& hessian) const;

protected:
	Scalar mSphericalParameter;

	// use double precision anyway
	typedef double               LScalar;
	typedef vcg::Point3<LScalar> LVector;

	// cached algebraic sphere coefficients
	mutable LScalar uConstant;
	mutable LVector uLinear;
	mutable LScalar uQuad;

	mutable LVector mCenter;
	mutable LScalar mRadius;
	mutable Status  mStatus;

	mutable LVector mCachedSumP;
	mutable LVector mCachedSumN;
	mutable LScalar mCachedSumDotPP;
	mutable LScalar mCachedSumDotPN;
	mutable LScalar mCachedSumW;

	mutable LVector mCachedGradSumP[3];
	mutable LVector mCachedGradSumN[3];
	mutable LScalar mCachedGradSumDotPN[3];
	mutable LScalar mCachedGradSumDotPP[3];
	mutable LScalar mCachedGradSumW[3];

	mutable LScalar mCachedGradNume[3];
	mutable LScalar mCachedGradDeno[3];

	mutable LScalar mCachedGradUConstant[3];
	mutable LVector mCachedGradULinear[3];
	mutable LScalar mCachedGradUQuad[3];
};

} // namespace GaelMls

//#include "apss.tpp"

#endif
