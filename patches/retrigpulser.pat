#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 Rescaler note(x, 36, 60)
200 Quantize note_quant(note)
200 EnvelopeGenerator pitch(touch, 0.001, 0.05, 0, 1)
200 Rescaler pitch_add(pitch, 0, 12)
200 Add note_offset(note_quant, pitch_add)
200 NoteToFrequency note_freq(note_offset, "minor")

#vibrato
200 Sine vibrato_lfo(3)
200 EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
200 Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
200 Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
200 Multiply freq(note_freq, vibrato)

#noise
200 Sine mult_unit(0.5, touch)
200 Rescaler mult(mult_unit, .5, 1.5)
200 Multiply detune(freq, mult)
Pulse osc_retrigger(freq)
Triangle osc(detune, osc_retrigger)

#filter
200 EnvelopeGenerator env(touch, 0.01, 0.7, 0.5, 0.1)
200 Rescaler cutoff_env(env, 400, 1200)
200 Rescaler cutoff_y(y, 0.25, 1)
200 Multiply cutoff(cutoff_y, cutoff_env)
200 Add cutoff_offset(cutoff, freq)
Filter filter(osc, cutoff_offset, 0.7)

#decimator
200 Rescaler trigger_freq_coef(y, 8, 41)
200 Multiply trigger_freq(freq, trigger_freq_coef)
Pulse trigger(trigger_freq, 0.5, 0)

SampleAndHold crushed(filter, trigger)

SlewLimiter smoothcrushed(crushed, 0.3, 0.3)

#amp
Multiply amped(smoothcrushed, env)

#panning
PingPongDelay output(0.15, 0.2, amped, 0.6, 1.0, 0.8)