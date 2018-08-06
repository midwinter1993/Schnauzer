#!/bin/bash

# Build sqlite3
tar -xzvf sqlite-snapshot-201708251543.tar.gz
mkdir -pv obj

cd obj
../sqlite-snapshot-201708251543/configure
make

cp sqlite3 ../sqlite3
cp .libs/libsqlite3.so.0.8.6 ../libsqlite3.so

cd ..
cp sqlite-snapshot-201708251543/sqlite3.h ./

### Build libhijack.so
make
