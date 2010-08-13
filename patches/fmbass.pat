#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)
 
#note
200 XToFrequency freq(x)

#noise
2000 EnvelopeGenerator mod_amt(touch, 0.01, 0.5, 0.2, 0.2)
2000 Rescaler y_mod(y, 0, 0.8)
2000 Multiply mod_amp(mod_amt, y_mod)
Sine mod_unit(freq, touch)
Multiply mod(mod_unit, mod_amp)
Sine carrier(freq, touch, mod)

#amp
2000 EnvelopeGenerator amp_unit(touch, 0.01, 8, 0, 0.1)
2000 Rescaler y_amp(y, 0.5, 1)
2000 Multiply amp(amp_unit, y_amp)
Multiply notes(carrier, amp)

#panning
PingPongDelay output(0.15, 0.1, notes, 0.4, 0.8, 0.8)