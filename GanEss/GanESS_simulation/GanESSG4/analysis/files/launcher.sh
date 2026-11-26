#!/bin/zsh

# Vessel
#PIECES=("VESSEL_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "CONE_ENDCAP1" "CONE_ENDCAP2")
#PIECES=("TUBS_CF1_INNER")
PIECES=("VESSEL_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "VESSEL_INNER_FLANGE1" "VESSEL_INNER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "CONE_ENDCAP" "COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2" "RINGS_Z+" "RINGS_Z-" "PTFE_PANEL" "CROSSBAR" "CATHODE" "EL" "PLATE" "STEEL_TUBE1" "STEEL_TUBE2" "STEEL_CONE" "STEEL_INSIDE_TUBE" "STEEL_TUBE_END" "STEEL_WINDOW_TUBE1" "STEEL_WINDOW_TUBE2" "STEEL_WINDOW_TUBE3" "STEEL_WINDOW_TUBE2_INNER1" "STEEL_WINDOW_TUBE2_INNER2" "SAPPHIRE_WINDOW")
# Field Cage
#PIECES=()

#PIECES=("VESSEL_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "VESSEL_INNER_FLANGE1" "VESSEL_INNER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "CONE_ENDCAP" "COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2" RINGS_Z+" "RINGS_Z-" "PTFE_PANEL" "CROSSBAR" "CATHODE" "EL" "PLATE" "STEEL_TUBE1" "STEEL_TUBE2" "STEEL_CONE" "STEEL_INSIDE_TUBE" "STEEL_TUBE_END" "STEEL_WINDOW_TUBE1" "STEEL_WINDOW_TUBE2" "STEEL_WINDOW_TUBE3" "STEEL_WINDOW_TUBE2_INNER1" "STEEL_WINDOW_TUBE2_INNER2" "SAPPHIRE_WINDOW")

folder="10-11-26_check" # Folder to save plots and outputs

python process.py "${PIECES[@]}"

for ((cpt=1; cpt<=${#PIECES[@]}; cpt++)); do

    piece=${PIECES[cpt]}
    echo "The piece is : $piece"

    mkdir -p "$folder"

    # Check the angular momentum of the primary vertex
    #python angular_momentum.py $piece
    #python check_geometry.py $piece --overlaps --points hits.csv --save $folder/$piece > output_$piece.txt

    # Check generator and compare it with a theoretical one (geantinos over the full detector)
    #python check_geometry2.py $piece --overlaps --points hits.csv hits_g4.csv --save $folder/$piece > output_$piece.txt
    
    python check_geometry_new.py $piece --overlaps --points hits_$piece.csv --save $folder/$piece > output_$piece.txt

    mv output_* $folder/
 
done