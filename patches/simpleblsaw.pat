200 Input x(0)
200 Input y(1)
200 Input touch(2)
200 XToFrequency note_freq(x)
200 EnvelopeGenerator env(touch, .1, .1, .9, .1)
BLSaw saw(note_freq, touch)
Multiply enved(saw, env)
Pan output(enved, .5)

