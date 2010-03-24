#input
Input x(0)
Input y(1)
Input touch(2)

#note
Rescaler note(x, 36, 60)
Quantize note_quant(note)
EnvelopeGenerator pitch(touch, 0.001, 0.05, 0, 1)
Rescaler pitch_add(pitch, 0, 12)
Add note_offset(note_quant, pitch_add)
NoteToFrequency note_freq(note_offset, "minor")

#vibrato
Sine vibrato_lfo(3, 0)
EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
Multiply freq(note_freq, vibrato)

#noise
Sine mult_unit(0.5, touch)
Rescaler mult(mult_unit, .5, 1.5)
Multiply detune(freq, mult)
Pulse osc_retrigger(freq, 0.5, 0)
Triangle osc(detune, osc_retrigger)

#filter
EnvelopeGenerator env(touch, 0.01, 0.7, 0.5, 0.1)
Rescaler cutoff_env(env, 400, 1200)
Rescaler cutoff_y(y, 0.25, 1)
Multiply cutoff(cutoff_y, cutoff_env)
Add cutoff_offset(cutoff, freq)
Filter filter(osc, cutoff_offset, 0.7)

Rescaler bits(y, 4, 64)
BitCrusher crushed(filter, bits)

#amp
Multiply notes(crushed, env)

#panning
PingPongDelay output(0.1, 0.2, notes, 0.0, 1.0, 0.9)