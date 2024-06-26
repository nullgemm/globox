#!/bin/bash

# example for a release build
# $ ./run.sh /scripts/build_appkit.sh release appkit software none osxcross
docker run \
	--privileged \
	--name globuf_container_appkit_osxcross_"$4" \
	-e AR=x86_64-apple-darwin20.2-ar \
	globuf_image_appkit_osxcross "$@" &>> log
