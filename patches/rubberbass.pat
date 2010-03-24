#input
Input x(0)
Input y(1)
Input touch(2)

#note
Rescaler note(x, 24, 48)
Quantize note_quant(note)
NoteToFrequency note_freq(note_quant, "lydian")
SlewLimiter note_freq_sl(note_freq, .0005, .0005)

#vibrato
Sine vibrato_lfo(3, 0)
EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
Multiply freq(note_freq_sl, vibrato)

#noise
Sine osc1(freq, 0)
Rectifier osc1_rect(osc1)
Add freq_detuned(freq, 0.3)
Pulse osc2(freq_detuned, 0.8, 0)
Add oscs(osc1_rect, osc2)

#filter
EnvelopeGenerator env(touch, 0.03, 4, 0.5, 0.1)
Rescaler cutoff_env(env, 100, 700)
Rescaler cutoff_y(y, 0.25, 1)
Multiply cutoff(cutoff_y, cutoff_env)
Add cutoff_offset(cutoff, freq)
Filter filter(oscs, cutoff_offset, 0.95)

#amp
Multiply notes(filter, env)

#panning
PingPongDelay output(0.1, 0.2, notes, 0.2, 1.0, 0.9)