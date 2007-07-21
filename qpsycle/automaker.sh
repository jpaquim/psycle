#!/bin/bash
#
# automaker.sh
#
# Originally Created By Gravity_0
#
# Modifications by erodix
#
# Released Under GPL Licence
#
#

QMAKE='qmake-qt4'

echo "---"
echo "Welcome to Automaker"
echo "Qt is a registered trademark of Trolltech."
echo "---"
echo ""

QMAKE_BINARY=$( which $QMAKE )

if [ "$QMAKE_BINARY" == '' ]; then
  echo "FATAL ERROR:  $QMAKE executable not found."
  echo "Try installing Qt4 development packages."
  echo "Bailing out."
  exit 1
else
  echo "Using $QMAKE at $QMAKE_BINARY."
fi

echo "---"
  $QMAKE_BINARY
QMAKE_RETVAL="$?"
echo "---"

if [ $QMAKE_RETVAL == '0' ]; then
  echo "QMake complete."
else
  echo "QMake failed."
  echo "Bailing out."
  exit $QMAKE_RETVAL
fi

MAKE_BINARY=$( which make )

if [ "$MAKE_BINARY" == '' ]; then
  echo "You seem to be _not_ having make installed. Please install some development tools."
  echo "Bailing out."
  exit 0
else
  echo "Executing make at $MAKE_BINARY."
fi

echo "Running make."

echo "---"
  $MAKE_BINARY
MAKE_RETVAL="$?"
echo "---"


if [ $MAKE_RETVAL == '0' ]; then
  echo "Make complete."
else
  echo "Make failed."
fi

echo ""

exit $MAKE_RETVAL

