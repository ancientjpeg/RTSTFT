#!/bin/sh

HEADER_SRC="src/rtstft.h"
HEADER_GEN="build/include/rtstft.h"
GLOBALS="src/rt_globals.h"

NEWLINE=$(echo)
sed -n '/include <string\.h>/,$p;' $GLOBALS | sed '1d;$d;' > globals.temp
echo "struct rt_params_t;\ntypedef struct rt_params_t *rt_params;\n" >> globals.temp
sed '/rt_internal_API/r globals.temp' $HEADER_SRC | sed '/rt_internal_API/d' > $HEADER_GEN
rm globals.temp
if [[ ! -f $HEADER_GEN ]]
then
    echo unsuccessful copy op
    exit 1
fi
# rm tempGlob
# cat $HEADER
# rm $GLOBALS
