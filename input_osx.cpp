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

void initInput()
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