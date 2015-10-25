#! /bin/sh

set -x &&

cd $(dirname $0)/llvm &&

svn up &&

prefix=$(pwd)/++install &&

mkdir -p ++build &&
cd ++build &&

cmake -G Ninja -DCMAKE_INSTALL_PREFIX=$prefix -DCMAKE_BUILD_TYPE=Release .. &&

jobs=$(nproc || getconf _NPROCESSORS_ONLN) &&

cmake --build . -- -j $jobs &&
cmake --build . --target install -- -j $jobs
