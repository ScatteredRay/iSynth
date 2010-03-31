#ifndef _INPUT_H
#define _INPUT_H

void initInput(int argc, char **argv);
void deinitInput();

int argCount();
char *getArg(int n);

void readInputAxis(int axis, float *buffer, int size);
char getKey();

const int K_ESCAPE = 27, K_LEFT = -1, K_RIGHT = -2;

#endif