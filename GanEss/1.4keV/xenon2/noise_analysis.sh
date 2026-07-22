#!/bin/bash
run_array=("2661" "2662" "2663" "2664")

for run_nb in "${run_array[@]}"; do
    
gas="Xe"
evt_start="0"
f_numb="200"
wd_func="coif"

echo "The selected gas is : $gas"

echo "The selected run number is : $run_nb"

echo "Number of file is : $f_numb"

echo "The analysis starts at the event number : $evt_start"


echo "The selected wavelet function is : $wd_func"

python denoised.py $run_nb $evt_start $wd_func $gas $f_numb
python fluct_study.py $run_nb $evt_start $wd_func $gas $f_numb
python time_generation.py $run_nb $evt_start $wd_func $gas $f_numb
python charge_study.py $run_nb $evt_start $wd_func $gas $f_numb
#python charge_total.py $run_nb $evt_start $wd_func $gas $f_numb


done