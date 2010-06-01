#input
200 Input x(0)
200 Input y(1)
200 Input touch(2)

200 Rescaler filter_freq(y, 10, 5000)
200 EnvelopeGenerator env(touch, 0.01, 5, 1, 0.1)
200 Rescaler delay_fb(x, 0, 0.7)
200 Rescaler delay_wet(x, 0, 0.3)

Sample loop("samples/funky.wav", 261.625565, touch, 0, 1)
Filter filter(loop, filter_freq, 0.7)
Multiply amped(filter, env)
PingPongDelay output(0.48, 1, amped, delay_wet, 1, delay_fb)