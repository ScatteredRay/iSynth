#!/bin/sh

g++ -o isynth -framework Carbon -framework AudioUnit main.cpp audio.cpp audio_osx.cpp