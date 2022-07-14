/*
Code licenced under GNU GPL Version 3
https://github.com/TranquilloCosta/Infinity_mirror_sk9822_x52
2021-01-03 code last modified
2022-07-14 minor changes: millis() rollover problem solved (IR receiver loop freezing after 49.7 days standby time)
*/

/*
Infinity mirror based on Ikea Ribba picture frame.
Code optimized for ATmega328P Arduino (nano) in 16 Mhz mode.
Assembly: Diode between Arduino and 5V DC. 47uF Capacitor between Arduino and IR receiver VS1838B.
52x SK9822 RGB LEDs stripe arrangement shematic (BACK VIEW):

                     (MIRROR TOP)
+--□---□---□---□---□---□---□---□---□---□---□---□---□--+
|                                                     |
□                                                     □ (LED 13)
|                                                     |
□                                                     □ (LED 12)
|                                                     |
□                                                     □ (LED 11)
|                                                     |
□                                                     □ (LED 10)
|                                                     |
□                                                     □ (LED  9)
|                                                     |
□                                                     □ (LED  8)
|                                                     |
□                                                     □ (LED  7)  (MIRROR LEFT)
|                                                     |
□                                                     □ (LED  6)
|                                                     |
□                                                     □ (LED  5)
|                                                     |
□                                                     □ (LED  4)
|                                                     |
□                                                     □ (LED  3)
|                                                     |
□                                                     □ (LED  2)
|                                                     |
□                                                     □ (LED  1)--DATA/CLOCK/5V(3A)/GND
|
+--□---□---□---□---□---□---□---□---□---□---□---□---□--- (LED 52) 5V(3A)/GND
                    (MIRROR BOTTOM)

Samsung AA59-00786A TV IR remote control buttons:

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

Animations loops:
   
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
*/
*/

#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;
long old_value;
uint32_t decode_result;
bool decode_status = false;
unsigned long resume_time;
unsigned long start_time;
bool overrun=false;
bool codefound=false;
bool power=false;
bool single_color=false;

#define button_power 0xE0E040BF
#define button_source 0xE0E0807F
#define button_1 0xE0E020DF
#define button_2 0xE0E0A05F
#define button_3 0xE0E0609F
#define button_4 0xE0E010EF
#define button_5 0xE0E0906F
#define button_6 0xE0E050AF
#define button_7 0xE0E030CF
#define button_8 0xE0E0B04F
#define button_9 0xE0E0708F
#define button_0 0xE0E08877
#define button_ttx_mix 0xE0E034CB
#define button_pre_ch 0xE0E0C837
#define button_vol_up 0xE0E0E01F
#define button_vol_down 0xE0E0D02F
#define button_mute 0xE0E0F00F
#define button_chlist 0xE0E0D629
#define button_prg_up 0xE0E048B7
#define button_prg_down 0xE0E008F7
#define button_menu 0xE0E058A7
#define button_smart_hub 0xE0E09E61
#define button_guide 0xE0E0F20D
#define button_tools 0xE0E0D22D
#define button_info 0xE0E0F807
#define button_return 0xE0E01AE5
#define button_exit 0xE0E0B44B
#define button_up 0xE0E006F9
#define button_down 0xE0E08679
#define button_left 0xE0E0A659
#define button_right 0xE0E046B9
#define button_enter 0xE0E016E9
#define button_red 0xE0E036C9
#define button_green 0xE0E028D7
#define button_yellow 0xE0E0A857
#define button_blue 0xE0E06897
#define button_e_manual 0xE0E0FC03
#define button_3d 0xE0E0F906
#define button_ad_subt 0xE0E0A45B
#define button_stop 0xE0E0629D
#define button_frev 0xE0E0A25D
#define button_play 0xE0E0E21D
#define button_pause 0xE0E052AD
#define button_ffwd 0xE0E012ED


#include <Adafruit_DotStar.h>

// The number of pixel on the board
#define NUMPIXELS 52 

//Use these pin definitions for the ItsyBitsy M4
#define DATAPIN    2
#define CLOCKPIN   4

int32_t hue1 = 0;
int16_t sat1 = 255;
int16_t val1 = 255;
int16_t brightness1 = 255;
int16_t brightness_w = 100;
int current_color = 1;
int pause = 0;
int active = 0;

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Colors from file raketenstart_raster_rot_weiss.xls
  const uint32_t colors_b[] = {0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  2,
  9,
  26,
  56,
  99,
  246,
  253,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  253,
  246,
  99,
  56,
  26,
  9,
  2,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0};

  const uint32_t blocks_x12_random_sort_a[] PROGMEM = {4,
  22,
  0,
  35,
  39,
  9,
  43,
  17,
  30,
  13,
  26,
  48};


  const uint32_t blocks_x12_random_sort_b[] PROGMEM = {8,
  25,
  3,
  38,
  42,
  12,
  47,
  21,
  34,
  16,
  29,
  51};



  const uint32_t leds_random_sort_x12[] PROGMEM = {1,
  5,
  0,
  8,
  9,
  2,
  10,
  4,
  7,
  3,
  6,
  11};

  const uint32_t leds_random_sort_x15[] PROGMEM = {4,
  13,
  1,
  9,
  3,
  0,
  12,
  6,
  10,
  11,
  5,
  7,
  8,
  14,
  2};

  const uint32_t leds_random_sort_x52[] PROGMEM = {12,
  14,
  18,
  7,
  51,
  43,
  35,
  36,
  28,
  10,
  41,
  29,
  1,
  20,
  40,
  38,
  48,
  16,
  39,
  45,
  31,
  47,
  44,
  8,
  15,
  22,
  5,
  25,
  0,
  34,
  42,
  46,
  6,
  27,
  4,
  50,
  13,
  11,
  37,
  21,
  33,
  19,
  30,
  49,
  3,
  17,
  9,
  32,
  26,
  2,
  24,
  23};


void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();

  if ( irrecv.decode(&results) ) {
    decode_result = results.value;
    decode_status = true;
  }
  else
  {
    decode_status = false;
  }
    
  strip.begin(); // Initialize pins for output
  strip.setBrightness(255);
  strip.show();  // Turn all LEDs off ASAP
  delay(1000); 
}

void loop() {

   
  if ( ( millis() - resume_time ) > 300) )
  {
    if (overrun==true)
    {
      //do nothing
      overrun=false;      
    }
    else if ( irrecv.decode(&results) ) 
    {
      decode_result = results.value;
      decode_status = true;
      resume_time = millis();
      irrecv.resume();
    }
    else
    {
      decode_status = false;
    }

//Serial.print("Nachher decode result: ");
//Serial.print(decode_result);
//Serial.println();
//Serial.print("active: ");
//Serial.print(active);
//Serial.println();
//Serial.print("pause: ");
//Serial.print(pause);
//Serial.println();
//delay(10000); 


    
  }
  if ( active && (decode_result == button_prg_up || decode_result == button_prg_down))
  {
    if (decode_result == button_prg_up)
    {
      if (active+1>9)
      {
        active=1;
      }
      else
      {
        active+=1;
      }
    }
    if (decode_result == button_prg_down)
    {
      if (active-1 < 1)
      {
        active=9;
      }
      else
      {
        active-=1;
      }
    }
    decode_result = 0;
    delay(2000);
    irrecv.resume();
  }
  if (! active || ! power){
    delay(100); 
  }
  
  if ( decode_status && (decode_result == button_stop)){
    decode_status = false;
    pause=0;
    active=0;
    for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,0,0,0);
      }
    strip.show();
    decode_result = 0;
  }
  else if ( decode_status && decode_result == button_power ){
    decode_status = false;
    pause=0;
    active=0;
    if (power == false)
    {
      power = true;
      single_color == false;

      if (brightness_w < 1) // so that one can at least see the mirror turn on
      {
        brightness_w  = 1;
      }
      int brightness_limit = pow((float)brightness_w/255,0.5)*255;
      int count = brightness_limit / 8;
      int brightness_temp = 0;
      int brightness_val;
      for (int i=0;i<count;i++)
      {
        brightness_temp += 8;
        brightness_val = pow((float)brightness_temp/255,2)*255;
        for(int x=0;x<strip.numPixels();x++)
        {
          strip.setPixelColor(x,brightness_val,brightness_val,brightness_val);   
        }
        strip.show();
        delay(33);
      }
    }
    else
    {
      power = false;
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,0,0,0);
      }
      strip.show();
      delay(2000);
    }
    decode_result = 0;
    irrecv.resume();
  }
  else if ( ( decode_status && decode_result == button_pause ) || pause)
  { 
    decode_status = false;
    pause = 1;
    if (decode_result == button_play)
    {
      pause = 0;
    }
    if (decode_result == button_vol_up)
    {
      int brightness_temp = pow((float)brightness1/255,0.5)*255;
      if (brightness_temp+8 > 255){
        brightness_temp = 255;
      }
      else 
      {
        brightness_temp +=8;
      }
      brightness1 = pow((float)brightness_temp/255,2)*255;
      if (brightness1 == 0) // doesn't increase anymore once smaller than 1!
      {
        brightness1 = 1;
      }
      strip.setBrightness(brightness1);
      strip.show(); 
    }
    if (decode_result == button_vol_down)
    {
      int brightness_temp = pow((float)brightness1/255,0.5)*255;
      if (brightness_temp-8 < 0){
        brightness_temp = 0;
      }
      else 
      {
        brightness_temp -=8;
      }
      brightness1 = pow((float)brightness_temp/255,2)*255;
      strip.setBrightness(brightness1);
      strip.show(); 
    }
    if (decode_result == button_ffwd)
    {
      uint32_t first_pixel_color = strip.getPixelColor(0);
      for(int x=0;x<strip.numPixels()-1;x++)
      {
        strip.setPixelColor(x,strip.getPixelColor(x+1));
      }
      strip.setPixelColor(strip.numPixels()-1, first_pixel_color);
      strip.show();
    }
    if (decode_result == button_frev)
    {
      uint32_t last_pixel_color = strip.getPixelColor(strip.numPixels()-1);
      for(int x=strip.numPixels()-1;x>0;x--)
      {
        strip.setPixelColor(x,strip.getPixelColor(x-1));
      }
      strip.setPixelColor(0, last_pixel_color);
      strip.show();
    }
    decode_result = 0;
  }
  else if ( decode_status && ( (decode_result == button_3d )  || ( decode_result == button_vol_up && ! active && single_color == false ) || ( decode_result == button_vol_down && ! active && single_color == false ) ) )
  {
    power = true;
    decode_status = false;
    active=0;
    single_color=false;
    for(int x=0;x<strip.numPixels();x++)
    {
      strip.setPixelColor(x,brightness_w,brightness_w,brightness_w);
    }
    strip.show();
    if (decode_result == button_vol_up)
    {
      int brightness_temp = pow((float)brightness_w/255,0.5)*255;
      if (brightness_temp+8 > 247){ // mirror freezes when max white brightness
        brightness_temp = 255;
      }
      else 
      {
        brightness_temp +=8;
      }
      brightness_w = pow((float)brightness_temp/255,2)*255;
      if (brightness_w == 0) // doesn't increase anymore once smaller than 1!
      {
        brightness_w = 1;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,brightness_w,brightness_w,brightness_w);
      }
      strip.show(); 
    }
    if (decode_result == button_vol_down)
    {
      int brightness_temp = pow((float)brightness_w/255,0.5)*255;
      if (brightness_temp-8 < 0){
        brightness_temp = 0;
      }
      else 
      {
        brightness_temp -=8;
      }
      brightness_w = pow((float)brightness_temp/255,2)*255;
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,brightness_w,brightness_w,brightness_w);
      }
      strip.show(); 
    }
    decode_result = 0;
  }
  else if ( decode_status && ( decode_result == button_red || decode_result == button_green || decode_result == button_yellow || decode_result == button_blue || decode_result == button_up || decode_result == button_down || decode_result == button_left || decode_result == button_right || ( decode_result == button_vol_up && ! active ) || ( decode_result == button_vol_down && ! active ) ) )
  {
    power = true;
    decode_status = false;
    active=0;
    single_color=true;
    strip.setBrightness(255);
    if (decode_result == button_red)
    {
      hue1=21845; // 65536/3
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_green)
    {
      hue1=0;
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_blue)
    {
      hue1=43690; // 65536*2/3
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_yellow)
    {
      hue1=10922+6*512; // 65536/6 + 6*512
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_vol_up)
    {
      if (val1+8 > 255){
        val1 = 255;
      }
      else {
        val1 +=8;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_vol_down)
    {
      if (val1-8 < 0){
        val1 = 0;
      }
      else {
        val1 -=8;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_up)
    {
      if (hue1+512 > 65536){
        hue1 = hue1 - 65536 + 512;
      }
      else {
        hue1 +=512;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_down)
    {
      if (hue1-512 < 0){
        hue1 = hue1 + 65536 - 512;
      }
      else {
        hue1 -=512;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_right)
    {
      if (sat1+8 > 255){
        sat1 = 255;
      }
      else {
        sat1 +=8;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    if (decode_result == button_left)
    {
      if (sat1-8 < 0){
        sat1 = 0;
      }
      else {
        sat1 -=8;
      }
      for(int x=0;x<strip.numPixels();x++)
      {
        strip.setPixelColor(x,strip.ColorHSV(hue1,sat1,val1));
      }
      strip.show(); 
    }
    decode_result = 0;
  }  

  else if ( decode_status )
  {
    power = true;
    decode_status = false;
    strip.setBrightness(brightness1);
    if (decode_result == button_1)
    {
      button1();
      active = 1;
    }
    if (decode_result == button_2)
    {
      button2();
      active = 2;
    }
    if (decode_result == button_3)
    {
      button3();
      active = 3;
    }
    if (decode_result == button_4)
    {
      button4();
      active = 4;
    }
    if (decode_result == button_5)
    {
      button5();
      active = 5;
    }
    if (decode_result == button_6)
    {
      button6();
      active = 6;
    }
    if (decode_result == button_7)
    {
      button7();
      active = 7;
    }
    if (decode_result == button_8)
    {
      button8();
      active = 8;
    }
    if (decode_result == button_9)
    {
      button9();
      active = 9;
    }
    if (decode_result == button_0)
    {
      button0();
      active = 10;
    }
  }
  else if ( active  && ! pause )
  { 
    decode_status = false;
    strip.setBrightness(brightness1);
    if (active == 1)
    {
      button1();
    }
    if (active == 2)
    {
      button2();
    }
    if (active == 3)
    {
      button3();
    }
    if (active == 4)
    {
      button4();
    }
    if (active == 5)
    {
      button5();
    }
    if (active == 6)
    {
      button6();
    }
    if (active == 7)
    {
      button7();
    }
    if (active == 8)
    {
      button8();
    }
    if (active == 9)
    {
      button9();
    }
    if (active == 10)
    {
      button0();
    }
  }
  
  

 
// GRÜN
// strip.setPixelColor(x,255,0,0);
//
// ROT
// strip.setPixelColor(x,0,255,0);
//
// BLAU
// strip.setPixelColor(x,0,0,255); 

// delay(3000);

}

void read_ir (int delay)
{
  decode_status = false;
  while ( ( millis() - start_time ) < delay )
  {
    if ( ( millis() - resume_time ) > 300 )
    {
      if ( irrecv.decode(&results) )
      {
        if(results.value == button_1 || results.value == button_2 || results.value == button_3 || results.value == button_4 || results.value == button_5 || results.value == button_6 || results.value == button_7 || results.value == button_8 || results.value == button_9 || results.value == button_red || results.value == button_green || results.value == button_yellow || results.value == button_3d || results.value == button_blue || results.value == button_0 || results.value == button_pause || results.value == button_stop || results.value == button_power || results.value == button_prg_up || results.value == button_prg_down)
        {
          decode_result = results.value;
          decode_status = true;
          resume_time = millis();
          irrecv.resume();
          overrun=true;
          active=0;
        }
      }     
    }  
  } 
}

    void button1()
    {
      strip.setBrightness(brightness1);
      random_color_splatters(5,0,40);
      decode_result = 0;
    }
    void button2()
    {
      random_colors(1, 1, 0, 0, 20);
      decode_result = 0;
    }
    void button3()
    {
      random_colors(2, 3, 0, 0, 5);
      decode_result = 0;
    }
    void button4()
    {
      axial_blocks_x12_lite(1, 1, 0, 200, 200); 
      decode_result = 0; 
    }
    void button5()
    {
      axial_blocks_x12_lite(5, 1, 0, 30, 30);
      decode_result = 0;
    }
    void button6()
    {
      curbs_takeoff(0);   
      strip.clear();
      strip.show();
      delay(115); 
      strip.setBrightness(brightness1);
      rainbow_takeoff(10,0.5,3);   
      rainbow_fadeout(10,0.25, 3);
      decode_result = 0;
      start_time = millis();
      read_ir(2000);
    }
    void button7()
    {
      rainbow(10, 1);
      decode_result = 0;
    }
    void button8()
    {
      wobbling_bars(2); 
      decode_result = 0;
      start_time = millis();
      read_ir(4000);
    }
    void button9()
    {
      if (current_color == 1)
      {
        current_color=3;
        single_pixels_single_color(0,0,255,50);
      }
      if (current_color == 2)
      {
        current_color=1;
        single_pixels_single_color(0,255,0,50);
      }
      if (current_color == 3)
      {
        current_color=2;
        single_pixels_single_color(255,0,0,50);
      }
      decode_result = 0;
    }
    void button0()
    {
      strip.setBrightness(255);
      playall();
      strip.setBrightness(brightness1);
      decode_result = 0;
      start_time = millis();
      read_ir(5000);
    }
    
void playall() {
// Very dimmed level White
// 60 = 2 sekunden x 30 frames
for(int b=0;b<60;b++)
{
  int lightness_val = pow((float)(b+1)/60,2)*15;
  for(int x=0;x<strip.numPixels();x++)
  {
    strip.setPixelColor(x,strip.ColorHSV(0,0,lightness_val));   
  }
  strip.show(); 
  // 2000 / 150, total 2 sekunden
  delay(33);
}

  delay(2000);
  
  // parameters int runs, int pixels_parallel, fadeout_duration, int wait_begin, int wait_end
  // base delay is ~20ms
  random_colors(1, 1, 0, 40, 100);
  random_colors(1, 1, 0, 10, 40);
  random_colors(2, 1, 0, 0, 10);
  random_colors(3, 2, 0, 0, 10);
  random_colors(4, 3, 0, 0, 15);
  random_colors(90, 3, 1500, 0, 5);
  
  wobbling_bars(2);

  strip.clear();
  strip.show();
  delay(666); 
    
  // white random sort single pixels
  single_pixels_single_color(127,127,127,50); 
  delay(500);

// SHOULD CHANGE DOTSTAR_BGR to DOTSTAR BRG AND MODIFY/CORRECT SOME PARTS OF THE PROGRAM.  
  // red color random sort single pixels
   single_pixels_single_color(0,255,0,50);
  delay(500);

// SHOULD CHANGE DOTSTAR_BGR to DOTSTAR BRG AND MODIFY/CORRECT SOME PARTS OF THE PROGRAM.  
  // green color random sort single pixels
  single_pixels_single_color(255,0,0,50);
  delay(500);
  
  // blue color random sort single pixels
  single_pixels_single_color(0,0,255,50);
  delay(500);
  
  strip.clear();
  strip.show(); 
  delay(666);

// White
strip.setBrightness(0);

// 150 = 5 sekunden x 30 frames
for(int b=0;b<90;b++)
{
  for(int x=0;x<strip.numPixels();x++)
  {
    strip.setPixelColor(x,255,255,255);   
  }
  strip.setBrightness(255*b/150);
  strip.show(); 
  // 5000 / 150, total 5 sekunden
  delay(33);
}

// Fade to red  
// 60 = 2 sekunden x 30 frames
for(int b=0;b<60;b++)
{
  for(int x=0;x<strip.numPixels();x++)
  {
    strip.setPixelColor(x,255-255*b/60,255,255-255*b/60);   
  }
  strip.show(); 
  // 2000 / 60, total 3 sekunden
  delay(33);
}

//for(int x=0;x<strip.numPixels();x++)
//{  
//  strip.setPixelColor(x,255,255,255); 
//}

// Strobe, dimmed, accelerating 
// ca 4 sekunden * 3...8 strobe light = total 20 loops 
for(int i=0;i<25;i++)
{
  for(int x=0;x<strip.numPixels();x++)
  {  
    strip.setPixelColor(x,0,255,0); 
  }
  strip.setBrightness(255-255*i/25);
  strip.show(); 
  // 5000 / 40, total 3 sekunden
  delay(165-150*i/25);
  for(int x=0;x<strip.numPixels();x++)
  {  
    strip.setPixelColor(x,255,255,255); 
  }
  strip.setBrightness(90);
  strip.show(); 
  // Blitze immer 50ms
  delay(165-115);
}

// 15 more loops * 8 strobes per second
for(int i=0;i<15;i++)
{
  strip.setBrightness(0);
  strip.show(); 
  // 5000 / 40, total 3 sekunden
  delay(165-150);
  strip.setBrightness(90-40*i/15);
  strip.show(); 
  // Blitze immer 50ms
  delay(165-115);
}

// One bright, white flash.
// noch ein kurzer heller Blitz 0.165 Sekunden anschwellend, 0.33 Sekunden warten, 0.333 Sekunden abschwellend.
for(int i=0;i<5;i++)
{
  strip.setBrightness(90+165*i/5);
  strip.show();
  delay(33); 
}

for(int i=0;i<10;i++)
{
  strip.setBrightness(255-255*i/10);
  strip.show();
  delay(33);  
}

  strip.setBrightness(255);
  
  strip.clear();
  strip.show();
  delay(666); 

curbs_takeoff(0);   

  strip.clear();
  strip.show();
  delay(115); 
  
  rainbow_takeoff(10,1, 5);   
  
  rainbow_fadeout(10,0.25, 10);             // Flowing rainbow cycle along the whole strip
  
  strip.clear();
  strip.show();
  delay(666);
 
  axial_blocks_x12_lite(1, 1, 0, 200, 200);
  axial_blocks_x12_lite(2, 1, 0, 30, 200);
  axial_blocks_x12_lite(25, 1, 1, 30, 30);

  strip.clear();
  strip.show();
  delay(666);

  random_color_splatters(120,1,40);

  strip.clear();
  strip.show();
  delay(333);
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait, int rotations) {  
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
// Anzahl Umläufe ändern: 3*65536
// += 256 erhöhen für schnellere Umläufe
  for(long firstPixelHue = 0; firstPixelHue < rotations*65536; firstPixelHue += 512) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow_fadeout(int wait, float fadein, int rotations) {  
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
// Anzahl Umläufe ändern: 3*65536
// += 256 erhöhen für schnellere Umläufe
  float dimm0 = 0;
  float dimm1 = 1;
  for(long firstPixelHue = 0; firstPixelHue < rotations*65536; firstPixelHue += 512) {
    if(firstPixelHue<fadein*65536){
      dimm0= (float)(firstPixelHue)/(65536*fadein);
    }
    if(firstPixelHue>=(rotations-1)*65536){
      dimm1= 1-(float)(firstPixelHue-65536*(rotations-1)+1032)/(65536);
    }
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, pow(dimm0,2)*255, pow(dimm1,3)*255)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow_takeoff(int wait, float fadeout, int rotations) {  
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
// Anzahl Umläufe ändern: 3*65536
// += 256 erhöhen für schnellere Umläufe
  float dimm1 = 1;
  for(long firstPixelHue = 0; firstPixelHue < rotations*65536; firstPixelHue += 512) {
    if(firstPixelHue>=((float)rotations-fadeout)*65536){
      dimm1= 1-(float)(firstPixelHue-65536*(float)(rotations-fadeout)+1032)/(65536);
    }
    for(int i=0; i<strip.numPixels()/4; i++) { // For each pixel on left edge...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / (strip.numPixels()));
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, pow(dimm1,2)*255, 255)));
    }
    
    for(int i=0; i<strip.numPixels()/4; i++) { // For each pixel on right edge...
      int i2 = strip.numPixels()*3/4 - i - 1;
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i2, strip.gamma32(strip.ColorHSV(pixelHue, pow(dimm1,2)*255, 255)));
    }
    for(int i=strip.numPixels()/4; i<strip.numPixels()/2; i++) { // upper edge
      strip.setPixelColor(i, strip.getPixelColor(12));
    }
    for(int i=strip.numPixels()*3/4; i<strip.numPixels(); i++) { // lower edge
      strip.setPixelColor(i, strip.getPixelColor(0));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rocket take-off with red and white bars
void curbs_takeoff(int wait) { 
  int loops = 6;
  int colors_count = (sizeof(colors_b)/sizeof(colors_b[0]));
  int counter = 0;
  for(int a=0; a<loops; a++) { // number of outer loops
    strip.clear(); 
    for (int b=0 ; b<colors_count; b++) { // 1x red bar 1x white bar
      counter++;
      
      for(int i=0; i<strip.numPixels()/4; i++) {
        int color_index = (b+i*8+52)%52;
        strip.setPixelColor(i, strip.Color(colors_b[color_index], 255, colors_b[color_index])); // For each pixel on left edge...
        
        int i2 = strip.numPixels()*3/4 - i - 1;
        strip.setPixelColor(i2, strip.Color(colors_b[color_index], 255, colors_b[color_index])); // For each pixel on right edge...
      }
      for(int i=strip.numPixels()/4; i<strip.numPixels()/2; i++) { // upper edge 
        strip.setPixelColor(i, strip.getPixelColor(12));
      }
      for(int i=strip.numPixels()*3/4; i<strip.numPixels(); i++) { // lower edge
        strip.setPixelColor(i, strip.getPixelColor(0));
      }
      strip.show(); // Update strip with new contents
      //int millisecs = wait + 50 - 50*(a*colors_count+b)/(loops * colors_count);
      int millisecs = wait + 50 - 50*counter/(312);
      delay(millisecs);  // Pause for a moment / accelerate
    }
  }

  // noch 2*2 schnelle loops
  loops = 4;
  colors_count = (sizeof(colors_b)/sizeof(colors_b[0]));
  for(int a=0; a<loops; a++) { // number of outer loops
    strip.clear(); 
    for (int b=0 ; b<colors_count; b++) { // 1x red bar 1x white bar
            
      for(int i=0; i<strip.numPixels()/4; i++) {
        int color_index = (b+i*8+52)%52;
        strip.setPixelColor(i, strip.Color(colors_b[color_index], 255, colors_b[color_index])); // For each pixel on left edge...
        
        int i2 = strip.numPixels()*3/4 - i - 1;
        strip.setPixelColor(i2, strip.Color(colors_b[color_index], 255, colors_b[color_index])); // For each pixel on right edge...
      }
      for(int i=strip.numPixels()/4; i<strip.numPixels()/2; i++) { // upper edge
        strip.setPixelColor(i, strip.getPixelColor(12));
      }
      for(int i=strip.numPixels()*3/4; i<strip.numPixels(); i++) { // lower edge
        strip.setPixelColor(i, strip.getPixelColor(0));
      }

      strip.show(); // Update strip with new contents
      int millisecs = wait +2;
      delay(millisecs);  // Pause for a moment
    }
  }

  // noch 3*2 schnelle loops, Rot wird zu Weiss, Helligkeit abnehmend
  loops = 6;
  colors_count = (sizeof(colors_b)/sizeof(colors_b[0]));
  counter = 0;
  //int brightness = 255;
  int brightness = brightness1;
  for(int a=0; a<loops; a++) { // number of outer loops
    strip.clear(); 
    for (int b=0 ; b<colors_count; b++) { // 1x red bar 1x white bar
      counter++;      
      
      
      
      
      for(int i=0; i<strip.numPixels()/4; i++) {
        int color_index = (b+i*8+52)%52;
        // 312=3x104 Farben STATISCH!
        int color_fade = colors_b[color_index] + (255-colors_b[color_index])*counter/(312*4);
        strip.setPixelColor(i, strip.Color(color_fade , 255, color_fade)); // For each pixel on left edge...
        
        int i2 = strip.numPixels()*3/4 - i - 1;
        strip.setPixelColor(i2, strip.Color(color_fade, 255, color_fade)); // For each pixel on right edge...
      }
      for(int i=strip.numPixels()/4; i<strip.numPixels()/2; i++) { // upper edge
        strip.setPixelColor(i, strip.getPixelColor(12));
      }
      for(int i=strip.numPixels()*3/4; i<strip.numPixels(); i++) { // lower edge
        strip.setPixelColor(i, strip.getPixelColor(0));
      }
      strip.show(); // Update strip with new contents
      int millisecs = wait +2;
      delay(millisecs);  // Pause for a moment
      brightness = pow(1-(float)(a*colors_count+b)/(loops*colors_count),2)*brightness1;
      //brightness = brightness * 0.999;
      strip.setBrightness(brightness);
    }
  }
  strip.setBrightness(255);
}

void wobbling_bars(int runs){
  // setup
  // front_led - back_led should be an even number!!!
  float front_led=34;
  float back_led=30;
  // Frames per second
  float framerate=36;
  float interval = (float)1000/framerate;
  // Base speed seconds per evolution
  float dt=8;
  // Revolutions per second
  float n=(float)1/dt;
  // Max speed to base speed ratio
  //\mathrm{for}\:x=\frac{\left(2vy-v\right)}{v} (y: multiple of the distance the base speed pixle goes, v:base speed)
  // https://www.symbolab.com/solver/equation-calculator/x%3D%20%5Cfrac%7B%5Cleft(2vy-v%5Cright)%7D%7Bv%7D
  int y = ( 52 / 4 - (front_led - back_led) / 2 ) / ( (front_led - back_led)/2 );
  int speed_ratio_max = 2 * y * - y;
  int front_led_start = front_led;
  int back_led_start = back_led;
  
  long hue = 0;
  int hue_intervall = 512;
  
  // run 0.5 loops before runaway starts
  // float loops = 0.875;
  // float loops = 0.125;
  float loops = 1.0;
  float t= loops / n;
  int frames= round(t*1000 / interval); 
  float pixel_interval = strip.numPixels()*loops / frames;
  float dimm1;
  for (int i=0;i<frames;i++){
    hue += hue_intervall;
    strip.clear();
    int front_round = round(front_led +0.5);
    int back_round = round(back_led -0.5);
    int led_count = front_round - back_round;
    int front_round_show = (front_round+52)%52;
    int back_round_show = (back_round+52)%52;
    // set all inner leds on the partial circle to 100% brightness
    if (i<frames/2){
      dimm1 = (float)i/(frames/2);
    }
    uint32_t color = strip.gamma32(strip.ColorHSV(hue,255,pow(dimm1,2)*255));
    for (int x=back_round_show;x<back_round_show+led_count;x++){
      strip.setPixelColor((x+52)%52, color);
      strip.setPixelColor((x+52+26)%52, color);
    }
    float dimm = 1-(front_round-front_led);
    // set front led brightness
    color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm1,2)*pow(dimm,2)*255));
    strip.setPixelColor(front_round_show, color);
    strip.setPixelColor((front_round_show+26)%52, color);
  
    dimm = 1-(back_led-back_round);
    // set back led brightness
    color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm1,2)*pow(dimm,2)*255));
    strip.setPixelColor(back_round_show, color);
    strip.setPixelColor((back_round_show+26)%52, color);
    front_led += pixel_interval;
    back_led += pixel_interval;
    strip.show();
    delay(interval);
  }
  for (int b=0;b<runs;b++){
  // outer loop increase pulsation 10 times and decrease 10 times
  // front end starts to chase back end, acelerate (0s-0.5s)
  for (int a=0; a<10; a++){
    // front end starts to chase back end, acelerate (0s-0.5s)
    float loops = 0.33333333333/13;
    float t= loops / n;
    int frames= round(t*1000 / interval);
    float pixel_interval_base = strip.numPixels()*loops / frames; 
    float pixel_interval = (strip.numPixels()/4-4)/2;
    for (int i=0;i<frames;i++){
      hue += hue_intervall;
      strip.clear();
      int front_round = round(front_led +0.5);
      int back_round = round(back_led -0.5);
      int led_count = front_round - back_round;
      int front_round_show = (front_round+52)%52;
      int back_round_show = (back_round+52)%52;
      // set all inner leds on the partial circle to 100% brightness (may be overwritten below with dimmed values)
      uint32_t color = strip.gamma32(strip.ColorHSV(hue));
      for (int x=back_round;x<back_round+led_count;x++){
        strip.setPixelColor((x+52)%52, color);
        strip.setPixelColor((x+52+26)%52, color);
      }
      
      float dimm = 1-(front_round-front_led);      
      // set front led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(front_round_show, color);
      strip.setPixelColor((front_round_show+26)%52, color);
  
      dimm = 1-(back_led-back_round);
      // set back led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(back_round_show, color);
      strip.setPixelColor((back_round_show+26)%52, color);
      front_led = front_led + pixel_interval_base + pixel_interval*(i+1)/frames*(a+1)/10;
      back_led = back_led + pixel_interval_base - pixel_interval*(i+1)/frames*(a+1)/10;
      strip.show();
      delay(interval);
    }      
    // max. speed 13x at half chase time, slow down again to base speed (0.5s-1s)
    //loops = 0.125; // 3 pixel
    t= loops / n;
    frames= round(t*1000 / interval); 
    //pixel_interval = strip.numPixels()*loops / frames;
    for (int i=0;i<frames;i++){
      hue += hue_intervall;
      strip.clear();
      int front_round = round(front_led +0.5);
      int back_round = round(back_led -0.5);
      int led_count = front_round - back_round;
      int front_round_show = (front_round+52)%52;
      int back_round_show = (back_round+52)%52;
      // set all inner leds on the partial circle to 100% brightness
      uint32_t color = strip.gamma32(strip.ColorHSV(hue));
      for (int x=back_round;x<back_round+led_count;x++){
        strip.setPixelColor((x+52)%52, color);
        strip.setPixelColor((x+52+26)%52, color);
      }
      float dimm = 1-(front_round-front_led);
      // set front led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(front_round_show, color);
      strip.setPixelColor((front_round_show+26)%52, color);
  
      dimm = 1-(back_led-back_round);
      // set back led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(back_round_show, color);
      strip.setPixelColor((back_round_show+26)%52, color);
      front_led = front_led + pixel_interval_base - (pixel_interval*(frames-i)/frames*(a+1)/10);
      back_led = back_led + pixel_interval_base + pixel_interval*(frames-i)/frames*(a+1)/10;
      strip.show();
      delay(interval);
    }
  }
  for (int a=0; a<10; a++){
    // front end starts to chase back end, acelerate (0s-0.5s)
    float loops = 0.33333333333/13;
    float t= loops / n;
    int frames= round(t*1000 / interval);
    float pixel_interval_base = strip.numPixels()*loops / frames; 
    float pixel_interval = (strip.numPixels()/4-4)/2;
    for (int i=0;i<frames;i++){
      hue += hue_intervall;
      strip.clear();
      int front_round = round(front_led +0.5);
      int back_round = round(back_led -0.5);
      int led_count = front_round - back_round;
      int front_round_show = (front_round+52)%52;
      int back_round_show = (back_round+52)%52;
      // set all inner leds on the partial circle to 100% brightness (may be overwritten below with dimmed values)
      uint32_t color = strip.gamma32(strip.ColorHSV(hue));
      for (int x=back_round;x<back_round+led_count;x++){
        strip.setPixelColor((x+52)%52, color);
        strip.setPixelColor((x+52+26)%52, color);
      }
      
      float dimm = 1-(front_round-front_led);      
      // set front led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(front_round_show, color);
      strip.setPixelColor((front_round_show+26)%52, color);
      
      dimm = 1-(back_led-back_round);
      // set back led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(back_round_show, color);
      strip.setPixelColor((back_round_show+26)%52, color);
      front_led = front_led + pixel_interval_base + pixel_interval*(i+1)/frames*(10-a)/10;
      back_led = back_led + pixel_interval_base - pixel_interval*(i+1)/frames*(10-a)/10;
      strip.show();
      delay(interval);
    }      
    // max. speed 13x at half chase time, slow down again to base speed (0.5s-1s)
    //float loops = 0.3/13; // 0.3 pixel
    t= loops / n;
    frames= round(t*1000 / interval); 
    //pixel_interval = strip.numPixels()*loops / frames;
    for (int i=0;i<frames;i++){
      hue += hue_intervall;
      strip.clear();
      int front_round = round(front_led +0.5);
      int back_round = round(back_led -0.5);
      int led_count = front_round - back_round;
      int front_round_show = (front_round+52)%52;
      int back_round_show = (back_round+52)%52;
      // set all inner leds on the partial circle to 100% brightness
      uint32_t color = strip.gamma32(strip.ColorHSV(hue));
      for (int x=back_round;x<back_round+led_count;x++){
        strip.setPixelColor((x+52)%52, color);
        strip.setPixelColor((x+52+26)%52, color);
      }
      
      float dimm = 1-(front_round-front_led);
      // set front led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(front_round_show, color);
      strip.setPixelColor((front_round_show+26)%52, color);
  
      dimm = 1-(back_led-back_round);
      // set back led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(back_round_show, color);
      strip.setPixelColor((back_round_show+26)%52, color);
      front_led = front_led + pixel_interval_base - (pixel_interval*(frames-i)/frames*(10-a)/10);
      back_led = back_led + pixel_interval_base + pixel_interval*(frames-i)/frames*(10-a)/10;
      strip.show();
      delay(interval);
    }
  }
    // both ends run 1/8 turns at base speed
    loops = 0.125;
    t= loops / n;
    frames= round(t*1000 / interval); 
    pixel_interval = strip.numPixels()*loops / frames;
    for (int i=0;i<frames;i++){
      hue += hue_intervall;
      strip.clear();
      int front_round = round(front_led +0.5);
      int back_round = round(back_led -0.5);
      int led_count = front_round - back_round;
      int front_round_show = (front_round+52)%52;
      int back_round_show = (back_round+52)%52;
      // set all inner leds on the partial circle to 100% white and... 
      float dimm1 = 1;
      // ...dimm to zero in last 360° rotation
      if (b+1==runs){
        dimm1 = (1-((float)i+1)/frames);
      }
      for (int x=back_round_show;x<back_round_show+led_count;x++){
        strip.setPixelColor((x+52)%52, strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm1,2)*255 ) ) );
        strip.setPixelColor((x+52+26)%52, strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm1,2)*255 ) ) );
      }
      float dimm = 1-(front_round-front_led);
      if (b+1==runs){
        dimm = dimm*(1-((float)i+1)/frames);
      }
      // set front led brightness
      uint32_t color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(front_round_show, color);
      strip.setPixelColor((front_round_show+26)%52, color);
  
      dimm = 1-(back_led-back_round);
   
      if (b+1==runs){
        dimm = dimm*(1-((float)i+1)/frames);
      }
      // set back led brightness
      color = strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm,2)*255));
      strip.setPixelColor(back_round_show, color);
      strip.setPixelColor((back_round_show+26)%52, color);


      front_led += pixel_interval;
      back_led += pixel_interval;
      strip.show();
      delay(interval);
    }
} 
}

// pixel by pixel, random colors, random order. Function arguments for VARIABLE DELAY and OPTIONAL FADEOUT
// should include randomSeed(analogRead(0)) in void setup() above for more random colors
// uses above progmem-array of random sort led numbers (leds_random_sort_x52)
// arguments: runs: number of full pixel count update, pixels_parallel: number of pixels to update at the same time, fadeout_duration: fadeout time [ms] (0 or too small number means no fadeout), wait_end: delay between strip updates at the end of the execution, wait_begin: delay between strip updates (every pixel_parallel pixel change). 
void random_colors(int runs, int pixels_parallel, int fadeout_duration, int wait_end, int wait_begin){
  // parameters
  // approximate fade out duration in ms
  int wait=wait_begin;
  int fadeout_runs_before_end =  round((float)fadeout_duration / ( strip.numPixels() * (((float)wait+15) / pixels_parallel)));
  if (fadeout_runs_before_end > runs){
    fadeout_runs_before_end = runs;
  }
  bool decrease_delay = false;
  if (wait_begin > wait_end){
    decrease_delay = true;
  }
  float dimm1=1;
  for (int b=0;b<runs;b++){
      for(int x=0;x<strip.numPixels();x+=pixels_parallel){
        // fade to black during variable runs before filling all lines with all black lines in the last run
        if (b>=(runs-1-fadeout_runs_before_end)&&b<(runs-1)){
          dimm1 = 1-((float)x+1+strip.numPixels()*(b-runs+fadeout_runs_before_end+1))/(fadeout_runs_before_end*strip.numPixels());
        }
        if (1){
          wait = wait_begin - (wait_begin - wait_end)*(b*strip.numPixels()+(float)x)/(runs*strip.numPixels());
        }
        for (int a=0;a<pixels_parallel;a++){
          int offset = x+a;
          int led_number = pgm_read_word(leds_random_sort_x52 + offset); 
          int hue = random(65536);
          //int brightness_val = pow(random(100)/(float)100,2)*255;
          //strip.setPixelColor(led_number,pow(dimm1,2)*brightness_val,0,0);   
          //strip.setPixelColor(led_number,pow(dimm1,2)*brightness_val,pow(dimm1,2)*brightness_val,pow(dimm1,2)*brightness_val);   
          strip.setPixelColor(led_number,strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm1,2)*255)));
        }
        strip.show();
        delay(wait); 
     }
     // no need to fade, just let the last run replace the circle with black lines
     //if (b==(runs-2)){
     //  dimm1 = 0;
     //}
  }
}

void random_color_splatters(int runs, bool fadeout, int wait){
  float dimm=1;
  for (int b=0;b<runs;b++){
    // half size of color bar -1 center point that will be surrounded by 1 white pixel!!!
    int size = random(2,8);
    // random hue
    int hue = random(65536);
    // random center point of the
    int center = random(0,51);
    // white border brightness (0-255)  
    int brightness = 15;
    // center point
    for (int y=0;y<1;y++){
      strip.setPixelColor(center+y*96,strip.gamma32(strip.ColorHSV(hue)));
    }
    strip.show();
    //delay (wait);
    for (int a=0;a<size;a++){
      for (int y=0;y<1;y++){  
        strip.setPixelColor((center+a+52)%52+y*96,strip.gamma32(strip.ColorHSV(hue,255,dimm*255)));
        strip.setPixelColor((center-a+52)%52+y*96,strip.gamma32(strip.ColorHSV(hue,255,dimm*255)));
      }
      for (int y=0;y<1;y++){  
        strip.setPixelColor((center+a+1+52)%52+y*96,strip.ColorHSV(0,0,dimm*brightness));
        strip.setPixelColor((center-a-1+52)%52+y*96,strip.ColorHSV(0,0,dimm*brightness));
      }
      if ( fadeout && b >= runs-38 ){
        int limit = 51*(float)(b+1-runs+38)/38;
        random_fill_black_52(limit);
      }      
      strip.show();
      delay(wait); 
    }
    if ( fadeout && b >= runs-18 ){
      dimm=pow((1-(float)(b+1-runs+18)/18),2);
      strip.setBrightness(dimm*255);
    }
  }
  strip.setBrightness(255);
}

// single color random sort single pixels. Arguments: int green/red/blue (0-255), int wait [ms].
void single_pixels_single_color(int green, int red, int blue, int wait){
 for(int x=0;x<strip.numPixels();x++)
  {
    int led_number = pgm_read_word(leds_random_sort_x52 + x);
    strip.setPixelColor(led_number,green,red,blue);
    strip.show(); 
    delay(wait);
  }
}

// groups of 4 and 5 lines, random colors, random update order
void axial_blocks_x12_lite(int runs, int lines_parallel, bool goblack, int wait_end, int wait_begin){
  int wait=wait_begin;
  bool decrease_delay = false;
  if (wait_begin > wait_end){
    decrease_delay = true;
  }
  float dimm=1;
  float dimm1=1;
  // divisor (1,2,3,4,6,8,12) of 24 leds each edge (divisor 6 means 6x4 = 24 blocks total)
  for (int r=0;r<runs;r++){
    // no need to fade, just let the last run replace the circle with black lines
    if (goblack && r==(runs-1)){
      dimm1=0;
    }
    for(int i=0;i<12;i+=lines_parallel){
      if (decrease_delay){
        wait = wait_begin - (wait_begin - wait_end)*(float)(i+1+r*12)/(12*(runs));
      }
      int hue = random(65536);
      for (int a=0;a<lines_parallel;a++){
        int offset = i+a;
        int led_number = pgm_read_word(blocks_x12_random_sort_a +offset);
        int led_count = pgm_read_word(blocks_x12_random_sort_b +offset) - led_number +1;
        for(int x=0;x<led_count;x++){
          strip.setPixelColor(led_number+x,strip.gamma32(strip.ColorHSV(hue, 255, pow(dimm*dimm1,2)*255)));          
        }  
      }
      strip.show();
      delay(wait);
    }
  }
}

// fill strip with black. arguments: int limit (num of pixels: 1-52).
void random_fill_black_52(int limit){
  for (int x=0;x<limit;x++){
    int led_number = pgm_read_word(leds_random_sort_x52 + x);
    strip.setPixelColor(led_number, strip.Color(0,0,0));
  }
}

void max_brightness_test() {
  for(int x=0;x<strip.numPixels();x++)
  {
    strip.setPixelColor(x,255,255,255);   
  }
  strip.setBrightness(255);
  strip.show(); 

  delay(30000);
}

// 52 LEDs ~2.5A peak. Use multimeter in 10A mode.
void max_brightness_progressive_test() {
  for(int x=0;x<strip.numPixels();x++)
  {
    strip.setPixelColor(x,255,255,255);     
    strip.show(); 
    delay(1000);
  }
  delay(500);
}
