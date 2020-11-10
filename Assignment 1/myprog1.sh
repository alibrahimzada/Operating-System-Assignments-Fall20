#!/bin/bash

if [ -z $1 ]
then
    echo "Error - No filename is given"
    exit 1
fi

counts=(0 0 0 0 0 0 0 0 0 0)  # TODO: try to solve for an empty array
while read line
do
    counts[$line]=`expr ${counts[$line]} + 1`
done <<< "$(cat $1)"

for (( i = 0 ; i < ${#counts[@]} ; i++ ))
do
    multiplexer=$(printf "%${counts[$i]}s")
    echo "$i ${multiplexer// /*}"
done
