Input x(0)
Input y(1)
Input touch(2)

EnvelopeGenerator pitch_env(touch, 0.01, 0.6, 0, 0.6)
Rescaler pitch(pitch_env, 30, 150)
Sine body(pitch, touch)

EnvelopeGenerator amp_env(touch, 0.01, 1.5, 0, 1.5)
Multiply body_enved(body, amp_env)
Rescaler body_vol(x, 1, 0)
Multiply body_amped(body_enved, body_vol)

Noise noise()
Multiply noise_enved(noise, amp_env)
Rescaler noise_r(x, 0, 0.5)
Filter noise_filtered(noise_enved, 8000, noise_r)
Rescaler noise_amp_x(x, 0, 0.5)
Rescaler noise_amp_y(y, 0, 0.5)
Add noise_amp(noise_amp_x, noise_amp_y)
Multiply noise_amped(noise_filtered, noise_amp)

EnvelopeGenerator gate_trigger(touch, 0.01, 0.01, 0, 0)
Rescaler gate_decay(y, 0.1, 4)
AttackRelease gate(gate_trigger, .01, gate_decay)
Multiply noise_gated(noise_amped, gate)

Add drums(body_amped, noise_gated)

Pan output(drums, 0.5)