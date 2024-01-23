
mkdir -p build
mkdir -p build/docker

cp -r generator build/docker/.
cp -r interface build/docker/.
cp -r asn.1 build/docker/.
cp -r examples build/docker/.
cp -r receiver build/docker/.
cp -r libs build/docker/.
cp CMakeLists.txt build/docker/.

docker build . -f docker/Dockerfile.gcc-4.8 -t build:gcc-4.8
docker build build -f docker/Dockerfile.build.gcc-4.8
