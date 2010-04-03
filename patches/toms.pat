Input x(0)
Input y(1)
Input touch(2)

Within within1(x, -1.0, -0.6)
Within within2(x, -0.6, -0.2)
Within within3(x, -0.2,  0.2)
Within within4(x,  0.2,  0.6)
Within within5(x,  0.6,  1.0)

Multiply trigger1(touch, within1)
Multiply trigger2(touch, within2)
Multiply trigger3(touch, within3)
Multiply trigger4(touch, within4)
Multiply trigger5(touch, within5)

Sample tom1("samples/tom1.wav", 261.2, trigger1, 1, 1)
Sample tom2("samples/tom2.wav", 261.2, trigger2, 1, 1)
Sample tom3("samples/tom3.wav", 261.2, trigger3, 1, 1)
Sample tom4("samples/tom4.wav", 261.2, trigger4, 1, 1)
Sample tom5("samples/tom5.wav", 261.2, trigger5, 1, 1)

Rescaler yvol(y, 0, 1)
SampleAndHold vol1(yvol, trigger1)
SampleAndHold vol2(yvol, trigger2)
SampleAndHold vol3(yvol, trigger3)
SampleAndHold vol4(yvol, trigger4)
SampleAndHold vol5(yvol, trigger5)

Multiply tom1_amped(tom1, vol1)
Multiply tom2_amped(tom2, vol2)
Multiply tom3_amped(tom3, vol3)
Multiply tom4_amped(tom4, vol4)
Multiply tom5_amped(tom5, vol5)

Pan tom1_p(tom1_amped, 0.2)
Pan tom2_p(tom2_amped, 0.35)
Pan tom3_p(tom3_amped, 0.5)
Pan tom4_p(tom4_amped, 0.65)
Pan tom5_p(tom5_amped, 0.8)

StereoAdd mix1  (tom1_p, tom2_p)
StereoAdd mix2  (mix1,   tom3_p)
StereoAdd mix3  (mix2,   tom4_p)
StereoAdd output(mix3,   tom5_p)