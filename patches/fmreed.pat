#input
Input x(0)
Input y(1)
Input touch(2)
 
#note
Rescaler note(x, 48, 72)
Quantize note_quant(note)
NoteToFrequency note_freq(note_quant, "minor")

#vibrato
EnvelopeGenerator vibrato_env(touch, 3, 1, 1, 1)
Sine vibrato_lfo(5)
Multiply vibrato_unit(vibrato_lfo, vibrato_env)
Rescaler vibrato(vibrato_unit, 0.985, 1.015)
Multiply freq(note_freq, vibrato)

#noise
Multiply freq_mult(freq, 2)
Sine mod_unit(freq_mult, touch)
EnvelopeGenerator mod_env(touch, 1, 3, 0.5, 0.2)
Multiply mod_enveloped(mod_unit, mod_env)
Rescaler y_mod(y, 0, 0.4)
Multiply mod(mod_enveloped, y_mod)
Sine carrier(freq, touch, mod)

#amp
EnvelopeGenerator amp_unit(touch, 0.1, 6, 0.3, 0.1)
Rescaler y_amp(y, 0.5, 1)
Multiply amp(amp_unit, y_amp)
Multiply notes(carrier, amp)

#panning
PingPongDelay output(0.15, 0.02, notes, 0.6, 0.8, 0.8)