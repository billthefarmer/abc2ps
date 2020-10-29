#!/bin/sh
#

if test $# -lt 2;
    then
    echo "Syntax: $0 <abc file> <tune number> [<box key(s)>] [<cross fingering>]"
    exit 1
fi

awk -f ${0%${0##*/}}tab.awk -v select=$2 box=$3 cross=$4 $5 $6 $1
