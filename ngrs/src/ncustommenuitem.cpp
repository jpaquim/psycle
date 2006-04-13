/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ncustommenuitem.h"

NCustomMenuItem::NCustomMenuItem()
 : NCustomItem()
{
  setTransparent(true);
  setBorder(0);
  iconSize_ = captionSize_ = shortCutSize_ = hintSize_ =0;
  mnemonic_ = '\0';
}


NCustomMenuItem::~NCustomMenuItem()
{
}

void NCustomMenuItem::setIconSize( int size )
{
  iconSize_ = size;
}

void NCustomMenuItem::setCaptionSize( int size )
{
  captionSize_ = size;
}

void NCustomMenuItem::setShortCutSize( int size )
{
  shortCutSize_ = size;
}

void NCustomMenuItem::setHintSize( int size )
{
  hintSize_ = size;
}

int NCustomMenuItem::maxIconSize( )
{
  return iconSize_;
}

int NCustomMenuItem::maxCaptionSize( )
{
  return captionSize_;
}

int NCustomMenuItem::maxShortCutSize( )
{
  return shortCutSize_;
}

int NCustomMenuItem::maxHintSize( )
{
  return hintSize_;
}

char NCustomMenuItem::mnemonic()
{
  return mnemonic_;
}

void NCustomMenuItem::setMnemonic(char c)
{
  mnemonic_ = c;
}

void NCustomMenuItem::add( NMenu * menu )
{
}

void NCustomMenuItem::add( NRuntime * comp )
{
  NVisualComponent::add(comp);
}


