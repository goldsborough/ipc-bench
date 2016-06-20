#!/bin/zsh

DYLD_INSERT_LIBRARIES=$PWD/../tssx/libtssx-server.dylib \
DYLD_FORCE_FLAT_NAMESPACE=1 ./experiment-server -c $1 -s $2
