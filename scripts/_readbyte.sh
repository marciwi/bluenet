#!/bin/bash

path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $path/_utils.sh
source $path/_config.sh

SCRIPT_DIR=$path/jlink
TEMP_DIR=$path/tmp
mkdir -p $TEMP_DIR

ADDRESS=${1:? "Requires address as argument"}
# DEVICE=nrf51822
DEVICE=nRF52832_xxAA

sed "s|@ADDRESS@|$ADDRESS|" $SCRIPT_DIR/readbyte.script > $TEMP_DIR/readbyte.script

echo "$JLINK -Device $DEVICE -speed 4000 -If SWD $TEMP_DIR/readbyte.script"
$JLINK -Device $DEVICE -speed 4000 -If SWD $TEMP_DIR/readbyte.script

echo "The result should be something like:"
echo "    1000005C = 3C 00 FF FF"
