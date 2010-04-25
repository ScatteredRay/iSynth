#include "input.h"

float osx_x;
float osx_y;
float osx_down;

void inputDown(float down)
{
    osx_down = down;
}

void inputXY(float X, float Y)
{
    osx_x = X;
    osx_y = Y;
}

void initInput(int _argc, char **_argv)
{
    osx_x = 0.0f;
    osx_y = 0.0f;
    osx_down = 0.0f;
}

void readInputAxis(int axis, float *buffer, int size)
{
    float src;
    switch(axis)
    {
        case 0:
            src = osx_x;
            break;
        case 1:
            src = osx_y;
            break;
        case 2:
            src = osx_down;
            break;
        default:
            src = 0.0f;
            break;
    }

    for(int i=0; i<size; i++)
        buffer[i] = src;
}

void populatePatchList(vector<string>& patches)
{
    //char *patch_filename = "patches/pad.pat";
    if(patches.size() == 0) patches.push_back("patches/pad.pat");
}

void setupLogging(vector<string>& log_list)
{
}
