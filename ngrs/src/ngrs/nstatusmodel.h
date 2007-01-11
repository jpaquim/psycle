/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#ifndef NSTATUSMODEL_H
#define NSTATUSMODEL_H

#include "sigslot.h"
#include <string>

// abstract interface for a statusbar textinfo model

class NCustomStatusModel {
public:
	
	NCustomStatusModel() {
	};

	virtual ~NCustomStatusModel() = 0 {
	}

	virtual void setText( const std::string & text ) = 0;
	virtual std::string text() const = 0;

	sigslot::signal1<const NCustomStatusModel&> changed;

};

class NStatusModel :  public NCustomStatusModel {
public:

    NStatusModel();

    ~NStatusModel();

	virtual void setText( const std::string & text );
    virtual std::string text() const;

private:

	std::string text_;

};


#endif