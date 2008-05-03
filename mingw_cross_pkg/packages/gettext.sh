#! /usr/bin/env bash

# gettext package recipe for MinGW cross pkg tool.
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2008-2008 Johan Boule <bohan@jabber.org>
#
# Based on MinGW cross compiling environment, version 1.4
# copyright 2007-2008 Volker Grabsch <vog@notjusthosting.com>
# copyright 2007-2008 Rocco Rutte <pdmef@gmx.net>
# copyright 2007-2008 Andreas Roever <roever@users.sf.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#---
#   gettext
#
#   http://www.gnu.org/software/gettext/
#---

set -ex

VERSION_gettext=0.17

case "$1" in

version)
    echo "$VERSION_gettext"
    ;;
    
new-version)
    VERSION=`
        wget -q -O- 'ftp://ftp.gnu.org/pub/gnu/gettext/' |
        $SED -n 's,.*gettext-\([0-9][^>]*\)\.tar.*,\1,p' |
        sort | tail -1`
    test -n "$VERSION"
    $SED "s,^VERSION_gettext=.*,VERSION_gettext=$VERSION," -i "$0"
    ;;

dep)
    #todo echo foo bar
    ;;
    
download)
    tar tfz "gettext-$VERSION_gettext.tar.gz" &>/dev/null ||
    wget -c "ftp://ftp.gnu.org/pub/gnu/gettext/gettext-$VERSION_gettext.tar.gz"
    ;;

unpack)
    tar xfvz "gettext-$VERSION_gettext.tar.gz"
    ;;
    
build)
    cd "gettext-$VERSION_gettext"
    cd gettext-runtime
    SHELL=bash ./configure \
        --host="$TARGET" \
        --disable-shared \
        --prefix="$PREFIX/$TARGET" \
        --enable-threads=win32
    $MAKE install -C intl SHELL=bash DESTDIR=$DESTDIR
    ;;

clean-build)
    rm -Rf "gettext-$VERSION_gettext"
    ;;

esac
