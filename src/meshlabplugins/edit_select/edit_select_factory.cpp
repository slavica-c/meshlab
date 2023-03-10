/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005-2008                                           \/)\/    *
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

#include "edit_select_factory.h"
#include "edit_select.h"

EditSelectFactory::EditSelectFactory()
{
	editSelect = new QAction(QIcon(":/images/select_face.png"),"Select Faces in a rectangular region", this);
	editSelectConnected = new QAction(QIcon(":/images/select_face_connected.png"),"Select Connected Components in a region", this);
	editSelectVert = new QAction(QIcon(":/images/select_vertex.png"),"Select Vertices", this);
	editSelectArea = new QAction(QIcon(":/images/select_area.png"), "Select Faces/Vertices inside polyline area", this);

	actionList.push_back(editSelectVert);
	actionList.push_back(editSelect);
	actionList.push_back(editSelectConnected);
	actionList.push_back(editSelectArea);
	
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true); 	
}

QString EditSelectFactory::pluginName() const
{
	return "EditSelect";
}

//get the edit tool for the given action
EditTool* EditSelectFactory::getEditTool(const QAction *action)
{
	if(action == editSelect)
		return new EditSelectPlugin(EditSelectPlugin::SELECT_FACE_MODE);
	else if(action == editSelectConnected)
		return new EditSelectPlugin(EditSelectPlugin::SELECT_CONN_MODE);
	else if(action == editSelectVert)
		return new EditSelectPlugin(EditSelectPlugin::SELECT_VERT_MODE);
	else if (action == editSelectArea)
		return new EditSelectPlugin(EditSelectPlugin::SELECT_AREA_MODE);

	assert(0); //should never be asked for an action that isn't here
	return nullptr;
}

QString EditSelectFactory::getEditToolDescription(const QAction * /*a*/)
{
	return EditSelectPlugin::info();
}

MESHLAB_PLUGIN_NAME_EXPORTER(EditSelectFactory)
