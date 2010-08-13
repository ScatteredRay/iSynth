#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)
 
#note
200 XToFrequency note_freq(x)

#vibrato
2000 EnvelopeGenerator vibrato_env(touch, 3, 1, 1, 1)
200 Sine vibrato_lfo(5)
200 Multiply vibrato_unit(vibrato_lfo, vibrato_env)
200 Rescaler vibrato(vibrato_unit, 0.985, 1.015)
200 Multiply freq(note_freq, vibrato)

#noise
2000 EnvelopeGenerator mod_env(touch, 1, 3, 0.5, 0.2)
2000 Rescaler y_mod(y, 0, 0.4)
2000 Multiply mod_amp(mod_env, y_mod)
200 Multiply freq_mult(freq, 2)
Sine mod_unit(freq_mult, touch)
Multiply mod(mod_unit, mod_amp)
Sine carrier(freq, touch, mod)

#amp
2000 EnvelopeGenerator amp_unit(touch, 0.5, 6, 0.3, 0.2)
2000 Rescaler y_amp(y, 0.5, 1)
2000 Multiply amp(amp_unit, y_amp)
Multiply amp_(amp, 1)
Multiply notes(carrier, amp_)

#panning
PingPongDelay output(0.15, 0.02, notes, 0.6, 0.8, 0.8)