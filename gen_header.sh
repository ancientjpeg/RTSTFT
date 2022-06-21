#!/bin/sh

GLOBALS="src/rt_globals.h"
HEADER_SRC="src/rtstft.h"
HEADER_GEN="build/include/librtstft.h"

NEWLINE=$(echo)
sed -n '/include <string\.h>/,$p;' $GLOBALS | sed '1d;$d;' > globals.temp
echo "struct rt_params_t;\ntypedef struct rt_params_t *rt_params;" >> globals.temp
echo "struct rt_chan_t;\ntypedef struct rt_chan_t *rt_chan;" >> globals.temp
echo "struct rt_manip_t;\ntypedef struct rt_manip_t *rt_manip;\n" >> globals.temp
sed '/rt_internal_API/r globals.temp' $HEADER_SRC | sed '/rt_internal_API/d' > $HEADER_GEN
rm globals.temp
if [[ ! -f $HEADER_GEN ]]
then
    echo unsuccessful copy op
    exit 1
fi
