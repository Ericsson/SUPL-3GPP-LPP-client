
set -e

mkdir -p build
mkdir -p build/docker

cp -r dependency build/docker/.
cp -r cmake build/docker/.
cp -r examples build/docker/.
cp -r external build/docker/.
cp CMakeLists.txt build/docker/.

docker build . -f docker/Dockerfile.gcc-4.8-ppa -t build:gcc-4.8 --network=host
docker build build -f docker/Dockerfile.build.gcc-4.8 --network=host
