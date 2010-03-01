#input
Input x(0)
Input y(1)
Input touch(2)

#note
Rescaler note(x, 24, 48)
Quantize note_quant(note)
NoteToFrequency note_freq(note_quant, "lydian")

#vibrato
Sine vibrato_lfo(3)
EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
Multiply freq(note_freq, vibrato)

#noise
Saw osc1(freq)
Add freq_detuned(freq, 0.3)
Pulse osc2(freq_detuned, 0.8)
Add oscs(osc1, osc2)

#filter
EnvelopeGenerator env(touch, 0.01, 0.7, 0.5, 0.1)
Rescaler cutoff_env(env, 100, 700)
Rescaler cutoff_y(y, 0.25, 1)
Multiply cutoff(cutoff_y, cutoff_env)
Add cutoff_offset(cutoff, freq)
Filter filter(oscs, cutoff_offset, 0.95)

#output
Multiply notes(filter, env)
PingPongDelay output(0.1, 0.5, notes, 0.5, 1.0, 0.8)