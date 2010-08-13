#!/bin/sh
CC=arm-linux-g++
CFLAGS="-Iinclude -O2 -msoft-float -fno-math-errno -funsafe-math-optimizations -fno-trapping-math -fcx-limited-range -fno-rounding-math -fno-signaling-nans"
LIBS="-L../libs/exports/arm-linux/lib -lasound"
SOURCES="main synth input_linux audio_alsa file_linux"
OBJECTS=

# Compile everything
for i in ${SOURCES}
do
    if [ ! -e ${i}.o -o ${i}.o -ot ${i}.cpp ]
	then
        echo "Compiling ${i}.cpp..."
		${CC} ${CFLAGS} ${i}.cpp -c -o ${i}.o || exit $?
	fi
	OBJECTS="${OBJECTS} ${i}.o"
done

# Link it together
echo "Linking synth..."
${CC} ${OBJECTS} -o synth ${LIBS}
