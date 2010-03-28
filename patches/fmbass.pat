#input
Input x(0)
Input y(1)
Input touch(2)
 
#note
Rescaler note(x, 24, 48)
Quantize note_quant(note)
NoteToFrequency freq(note_quant, "pentminor")

#noise
Sine mod_unit(freq, touch)
EnvelopeGenerator mod_amt(touch, 0.01, 0.5, 0.2, 0.2)
Multiply mod_enveloped(mod_unit, mod_amt)
Rescaler y_mod(y, 0, 0.8)
Multiply mod(mod_enveloped, y_mod)
Sine carrier(freq, touch, mod)

#amp
EnvelopeGenerator amp_unit(touch, 0.01, 8, 0, 0.1)
Rescaler y_amp(y, 0.5, 1)
Multiply amp(amp_unit, y_amp)
Multiply notes(carrier, amp)

#panning
PingPongDelay output(0.15, 0.1, notes, 0.4, 0.8, 0.8)