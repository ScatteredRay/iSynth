200 Input x(0)
200 Input y(1)
200 Input touch(2)

200 EnvelopeGenerator pitch_env(touch, 0.001, 0.6, 0, 0.6)
200 Rescaler pitch(pitch_env, 210, 180)
Sine body(pitch, touch)
Sine body2(150, touch)

2000 EnvelopeGenerator body_env(touch, 0.001, 0.5, 0, 0.5)
2000 Rescaler body_vol(x, 1, 0)
2000 Multiply body_amp(body_env, body_vol)
200 EnvelopeGenerator body2_env(touch, 0.001, 0.3, 0, 0.3)

Multiply body_amped(body, body_amp)
Multiply body2_enved(body2, body2_env)
Add bodies(body_amped, body2_enved)

200 EnvelopeGenerator noise_env(touch, 0.01, 1, 0, 1)
Noise noise()
Multiply noise_enved(noise, noise_env)
200 Rescaler noise_r(x, 0, 0.5)
Filter noise_filtered(noise_enved, 10000, noise_r)
200 Rescaler noise_amp_x(x, 0, 0.5)
200 Rescaler noise_amp_y(y, 0, 0.5)
200 Add noise_amp(noise_amp_x, noise_amp_y)
Multiply noise_amped(noise_filtered, noise_amp)

2000 EnvelopeGenerator gate_trigger(touch, 0.01, 0.01, 0, 0)
200 Rescaler gate_decay(y, 0.1, 1)
AttackRelease gate(gate_trigger, .01, gate_decay)
Limiter gate_limited(gate, 3)
Multiply noise_gated(noise_amped, gate_limited)

Add drums(bodies, noise_gated)

Pan output(drums, 0.5)