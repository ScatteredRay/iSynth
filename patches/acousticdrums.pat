200 Input x(0)
200 Input y(1)
200 Input touch(2)

200 Rescaler xpos(x, 0, 1)
200 Rescaler ypos(y, 0, 1)
200 Rescaler xneg(x, 1, 0)
200 Rescaler yneg(y, 1, 0)

Sample kick     ("samples/kick.wav",      261.2, touch, 1, 1)
Sample snare    ("samples/snare.wav",     261.2, touch, 1, 1)
Sample openhat  ("samples/openhat.wav",   261.2, touch, 1, 1)
Sample closedhat("samples/closedhat.wav", 261.2, touch, 1, 1)

Multiply kick_amp     (xneg, yneg)
Multiply snare_amp    (xneg, ypos)
Multiply openhat_amp  (xpos, ypos)
Multiply closedhat_amp(xpos, yneg)

Multiply kick_amped     (kick,      kick_amp)
Multiply snare_amped    (snare,     snare_amp)
Multiply openhat_amped  (openhat,   openhat_amp)
Multiply closedhat_amped(closedhat, closedhat_amp)

Pan kick_p     (kick_amped,      0.5)
Pan snare_p    (snare_amped,     0.3)
Pan openhat_p  (openhat_amped,   0.8)
Pan closedhat_p(closedhat_amped, 0.8)

StereoAdd mix   (kick_p, snare_p)
StereoAdd mix_  (mix,    openhat_p)
StereoAdd output(mix_,   closedhat_p)