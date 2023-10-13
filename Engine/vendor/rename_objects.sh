#!/bin/bash
for file in raylib/src/*.obj
do
  mv "$file" "${file/.obj/.o}"
done
