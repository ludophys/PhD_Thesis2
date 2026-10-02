#!/bin/bash

dec_levels=(4 6 8)

run_array=(2661 2662 2663 2664) #Xenon
#run_array=(3131 3132 3133 3134) #Argon
run_array_str=$(IFS=,; echo "${run_array[*]}")

echo "The selected run numbers are : $run_array_str"

gas="Xenon"

for dec_level in "${dec_levels[@]}"; do

wd_func=("haar") #"coif$dec_level" "db$dec_level" "sym$dec_level")
wd_func_array_str=$(IFS=,; echo "${wd_func[*]}")

python wavelet_function_study.py $run_array_str $wd_func_array_str $dec_level $gas
done
