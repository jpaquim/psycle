#! /usr/bin/m4

##############################################################################
#
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule
# Copyright (C) Alexandre Duret-Lutz <duret_g@epita.fr> and Guiodo Draheim <guidod@gmx.de>
#
# m4 macros for autoconf included by ./configure.ac
# \meta hook ./configure.ac
# \meta generic
# \meta standard posix
#
##############################################################################



###############################################################################################
#
# AX_PREFIX_CONFIG_H
#
# Origin: http://gnu.org/software/ac-archive/htmldoc/ax_prefix_config_h.html
#
# Synopsis: AX_PREFIX_CONFIG_H [(OUTPUT-HEADER [,PREFIX [,ORIG-HEADER]])]
#
# Description:
# takes the usual config.h generated header file;
# looks for each of the generated "#define SOMEDEF" lines, and prefixes the defined name
# (ie. makes it "#define PREFIX_SOMEDEF".
# The result is written to the output config.header file.
# The PREFIX is converted to uppercase for the conversions.
#
# Version: 1.4 (last modified: 2003-11-04)
# Author: Guiodo Draheim <guidod@gmx.de>
#
# modified by bohan
# AC_REQUIRE([AC_CONFIG_HEADER]) -> AC_REQUIRE([AC_CONFIG_HEADERS])
# _DEF=$_UPP
# _UPP=${_UPP}_
# _LOW=${_LOW}_
#
###############################################################################################

AC_DEFUN([AX_PREFIX_CONFIG_H__MODIFIED],
	[
		AC_REQUIRE([AC_CONFIG_HEADERS])
		AC_CONFIG_COMMANDS(
			[ifelse($1,,$PACKAGE-config.h,$1)],
			[dnl
				AS_VAR_PUSHDEF([_OUT],[ac_prefix_conf_OUT])dnl
				AS_VAR_PUSHDEF([_DEF],[ac_prefix_conf_DEF])dnl
				AS_VAR_PUSHDEF([_PKG],[ac_prefix_conf_PKG])dnl
				AS_VAR_PUSHDEF([_LOW],[ac_prefix_conf_LOW])dnl
				AS_VAR_PUSHDEF([_UPP],[ac_prefix_conf_UPP])dnl
				AS_VAR_PUSHDEF([_INP],[ac_prefix_conf_INP])dnl
				m4_pushdef([_script],[conftest.prefix])dnl
				m4_pushdef([_symbol],[m4_cr_Letters[]m4_cr_digits[]_])dnl
				_OUT=`echo ifelse($1, , $PACKAGE-config.h, $1)`
				_DEF=`echo _$_OUT | sed -e "y:m4_cr_letters:m4_cr_LETTERS[]:" -e "s/@<:@^m4_cr_Letters@:>@/_/g"`
				_PKG=`echo ifelse($2, , $PACKAGE, $2)`
				_LOW=`echo _$_PKG | sed -e "y:m4_cr_LETTERS-:m4_cr_letters[]_:"`
				_UPP=`echo $_PKG | sed -e "y:m4_cr_letters-:m4_cr_LETTERS[]_:"  -e "/^@<:@m4_cr_digits@:>@/s/^/_/"`
				_DEF=$_UPP
				_UPP=${_UPP}_
				_LOW=${_LOW}_
				_INP=`echo "ifelse($3,,,$3)" | sed -e 's/ *//'`
				if test ".$_INP" = "."; then
				   for ac_file in : $CONFIG_HEADERS; do test "_$ac_file" = _: && continue
				     case "$ac_file" in
				        *.h) _INP=$ac_file ;;
				        *)
				     esac
				     test ".$_INP" != "." && break
				   done
				fi
				if test ".$_INP" = "."; then
				   case "$_OUT" in
				      */*) _INP=`basename "$_OUT"`
				      ;;
				      *-*) _INP=`echo "$_OUT" | sed -e "s/@<:@_symbol@:>@*-//"`
				      ;;
				      *) _INP=config.h
				      ;;
				   esac
				fi
				if test -z "$_PKG" ; then
				   AC_MSG_ERROR([no prefix for AX_PREFIX_PKG_CONFIG_H])
				else
				  if test ! -f "$_INP" ; then if test -f "$srcdir/$_INP" ; then
				     _INP="$srcdir/$_INP"
				  fi fi
				  AC_MSG_NOTICE(creating $_OUT - prefix ${_UPP}_ for $_INP defines)
				  
				  if test -f $_INP ; then
				    echo "s/@%:@undef  *\\(@<:@m4_cr_LETTERS[]_@:>@\\)/@%:@undef $_UPP""_\\1/" > $tmp/pconfig.sed
				    echo "s/@%:@undef  *\\(@<:@m4_cr_letters@:>@\\)/@%:@undef $_LOW""_\\1/" >> $tmp/pconfig.sed
				    echo "s/@%:@def[]ine  *\\(@<:@m4_cr_LETTERS[]_@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef $_UPP""_\\1 \\" >> $tmp/pconfig.sed
				    echo "@%:@def[]ine $_UPP""_\\1 \\2 \\" >> $tmp/pconfig.sed
				    echo "@%:@endif/" >> $tmp/pconfig.sed
				    echo "s/@%:@def[]ine  *\\(@<:@m4_cr_letters@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef $_LOW""_\\1 \\" >> $tmp/pconfig.sed
				    echo "@%:@define $_LOW""_\\1 \\2 \\" >> $tmp/pconfig.sed
				    echo "@%:@endif/" >> $tmp/pconfig.sed

				    # create _OUT output file
				    echo "@%:@pragma once" > $tmp/pconfig.h
#				    echo "@%:@ifndef $_DEF" >> $tmp/pconfig.h
#				    echo "@%:@def[]ine $_DEF 1" >> $tmp/pconfig.h
				    echo ' ' >> $tmp/pconfig.h
				    echo /'*' $_OUT. Generated automatically at end of configure. '*'/ >> $tmp/pconfig.h

				    # execute $tmp/pconfig.sed on _DEF input to create _OUT output file
				    sed -f $tmp/pconfig.sed $_INP >> $tmp/pconfig.h
				    rm -f $tmp/pconfig.sed

				    echo ' ' >> $tmp/pconfig.h
#				    echo '/* once:' $_DEF '*/' >> $tmp/pconfig.h
#				    echo "@%:@endif" >> $tmp/pconfig.h
				    if cmp -s $_OUT $tmp/pconfig.h 2>/dev/null; then
				      AC_MSG_NOTICE([$_OUT is unchanged])
				      rm -f $tmp/pconfig.h
				    else
				      ac_dir=`AS_DIRNAME(["$_OUT"])`
				      AS_MKDIR_P(["$ac_dir"])
				      rm -f "$_OUT"
				      mv $tmp/pconfig.h "$_OUT"
				    fi
				  else
				    AC_MSG_ERROR([input file $_INP does not exist - skip generating $_OUT])
				  fi
				  rm -f conftest.*
				fi
				m4_popdef([_symbol])dnl
				m4_popdef([_script])dnl
				AS_VAR_POPDEF([_INP])dnl
				AS_VAR_POPDEF([_UPP])dnl
				AS_VAR_POPDEF([_LOW])dnl
				AS_VAR_POPDEF([_PKG])dnl
				AS_VAR_POPDEF([_DEF])dnl
				AS_VAR_POPDEF([_OUT])dnl
			],
			[PACKAGE="$PACKAGE"]
		)
	]
)
