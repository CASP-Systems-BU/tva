#!/bin/bash

if (( $# < 2 ))
then
    echo "Usage: ${0} <exp_protocol> <exp_setting> <optional_just_build>"
fi

# Initial defaults
exp_name="micro_primitives"
exp_protocol="semi"
exp_setting="same"

source ./utils.sh
cd ../build
define_defaults_and_build $1 $2

if (($# < 3))
then
    for ROWS in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304
    do
        if [ ${exp_protocol} = "semi" ]
        then
            if [ ${exp_setting} = "same" ]
            then
                mpirun -np 3 ./${exp_name} 1 1 8192 $ROWS >> ../results/same/${exp_name}-semi-results.txt
            elif [ ${exp_setting} = "lan" ]
            then
                mpirun --host machine-1,machine-2,machine-3 -np 3 ./${exp_name} 16 1 8192 $ROWS >> ../results/lan/${exp_name}-semi-results.txt
            elif [ ${exp_setting} = "wan" ]
            then
                mpirun --host machine-wan-1,machine-wan-2,machine-wan-3 -np 3 ./${exp_name} 64 1 8192 $ROWS >> ../results/wan/${exp_name}-semi-results.txt
            fi
        elif [ ${exp_protocol} = "mal" ]
        then
            if [ ${exp_setting} = "same" ]
            then
                mpirun -np 4 ./${exp_name} 1 1 8192 $ROWS >> ../results/same/${exp_name}-mal-results.txt
            elif [ ${exp_setting} = "lan" ]
            then
                mpirun --host machine-1,machine-2,machine-3,machine-4 -np 4 ./${exp_name} 16 1 8192 $ROWS >> ../results/lan/${exp_name}-mal-results.txt
            elif [ ${exp_setting} = "wan" ]
            then
                mpirun --host machine-wan-1,machine-wan-2,machine-wan-3,machine-wan-4 -np 4 ./${exp_name} 16 1 8192 $ROWS >> ../results/wan/${exp_name}-mal-results.txt
            fi
        fi
    done
fi