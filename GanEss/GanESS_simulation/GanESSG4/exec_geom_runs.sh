#!/bin/zsh

# This script launch events in defined regions and then convert 
# the .h5 output into a .csv file containing only usefull 
# information for geometry and generator studies

# ============================================================
# Setup environment
# ============================================================

source ~/.zshrc

cd /Users/ldonneger/Desktop/PhD_Thesis2/ || exit 1

source ./source.sh

cd /Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4 || exit 1


# ============================================================
# Configuration
# ============================================================

MACRO_CONF="macros/GanESS_geantino_geo_temp.config.mac"
MACRO_INIT="macros/GanESS_geantino_geo_temp.init.mac"

RUN_MACRO_CONF="macros/GanESS_geantino_geo_auto.config.mac"
RUN_MACRO_INIT="macros/GanESS_geantino_geo_auto.init.mac"

EVENTS=1000000

EXECUTABLE="./bin/ganess"


# Full geometry check

PIECES=("VESSEL_BODY" "VESSEL_FLANGE1" "VESSEL_FLANGE2" "VESSEL_INNER_FLANGE1" "VESSEL_INNER_FLANGE2" "CYL_ENDCAP1" "CYL_ENDCAP2" "CONE_ENDCAP" "COVER_FLANGE1" "TUBS_CF1_INNER" "COVER_FLANGE2")
#PIECES=("PTFE_PANEL" "TPB_PANEL" "CROSSBAR" "RINGS_Z+" "RINGS_Z-" "CATHODE" "EL")
#PIECES=("PLATE" "STEEL_TUBE1" "STEEL_TUBE2" "STEEL_INSIDE_TUBE" "STEEL_TUBE_END" "STEEL_WINDOW_TUBE1" "STEEL_WINDOW_TUBE2" "STEEL_WINDOW_TUBE3" "STEEL_WINDOW_TUBE2_INNER1" "STEEL_WINDOW_TUBE2_INNER2" "SAPPHIRE_WINDOW" "STEEL_CONE")

#PIECES=("TPB_PANEL")

# ============================================================
# Verifications
# ============================================================

echo "======================================"
echo "Environment"
echo "======================================"

echo "PWD = $PWD"
echo "PATH = $PATH"
echo "DYLD_LIBRARY_PATH = $DYLD_LIBRARY_PATH"

echo ""
echo "Geant4 :"
which geant4-config 2>/dev/null
geant4-config --version 2>/dev/null

echo ""
echo "Executable :"
ls -lh "$EXECUTABLE"

echo ""
echo "======================================"
echo ""


# ============================================================
# Loop over the runs
# ============================================================

for ((cpt=1; cpt<=${#PIECES[@]}; cpt++)); do

    piece=${PIECES[cpt]}

    echo "======================================"
    echo "Piece $piece"
    echo "======================================"

    # --------------------------------------------------------
    # Creation of config macro
    # --------------------------------------------------------
    
    sed \
        -e "s/PIECE/${piece}/g" \
        "$MACRO_CONF" > "$RUN_MACRO_CONF"

    if [ $? -ne 0 ]; then
        echo "ERREUR : impossible de créer $RUN_MACRO_CONF"
        exit 1
    fi

    # --------------------------------------------------------
    # print used macros
    # --------------------------------------------------------

    echo ""
    echo "----- $RUN_MACRO_INIT -----"
    cat "$RUN_MACRO_INIT"

    echo ""
    echo "----- $RUN_MACRO_CONF -----"
    cat "$RUN_MACRO_CONF"

    echo ""


    # --------------------------------------------------------
    # Geant4 commands
    # --------------------------------------------------------

    CMD=(
        "$EXECUTABLE"
        -b
        -n "$EVENTS"
        "$MACRO_INIT"
    )

    echo "Commande exécutée :"
    printf '%q ' "${CMD[@]}"
    echo ""
    echo ""


    # --------------------------------------------------------
    # Launch
    # --------------------------------------------------------

    "${CMD[@]}"

    STATUS=$?


    # --------------------------------------------------------
    # Verification
    # --------------------------------------------------------

    if [ $STATUS -ne 0 ]; then
        echo ""
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "ERROR : piece $piece failed."
        echo "Error code = $STATUS"
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        exit $STATUS
    fi

    python convert_output.py $piece

    rm -r $piece.h5

    echo ""
    echo "Piece $piece finished correctly."
    echo ""

done


echo "======================================"
echo "All the runs are finished."
echo "======================================"
