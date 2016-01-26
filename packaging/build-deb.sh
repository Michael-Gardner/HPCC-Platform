#!/usr/bin/env bash

function error()
{
    echo -e "\e[31mError:\e[39m $@"
    exit 1
}

version="6.0.0"
debbase="hpccsystems-platform-$version"

directorylist=( packaging/debian build-config.h.cmake build_utils clienttools CMakeLists.txt cmake_modules common dali deploy deployment docs ecl ecllibrary esp initfiles lib2 misc plugins roxie rtl services system testing thorlcr tools version.cmake VERSIONS LICENSE.txt )

if [[ -d build ]]; then
	rm -rf build
fi
mkdir -p "build/$debbase"
for directory in ${directorylist[@]}; do
	cp -r "../$directory" "build/$debbase"
done

pushd "build/$debbase"
debuild -i -uc -us
popd
