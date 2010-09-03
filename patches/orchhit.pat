#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

#note
200 XToFrequency freq(x)

Sample osc("samples/orchhit.wav", freq, touch, 1, 1)

#filter
200 Rescaler filter_offset(y, 1000, 5000)
200 Add filter_freq(freq, filter_offset)
Filter filter(osc, filter_freq, 0)

#panning
200 Rescaler panpos(x, 0.3, 0.7)
Pan output(filter, panpos)