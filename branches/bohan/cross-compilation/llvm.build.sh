#! /bin/sh

set -x &&

cd $(dirname $0)/llvm &&

svn up &&

prefix=$(pwd)/++install &&

mkdir -p ++build &&
cd ++build &&

cmake -G Ninja -DCMAKE_INSTALL_PREFIX=$prefix -DCMAKE_BUILD_TYPE=Release .. &&
cmake --build . &&
cmake --build . --target install -- -j8

