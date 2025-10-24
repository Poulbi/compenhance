#!/bin/sh

ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"

mkdir -p ../build
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

Libs="./libs/reference_decoder/sim86_lib.cpp"

printf '[metadata generation]\n'
Source="sim86_meta.c"
$Compiler $CompilerFlags $WarningFlags \
 -o ../build/sim86_meta \
 $Source
../build/sim86_meta ./sim86.mdesk > ./generated/generated.cpp

printf '[%s build]\n' "$Compiler" 
Source="sim86.cpp"
$Compiler $CompilerFlags $WarningFlags \
 -o ../build/sim86 \
 $Libs $Source
