#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 XToFrequency freq(x)

#noise
BLSaw osc(freq)

#filter
200 EnvelopeGenerator cutoff_env_unit(touch, 0, 2, 0, 0.1)
200 Rescaler cutoff_env(cutoff_env_unit, 800, 1600)
200 Rescaler cutoff_y(y, 0.5, 1)
200 Multiply cutoff(cutoff_y, cutoff_env)
Filter filter(osc, cutoff, 0.9)

#amp
400 EnvelopeGenerator amp(touch, 0.1, 0.5, 0.5, 0.1)
Multiply notes(filter, amp)

#panning
PingPongDelay output(0.15, 0.05, notes, 0.6, 0.8, 0.8)
