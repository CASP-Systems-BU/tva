#!/bin/bash

cd ../build && rm -rf *
if (( $# > 0 ))
then
    exp_protocol=${1}
else
    exp_protocol="semi"
fi

if [ ${exp_protocol} = "semi" ]
then
    cmake ..
    make clean
    make
elif [ ${exp_protocol} = "mal" ]
then
    cmake .. -DPROTOCOL_VAR='-DUSE_FANTASTIC_FOUR'
    make clean
    make
    cmake ..
fi
