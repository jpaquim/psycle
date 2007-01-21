/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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

#include "ntestwindow.h"

#include <ngrs/app.h>
#include <ngrs/regexp.h>
#include <ngrs/runtime.h>
#include <ngrs/property.h>
#include <ngrs/label.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[])
{
  std::vector<int> a;
  for ( int i = 0; i < 10; i++)
    a.push_back(i);

  std::vector<int>::iterator it = a.begin();
  for ( ; it < a.end(); it++ ) {
    int& a = *it;
    std::cout << "value" << a << "memadress" << &a << std::endl;
  }

  it = find( a.begin(), a.end(), 5 );
  a.erase( it);
  it = a.begin();
  for ( ; it < a.end(); it++ ) {
    int& a = *it;
    std::cout << "value" << a << "memadress" << &a << std::endl;
  }

   return EXIT_SUCCESS;

  ngrs::App app;

  ngrs::Window* myMainWindow = new NTestWindow();
  app.setMainWindow( myMainWindow );
  app.run();

  return EXIT_SUCCESS;
}
