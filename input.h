#ifndef _INPUT_H
#define _INPUT_H

#include <vector>
#include <string>

void initInput(int argc, char **argv);
void deinitInput();

void populatePatchList(std::vector<std::string>& patches);
void setupLogging(std::vector<std::string>& log_list);

void readInputAxis(int axis, float *buffer, int size);
char getKey();

const int K_ESCAPE = 27, K_LEFT = -1, K_RIGHT = -2;

#endif
