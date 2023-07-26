
mkdir -p build
mkdir -p build/docker

cp -r include build/docker/.
cp -r src build/docker/.
cp -r libs build/docker/.
cp CMakeLists.txt build/docker/.

sudo docker build . -f docker/Dockerfile.gcc-4.8 -t build:gcc-4.8
sudo docker build ./build -f docker/Dockerfile.build.gcc-4.8
