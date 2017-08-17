#!/bin/bash

M1=$(../gv/gv -t 17 -T 7 -P salidatmp.eps $1 salida.txt 2>&1 | egrep -i "mediciones" | sed -e "s/\t//g")
M11=$(echo $M1|sed -e "s/Nro de Mediciones = //" -e "s/ .*//")
M2=$(../gv/gv -t 30 -T 17 -P salidatmp.eps $1 salida.txt 2>&1 | egrep -i "mediciones" | sed -e "s/\t//g")
M22=$(echo $M2|sed -e "s/Nro de Mediciones = //" -e "s/ .*//")
M3=$(../gv/gv -t 100 -T 30 -P salidatmp.eps $1 salida.txt 2>&1 | egrep -i "mediciones" | sed -e "s/\t//g")
M33=$(echo $M3|sed -e "s/Nro de Mediciones = //" -e "s/ .*//")
let TOTAL=M11+M22+M33

if [ $TOTAL -ne 0 ]; then
  echo -n "0-17um ($((M11*100/TOTAL))%) "
  echo $M1
  echo -n "17-30um ($((M22*100/TOTAL))%) "
  echo $M2
  echo -n "30-100um ($((M33*100/TOTAL))%) "
  echo $M3
fi
