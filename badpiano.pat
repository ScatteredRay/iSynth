#input
Input x(0)
Input y(1)
Input touch(2)

#note
Rescaler note(x, 48, 72)
Quantize note_quant(note)
NoteToFrequency freq(note_quant, "lydian")

Multiply freq_offset(freq, 1.01)
Sample osc("badpiano.wav", 44100, freq, touch, 0.784, 1)

#filter
Rescaler filter_offset(y, 1000, 5000)
Add filter_freq(freq, filter_offset)
Filter filter(osc, filter_freq, 0)

#amp
EnvelopeGenerator env(touch, 0.0001, 5, 0, 0.1)
Rescaler velocity(y, 0.3, 1)
Multiply env_vel(env, velocity)
Multiply notes(filter, env_vel)

#panning
Rescaler panpos(x, 0.3, 0.7)
Pan notes_panned(notes, panpos)
PingPongDelay delay(0.1, 0.2, notes, 0.4, 0.0, 0.7)
StereoAdd output(notes_panned, delay)