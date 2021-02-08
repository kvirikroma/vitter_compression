#!/bin/bash

export start_directory=$PWD
cd ./asm
mkdir -p /tmp/vitter/compiler_symlinks
rm -rf /tmp/vitter/compiler_symlinks
mkdir /tmp/vitter/compiler_symlinks

for f in ./*.asm
do
    base_f="$(basename -- $f)"
    ln -s $start_directory/asm/$f /tmp/vitter/compiler_symlinks/$base_f
done

for f in ./include/*.asm
do
    base_f="$(basename -- $f)"
    ln -s $start_directory/asm/$f /tmp/vitter/compiler_symlinks/$base_f
done

cd /tmp/vitter/compiler_symlinks/
for f in ./*.asm
do
    if \
        nasm -f elf64 $f -o $start_directory/asm/${f::-4}.o &&\
        yasm -f elf64 -g dwarf2 $f -o $start_directory/asm/${f::-4}.o
    then
        true
    else
        exit
    fi
done
cd $start_directory
rm -rf /tmp/vitter/compiler_symlinks

gcc -Wall -m64 -no-pie -gdwarf-2 -o ./vitter ./*.c ./asm/*.o -Bstatic -Ofast

rm -rf ./asm/*.o
rm -rf ./*.o
