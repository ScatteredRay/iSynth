#ifndef _SYNTH_H
#define _SYNTH_H

#include "file.h"

void synthProduceStream(short *buffer, int samples);
char *describeTimeSpent();

void synthSetPatch(const char*);
void synthSetKey  (int key);
void synthSetScale(const char *name);
void synthSetRange(int start_octave, int octave_range);

struct scale
{ 
  const char *name;
  const char *steps;
};

extern scale g_scales[];
extern const char *g_keys[];

#endif
