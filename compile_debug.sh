#!/bin/bash
##gcc main.c `sdl-config --cflags --libs` -lSDL_image
gcc main.c -g `sdl-config --cflags --libs`
echo done!
