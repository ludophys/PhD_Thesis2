#!/bin/zsh

#PIECES=("VESSEL_BODY" "VESSEL_GAS_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "VESSEL_INNER_FLANGE1" "VESSEL_INNER_FLANGE2" "GAS_FLANGE1" "GAS_FLANGE2" "COVER_FLANGE1" "GAS_COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2" "GAS_COVER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "GAS_CYL_ENDCAP1" "GAS_CYL_ENDCAP2" "CONE_ENDCAP1" "CONE_ENDCAP2" "GAS_CONE_ENDCAP1" "GAS_CONE_ENDCAP2")
PIECES=("VESSEL_BODY")
folder="geant4check"
#python process.py

for ((cpt=1; cpt<=${#PIECES[@]}; cpt++)); do

    piece=${PIECES[cpt]}
    echo "The piece is : $piece"
    python check_geometry.py $piece --overlaps --points hits_g4.csv --save $folder/$piece > output_$piece.txt
    mv output_* $folder/
done