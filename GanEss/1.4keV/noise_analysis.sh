#!/bin/bash
#run_array=("3135" "3136" "3137" "3138")

#Argon
#run_array=("3131" "3132" "3133" "3134")

#Xenon
#run_array=(2661 2662 2663 2664)
run_array=(2661)

for run_nb in "${run_array[@]}"; do

#Define the main path where the data is stored
#A folder "gas"/data_data_of_today will be created to store the results of the analysis
main_path="/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV"

gas="Xenon"
evt_start="0" #starting event inside a run (doing the entire run can produces crashes due to memory issues)
f_numb="200" #ending event number (starting event + number of events to analyze)
wd_func="haar" #wavelet function to use for the denoising (check the exact names on https://pywavelets.readthedocs.io/en/latest/ref/wavelets.html)
dec_level=4

echo "The main path is : $main_path"
echo "The selected gas is : $gas"
echo "The selected run number is : $run_nb"
echo "Number of file is : $f_numb"
echo "The analysis starts at the event number : $evt_start"
echo "The selected wavelet function is : $wd_func"
echo "The decomposition level is : $dec_level"

#In this section launch what you want
#Denoised is the starting point so it should always be launched in first
python denoised.py $run_nb $evt_start $wd_func $gas $f_numb $main_path $dec_level # Apply wavelet decomposition
#python fluct_wf.py $run_nb $evt_start $wd_func $gas $f_numb $main_path

python fluct_study.py $run_nb $evt_start $wd_func $gas $f_numb $main_path $dec_level # Calculate the fluctuations of the cumsum ratio
python time_generation.py $run_nb $evt_start $wd_func $gas $f_numb $main_path $dec_level # Calculate t07 and t03

#python charge_study.py $run_nb $evt_start $wd_func $gas $f_numb $main_path
#python charge_total.py $run_nb $evt_start $wd_func $gas $f_numb $main_path
done

#Here to make the plots
# If the files are already generated with the previous scripts, just launch this command
python making_plots_$gas.py $run_nb $evt_start $wd_func $gas $f_numb $main_path $dec_level # Apply the cuts and see the final plot
