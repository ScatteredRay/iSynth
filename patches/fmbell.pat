#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)
 
#note
200 XToFrequency freq(x)

#noise
200 Rescaler mod_freqmult(y, 6, 8)
200 Multiply mod_freq(freq, mod_freqmult)
200 Clipper mod_freq_clipped(mod_freq, 0, 22050)
Sine mod_unit(mod_freq_clipped, touch)
Multiply mod(mod_unit, 0.2)
Sine carrier(freq, touch, mod)

#amp
2000 EnvelopeGenerator amp_unit(touch, 0, 2, 0, 2)
2000 Rescaler y_amp(y, 0.5, 1)
2000 Multiply amp(amp_unit, y_amp)
Multiply notes(carrier, amp)

#panning
PingPongDelay output(0.15, 0.05, notes, 0.6, 0.8, 0.8) 
