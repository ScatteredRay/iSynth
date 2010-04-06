Input x(0)
Input y(1)
Input touch(2)

Rescaler pitch(x, 55, 220)
Rescaler pan(y, 0, 1)

Saw saw(pitch) 
Multiply saw_gated(saw, touch)
Pan output(saw_gated, pan)