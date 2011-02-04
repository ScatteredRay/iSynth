#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 XToFrequency freq(x)
200 Multiply freq_offset(freq, 1.01)
Sample osc("samples/badpiano.wav", freq, touch, 0.784, 1)

#filter
Rescaler filter_offset(y, 1000, 5000)
Add filter_freq(freq, filter_offset)
Filter filter(osc, filter_freq, 0)

#amp
2000 EnvelopeGenerator env(touch, 0.0001, 10, 0, 0.1)
2000 Rescaler velocity(y, 0.3, 1)
2000 Multiply env_vel(env, velocity)
Multiply notes(filter, env_vel)

#panning
2000 Rescaler panpos(x, 0.3, 0.7)
Pan notes_panned(notes, panpos)
PingPongDelay delay(0.1, 0.2, notes, 0.4, 0.0, 0.7)
StereoAdd output(notes_panned, delay)
