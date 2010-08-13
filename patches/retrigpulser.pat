#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 XToFrequency note_freq(x)

#vibrato
200 Sine vibrato_lfo(3)
200 EnvelopeGenerator vibrato_env(touch, 2, 1, 1, 1)
200 Multiply vibrato_enveloped(vibrato_lfo, vibrato_env)
200 Rescaler vibrato(vibrato_enveloped, 0.985, 1.015)
200 Multiply freq(note_freq, vibrato)

#noise
Sine mult_unit(0.5, touch)
Rescaler mult(mult_unit, .5, 1.5)
Multiply detune(freq, mult)
Pulse osc_retrigger(freq)
Triangle osc(detune, osc_retrigger)

#filter
EnvelopeGenerator env(touch, 0.01, 0.7, 0.5, 0.1)
Rescaler cutoff_env(env, 400, 1200)
Rescaler cutoff_y(y, 0.25, 1)
Multiply cutoff(cutoff_y, cutoff_env)
Add cutoff_offset(cutoff, freq)
Filter filter(osc, cutoff_offset, 0.7)

#decimator
Rescaler trigger_freq_coef(y, 8, 40)
Multiply trigger_freq(freq, trigger_freq_coef)
Clipper limited_trigger_freq(trigger_freq, 0, 22050)
Pulse trigger(limited_trigger_freq, 0.5, 0)
SampleAndHold crushed(filter, trigger)
SlewLimiter smoothcrushed(crushed, 0.3, 0.3)

#amp
Multiply amped(smoothcrushed, env)

#panning
PingPongDelay output(0.15, 0.2, amped, 0.6, 1.0, 0.8)