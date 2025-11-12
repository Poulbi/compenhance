#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

Build="../../build"
mkdir -p "$Build"
mkdir -p generated

Compiler="clang"

CompilerFlags="
-g
-fdiagnostics-absolute-paths
-nostdinc++
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
-Wno-unused-parameter
"

LinkerFlags="-lm"

printf '[debug mode]\n'
printf '[%s build]\n' "$Compiler" 
$Compiler $CompilerFlags $WarningFlags $LinkerFlags \
 -o "$Build"/haversine_generator \
  haversine_generator.cpp