#!/bin/zsh

# Here we launch the geometry and generator analysis
# for each region

PIECES=("VESSEL_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "VESSEL_INNER_FLANGE1" "VESSEL_INNER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "CONE_ENDCAP" "COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2")
#PIECES=("PTFE_PANEL" "CROSSBAR" "RINGS_Z+" "RINGS_Z-" "CATHODE" "EL" "TPB_PANEL")
#PIECES=("PLATE" "STEEL_TUBE1" "STEEL_TUBE2" "STEEL_INSIDE_TUBE" "STEEL_TUBE_END" "STEEL_WINDOW_TUBE1" "STEEL_WINDOW_TUBE2" "STEEL_WINDOW_TUBE3" "STEEL_WINDOW_TUBE2_INNER1" "STEEL_WINDOW_TUBE2_INNER2" "SAPPHIRE_WINDOW" "STEEL_CONE")

PIECES=("PTFE_PANEL")

folder="23-11-26_check" # Folder to save plots and outputs

python process.py "${PIECES[@]}"

for ((cpt=1; cpt<=${#PIECES[@]}; cpt++)); do

    piece=${PIECES[cpt]}
    echo "The piece is : $piece"

    mkdir -p "$folder"

    python check_geometry.py $piece --overlaps --points hits_pmt.csv hits_g4.csv --save $folder/$piece > output_$piece.txt
    
    # crosscheck
    python distrib.py $piece

    mv output_* $folder/
 
done
