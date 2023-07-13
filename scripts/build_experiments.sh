#!/bin/bash

if (( $# > 0 ))
then
    exp_protocol=${1}
else
    exp_protocol="semi"
fi

cwd=$(pwd)

if (( $# > 1 ))
then
    if [ ${2} = "lan" ]
    then
        if [ ${exp_protocol} = "semi" ]
        then
            ./build_experiments.sh ${exp_protocol} \
            & ssh machine-2 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            & ssh machine-3 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            && fg && fg
            wait
        elif [ ${exp_protocol} = "mal" ]
        then
            ./build_experiments.sh ${exp_protocol} \
            & ssh machine-2 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            & ssh machine-3 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            & ssh machine-4 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            && fg && fg && fg
            wait
        fi

    elif [ ${2} = "wan" ]
    then
        if [ ${exp_protocol} = "semi" ]
        then
        ./build_experiments.sh ${exp_protocol} \
        & ssh machine-wan-2 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
        & ssh machine-wan-3 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
        && fg && fg
        wait
        elif [ ${exp_protocol} = "mal" ]
        then
            ./build_experiments.sh ${exp_protocol} \
            & ssh machine-wan-2 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            & ssh machine-wan-3 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            & ssh machine-wan-4 "cd ${cwd} && ./build_experiments.sh ${exp_protocol}" \
            && fg && fg && fg
            wait
        fi
    fi
else
    mkdir -p ../build && cd ../build && rm -rf *
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
    fi
fi