#!/bin/sh

dd if=/dev/random bs=1 count=16 of=numbers 2>/dev/null

mpic++ xmatea00.cpp -o xmatea00

mpirun -np 5 xmatea00 2> /dev/null

rm numbers
rm xmatea00
