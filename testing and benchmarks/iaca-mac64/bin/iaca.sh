#!/bin/bash
mypath=`pwd`
ld_lib="$DYLD_LIBRARY_PATH"
app_loc="../lib"

if [ "$DYLD_LIBRARY_PATH" = "" ]
then
export DYLD_LIBRARY_PATH="$mypath/$app_loc"
else
export DYLD_LIBRARY_PATH="$mypath/$app_loc:$DYLD_LIBRARY_PATH"
fi

$mypath/iaca $@

export DYLD_LIBRARY_PATH="$ld_lib"