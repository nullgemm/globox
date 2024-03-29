#!/bin/bash

echo "please clean docker"

sudo rm -rf globox_bin_v* log
sudo docker rm globox_container_x11_gcc_software
sudo docker rm globox_container_x11_gcc_vulkan
sudo docker rm globox_container_x11_gcc_egl
sudo docker rm globox_container_x11_gcc_glx
sudo docker rmi globox_image_x11_gcc
sudo docker rmi alpine:edge

sudo ./build.sh

sudo ./run.sh /scripts/build_x11.sh release x11 software none native
sudo ./run.sh /scripts/build_x11.sh release x11 vulkan none native
sudo ./run.sh /scripts/build_x11.sh release x11 egl none native
sudo ./run.sh /scripts/build_x11.sh release x11 glx none native

sudo ./artifact.sh software
sudo ./artifact.sh vulkan
sudo ./artifact.sh egl
sudo ./artifact.sh glx

sudo chown -R $(id -un):$(id -gn) globox_bin_v*
