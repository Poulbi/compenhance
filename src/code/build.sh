#!/bin/sh

ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"

Compiler="clang"

CompilerFlags="
-g
-nostdinc++
-DSIM86_INTERNAL
"

WarningFlags="
-Wall
-Wextra
-Wno-unused-label
-Wno-unused-variable
-Wno-unused-function
-Wno-unused-but-set-variable
-Wno-missing-field-initializers
-Wno-write-strings
"

Libs="./reference_decoder/sim86_lib.cpp"

if false
then
 Source="./shared_library_test.cpp"
 printf '%s\n' "$Source"
 $Compiler $CompilerFlags $WarningFlags \
  -o ../build/shared_library_test \
  $Libs $Source
fi

Source="sim86.cpp"
printf '%s\n' "$Source"
$Compiler $CompilerFlags $WarningFlags \
 -o ../build/sim86 \
 $Libs $Source
