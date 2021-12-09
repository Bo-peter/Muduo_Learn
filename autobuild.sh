#! /bin/bash

set -e

#如果没有build目录,创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build&&
    cmake ..&&
    make

cd ..

if [ ! -d /usr/include/minimuduo ]; then
    mkdir /usr/include/minimuduo
fi

for header in `ls *.h`
do
    cp ./$header /usr/include/minimuduo
done

cp `pwd`/lib/libminimuduo.so /usr/lib

ldconfig

