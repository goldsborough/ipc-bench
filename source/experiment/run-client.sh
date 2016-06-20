#!/bin/zsh

DYLD_INSERT_LIBRARIES=$PWD/../tssx/libtssx-client.dylib \
DYLD_FORCE_FLAT_NAMESPACE=1 ./experiment-client -c $1 -s $2
