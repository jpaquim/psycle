/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef NSTATUSBAR_H
#define NSTATUSBAR_H

#include "npanel.h"

class NCustomStatusItem;
class NTextStatusItem;
class NCustomStatusModel;

/**
@author Stefan Nattkemper
*/

// view for a ncustomstatus based data model

class NStatusBar : public NPanel
{
public:

    NStatusBar();

    ~NStatusBar();

    virtual void add(NCustomStatusItem* component);
    virtual void add(NCustomStatusItem* component, int align);
	virtual void add(NVisualComponent* component, int align);

	void setModel( NCustomStatusModel & model );
	NCustomStatusModel* model() const;

private:

	NCustomStatusModel* statusModel_;
	std::vector<NCustomStatusItem*> statusItems_;

	void onModelDataChange( const NCustomStatusModel & sender, unsigned int index );

};

#endif
