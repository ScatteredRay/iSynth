#ifndef _INPUT_H
#define _INPUT_H

void initInput();
void deinitInput();

void readInputAxis(int axis, float *buffer, int size);
char getKey();

#endif