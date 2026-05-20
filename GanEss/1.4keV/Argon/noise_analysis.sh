#!/bin/bash

echo "Run number ?"
read run_nb

echo "What is the gas (Xe or Ar) ?"
read gas

echo "The selected gas is : $gas"

echo "The selected run number is : $run_nb"

echo "Number of starting event ?"
read evt_start

echo "The analysis starts at the event number : $evt_start"

echo "What is the wavelet function to use ?"
read wd_func

echo "The selected wavelet function is : $wd_func"

#python denoised.py $run_nb $evt_start $wd_func $gas
#python fluct_study.py $run_nb $evt_start $wd_func $gas
#python time_generation.py $run_nb $evt_start $wd_func $gas
python charge_study.py $run_nb $evt_start $wd_func $gas
#python try.py $run_nb $evt_start $wd_func