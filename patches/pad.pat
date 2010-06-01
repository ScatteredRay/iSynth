#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 Rescaler note(x, 46, 70)
200 Quantize note_quant(note)
200 NoteToFrequency note_freq(note_quant, "mixolydian")

# vibrato
200 Sine vibrato_lfo(5)
200 EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
200 Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)

200 Rescaler vibrato_y(y, 0, 1)
200 Multiply vibrato_unit(vibrato_enveloped, vibrato_y)

200 Rescaler vibrato(vibrato_unit, 0.975, 1.025)
200 Multiply freq(note_freq, vibrato)

#noise
2000 EnvelopeGenerator pw_unit(touch, 4, 1, 0.9, 0.1)
200 Rescaler pw(pw_unit, 0.5, 0.05)
200 Add freq_detuned(freq, 1)
Pulse osc_1(freq, pw)
Pulse osc_2(freq_detuned, pw)
Add osc_mix(osc_1, osc_2)

#filter
200 EnvelopeGenerator cutoff_env_unit(touch, 0.02, 4, 0, 0.1)
2000 EnvelopeGenerator vol(touch, 0.6, 1.0, 0.3, 0.1)
200 Rescaler cutoff_env(cutoff_env_unit, 1000, 2000)
200 Rescaler cutoff_y(y, 0.5, 1)
200 Multiply cutoff(cutoff_y, cutoff_env)
Filter filter(osc_mix, cutoff, 0.7)
Multiply notes(filter, vol)

#panning
200 Rescaler panpos(x, 0.15, 0.85)
Pan panned_notes(notes, panpos)
PingPongDelay delay(0.5, 0.5, notes, 0.5, 0, 0.3)
StereoAdd output(delay, panned_notes)