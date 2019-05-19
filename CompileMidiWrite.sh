#!/bin/bash
#clang++ -std=c++11 -stdlib=libc++ -lstdc++ -I/Users/Mike/Desktop/Codes/C_PicProcess/include GetPicFeature.cpp `pkg-config --cflags --libs opencv4` -v
clang++ -std=c++11 -stdlib=libc++ -lstdc++ -I/Users/Mike/Desktop/Codes/C_PicProcess/include GetPicFeature.cpp `pkg-config --cflags --libs opencv4` -o GetPicFeature
