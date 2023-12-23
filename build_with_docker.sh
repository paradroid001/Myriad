#!/usr/bin/bash
docker run -it --rm --mount type=bind,source="$(pwd)",target=/myriad myriad_build make PROJECT=Engine BUILD_MODE=Debug
docker run -it --rm --mount type=bind,source="$(pwd)",target=/myriad myriad_build make PROJECT=Test BUILD_MODE=Debug