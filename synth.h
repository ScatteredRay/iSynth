#ifndef _SYNTH_H
#define _SYNTH_H

void synthProduceStream(short *buffer, int samples);
void synthNextPatch(int offset = 1);

#endif
