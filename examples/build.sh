#!/bin/bash
basepath=$(cd `dirname $0`; pwd)

dir=$basepath/build
mkdir -p "$dir"
cd "$dir"
cmake ..
make -j8
