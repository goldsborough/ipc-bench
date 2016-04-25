#!/bin/sh -x

clang++ -O2 -fno-exceptions -fno-rtti -std=c++1z -o server server.cpp `pkg-config --cflags --libs dbus-1` &&
clang++ -O2 -fno-exceptions -fno-rtti -std=c++1z -o client client.cpp `pkg-config --cflags --libs dbus-1`

