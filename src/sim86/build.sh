#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

Build="../../build"
mkdir -p "$Build"
mkdir -p generated

Compiler="clang"

CompilerFlags="
-I./libs/metadesk
-g
-fdiagnostics-absolute-paths
-nostdinc++
-DSIM86_INTERNAL
"

WarningFlags="
-Wall
-Wextra
-Wno-unused-label
-Wno-unused-variable
-Wno-unused-function
-Wno-unused-value
-Wno-unused-but-set-variable
-Wno-missing-field-initializers
-Wno-write-strings
"

printf '[metadata generation]\n'
$Compiler $CompilerFlags $WarningFlags \
 -o "$Build"/sim86_meta \
 sim86_meta.c
"$Build"/sim86_meta ./sim86.mdesk > ./generated/generated.cpp

printf '[debug mode]\n'
printf '[%s build]\n' "$Compiler" 
Source="sim86.cpp"
$Compiler $CompilerFlags $WarningFlags \
 -o "$Build"/sim86 \
  sim86.cpp
