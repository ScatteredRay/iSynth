#input
Input x(0)
Input y(1)
Input touch(2)

#note
Rescaler note(x, 48, 72)
Quantize note_quant(note)
NoteToFrequency freq(note_quant, "major")

Sample osc("samples/orchhit.wav", freq, touch, 1, 1)

#filter
Rescaler filter_offset(y, 1000, 5000)
Add filter_freq(freq, filter_offset)
Filter filter(osc, filter_freq, 0)

#panning
Rescaler panpos(x, 0.3, 0.7)
Pan output(filter, panpos)