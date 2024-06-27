#!/bin/sh

gcc -Wall bmp24tobmp4bmp2.c -o bmp24tobmp4bmp2

gcc -Wall -Wno-unused-function -D_TOOL1_ fonttool.c -lgdi32 -o fonttool1

gcc -Wall -Wno-unused-function --static -D_TOOL2_ fonttool.c -lgdi32 -liconv -o fonttool2

gcc -Wall -D_TOOL1_ mergefont.c -o mergefont1

gcc -Wall -D_TOOL2_ mergefont.c -o mergefont2

strip --strip-unneeded *.exe
