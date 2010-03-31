Input x(0)
Input y(1)
Input touch(2)

EnvelopeGenerator pitch_env(touch, 0.001, 0.6, 0, 0.6)
Rescaler pitch(pitch_env, 210, 180)
Sine body(pitch, touch)
Sine body2(150, touch)

EnvelopeGenerator body_env(touch, 0.001, 0.5, 0, 0.5)
Multiply body_enved(body, body_env)
Rescaler body_vol(x, 1, 0)
Multiply body_amped(body_enved, body_vol)

EnvelopeGenerator body2_env(touch, 0.001, 0.3, 0, 0.3)
Multiply body2_enved(body2, body2_env)
Add bodies(body_amped, body2_enved)

Noise noise()
EnvelopeGenerator noise_env(touch, 0.01, 1, 0, 1)
Multiply noise_enved(noise, noise_env)
Rescaler noise_r(x, 0, 0.5)
Filter noise_filtered(noise_enved, 10000, noise_r)
Rescaler noise_amp_x(x, 0, 0.5)
Rescaler noise_amp_y(y, 0, 0.5)
Add noise_amp(noise_amp_x, noise_amp_y)
Multiply noise_amped(noise_filtered, noise_amp)

EnvelopeGenerator gate_trigger(touch, 0.01, 0.01, 0, 0)
Rescaler gate_decay(y, 0.1, 1)
AttackRelease gate(gate_trigger, .01, gate_decay)
Limiter gate_limited(gate, 3)
Multiply noise_gated(noise_amped, gate_limited)

Add drums(bodies, noise_gated)

Pan output(drums, 0.5)