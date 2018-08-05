#!/bin/bash

if [ ! $# -eq 1 ]; then
    echo Need one arg : which file you want to scan ?
    exit
fi

DIRECTORY=./backup_file

if [ ! -d "$DIRECTORY" ]; then
    mkdir $DIRECTORY
fi

TIME_PREF=$(date +"%s")
FILENAME=$(basename $1)
BKFILE=./backup_file/$FILENAME.$TIME_PREF

cp $1 $BKFILE

sed -i '1d' config
sed -i "1 i\INPUT = $BKFILE" config



#retirer tout les occurences de
