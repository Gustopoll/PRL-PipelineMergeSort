#!/bin/bash

# ak je viacej arguemtov, tak je to chyba
if [ $# -gt 0 ];then 
    echo "Too many arguemnts";
    exit 1;    
fi;
numbers=16;

#vypočíta sa logaritmus
function log2 {
    local x=0
    for (( y=$1-1 ; $y > 0; y >>= 1 )) ; do
        let x=$x+1
    done
    echo $x
}

# výpočet počtu procesorov 
lg=$(log2 $numbers)
let process=$lg+1

#kompilácia c++
mpic++ --prefix /usr/local/share/OpenMPI  -o pms pms.cpp

#vytvorenie súboru z náhodnými číslami
dd if=/dev/random bs=1 count=$numbers of=numbers 2>/dev/null

#spustenie
mpirun --prefix /usr/local/share/OpenMPI -np $process pms

#vymazanie
rm -f pms numbers