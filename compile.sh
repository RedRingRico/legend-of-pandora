#!/bin/bash
##gcc main.c `sdl-config --cflags --libs` -lSDL_image
gcc main.c `sdl-config --cflags --libs` -lSDL_mixer
echo done!
