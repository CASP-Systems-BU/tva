#!/bin/bash

if (( $# > 0 ))
then
    exp_setup=${1}
else
    exp_setup="lan"
fi

if [ ${exp_setup} = "lan" ]
then
    ./build_experiments.sh semi lan
    ./energy.sh semi lan
    ./medical.sh semi lan
    ./cloud.sh semi lan

    ./build_experiments.sh mal lan
    ./energy.sh mal lan
    ./medical.sh mal lan
    ./cloud.sh mal lan
elif [ ${exp_setup} = "wan" ]
then
    ./build_experiments.sh semi wan
    ./energy.sh semi wan
    ./medical.sh semi wan
    ./cloud.sh semi wan

    ./build_experiments.sh mal wan
    ./energy.sh mal wan
    ./medical.sh mal wan
    ./cloud.sh mal wan
elif [ ${exp_setup} = "waldo" ]
then
    ./build_experiments.sh semi lan
    ./waldo_energy_query.sh semi lan
    ./waldo_table_equality.sh semi lan
    ./waldo_table_greater.sh semi lan

    ./build_experiments.sh mal lan
    ./waldo_energy_query.sh mal lan
    ./waldo_table_equality.sh mal lan
    ./waldo_table_greater.sh mal lan
fi