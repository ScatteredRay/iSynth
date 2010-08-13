#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 XToFrequency note_freq(x)
200 SlewLimiter note_freq_sl(note_freq, .05, .05)

#vibrato
200 Sine vibrato_lfo(3)
200 EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
200 Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
200 Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
200 Multiply freq(note_freq_sl, vibrato)

#noise
Sine osc1(freq)
Rectifier osc1_rect(osc1)
200 Add freq_detuned(freq, 0.3)
Pulse osc2(freq_detuned, 0.8)
Add oscs(osc1_rect, osc2)

#filter
EnvelopeGenerator env(touch, 0.03, 4, 0.5, 0.1)
200 Rescaler cutoff_env(env, 100, 700)
200 Rescaler cutoff_y(y, 0.25, 1)
200 Multiply cutoff(cutoff_y, cutoff_env)
200 Add cutoff_offset(cutoff, freq)
Filter filter(oscs, cutoff_offset, 0.95)

#amp
Multiply notes(filter, env)

#panning
PingPongDelay output(0.1, 0.2, notes, 0.2, 1.0, 0.9)