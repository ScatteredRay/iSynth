#input
Input x(0)
Input y(1)
Input touch(2)

Sample loop("samples/funky.wav", 261.625565, touch, 0, 1)

Rescaler filter_freq(y, 10, 5000)
Filter filter(loop, filter_freq, 0.7)

EnvelopeGenerator env(touch, 0.0001, 5, 1, 0.1)
Multiply amped(filter, env)

Rescaler delay_fb(x, 0, 0.7)
Rescaler delay_wet(x, 0, 0.3)
PingPongDelay output(0.48, 1, amped, delay_wet, 1, delay_fb)
