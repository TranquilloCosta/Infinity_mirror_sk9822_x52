# Infinity_mirror_sk9822_x52

<b>Arduino based led driver with 10 different modes. For decoration or lighting.</b>  

IR remote controlled led controller with ten different static and animated modes. For decoration or lighting.

Please note: Code available for free usage under under GNU GPL Version 3 licence.
https://github.com/TranquilloCosta/Infinity_mirror_sk9822_x52

<b>Code usage example:</b>

Infinity mirror based on Ikea Ribba picture frame. Inspired by https://www.instructables.com/IKEA-Infinity-Mirror/

- Code optimized for ATmega328P Arduino (nano) in 16 Mhz mode.
- Material: IKEA Ribba picture frame, Mirropane Chrome Spy 4 MM 2-way mirror, regular mirror, LED strip 52x SK9822 RGB LEDs, 47uF electrolytic capacitor, Shotkey diode.
- Assembly: Diode between Arduino and 5V DC. 47uF capacitor between Arduino and IR receiver VS1838B.

52x SK9822 RGB LEDs stripe arrangement shematic (BACK VIEW):

![alt text](https://github.com/TranquilloCosta/Infinity_mirror_sk9822_x52/blob/main/shematic.jpg?raw=true)

<b>Note</b>: No power injection necessary. Powering on both ends sufficent. No noticeable voltage drop over 26 LEDs.  
  
<b>IR Remote control buttons (Samsung AA59-00786A TV remote control)</b>:  
  
Power: On / off (standby)  
Digits: Animations 1-9  
0: Play all animations  
Vol+-: Brightness (press "pause")  
Up/down: Color tone adjustment  
Left/right: Saturisation adjustment  
Red/green/yellow/blue: Plain color  
3D: Plain white  
Play/pause: Freeze / resume animation  
FFW/FRW: Pattern rotation (press "pause")  
  
<b>Animations loops</b>:  
  
1: «Color splashes»  
2:  Random colors slow  
3:  Random colors fast  
4: «Color bars» slow  
5: «Color bars» fast  
6: «Rocket start»  
7:  Color wheel  
8: «Wobbling bars»  
9:  RGB color change  
0:  Play all Animations  
  
<b>Infinity mirror specification</b>:  
  
Input voltage: 5V DC  
Max. peak current: 3‘150 mA  
Max. peak power: 15.75 W  
Standby power: 0.36 W  
