#include "FX.h"

FX::FX(CRGB * gLeds ){
  leds = gLeds;
}

FX::FX(CRGB * gLeds, CRGB * gMatrixLeds ){
  leds = gLeds;
  _matrixLeds = gMatrixLeds;
}


#define STEPS       3   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes

void FX::FillLEDsFromPaletteColors() {
  static uint8_t startIndex = 1;  // initialize at start
  static boolean firstPatternIteration = true; 
  const int8_t flowDir = -1 ;

  if( firstPatternIteration ) {
    startIndex = 1 ;
    firstPatternIteration = false ;   // reset flag
  }

  startIndex += flowDir ;

  uint8_t colorIndex = startIndex ;

  for ( uint8_t i = 0; i < _meshNumLeds; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, 255, LINEARBLEND );
    colorIndex += STEPS;
  }

  // add extra glitter during "fast"
  // if ( taskCurrentPatternRun.getInterval() < 10 ) {
  //   addGlitter(250);
  // } else {
  //   addGlitter(25);
  // }

// DEBUG_PRINT("maxBright: ");
// DEBUG_PRINTLN(maxBright);

//  taskCurrentPatternRun.setInterval( beatsin16( _tempo, 1500, 50000) ) ; // microseconds
// taskCurrentPatternRun.setInterval( beatsin16( _tempo, 5, 50 ) ) ;
}



void FX::fadeGlitter() {
  addGlitter(90);
  FastLED.show();
  fadeToBlackBy(leds, _meshNumLeds, 200);
}



void FX::discoGlitter() {
  fill_solid(leds, _meshNumLeds, CRGB::Black);
  addGlitter(240);
}



#ifdef RT_STROBE1
void FX::strobe1() {
  if ( tapTempo.beatProgress() > 0.95 ) {
    fill_solid(leds, _meshNumLeds, CRGB::White ); // yaw for color
  } else if ( tapTempo.beatProgress() > 0.80 and tapTempo.beatProgress() < 0.85 ) {
    //    fill_solid(leds, _meshNumLeds, CRGB::White );
  } else {
    fill_solid(leds, _meshNumLeds, CRGB::Black); // black
  }
  FastLED.setBrightness( maxBright ) ;
}
#endif



#ifdef RT_FIRE2012
#define COOLING  55
#define SPARKING 120
#define FIRELEDS round( _meshNumLeds / 2 )

// TODO: replace with original Fire2012 for LED strips

// Adapted Fire2012. This version starts in the middle and mirrors the fire going down to both ends.
// Works well with the Adafruit glow fur scarf.
// FIRELEDS defines the position of the middle LED.

void FX::Fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[FIRELEDS];

  // Step 1.  Cool down every cell a little
  for ( uint8_t i = 0; i < FIRELEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / FIRELEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = FIRELEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = FIRELEDS; j < _meshNumLeds; j++) {
    int heatIndex = j - FIRELEDS ;
    CRGB color = HeatColor( heat[heatIndex]);
    leds[j] = color;
  }

  //  "Reverse" Mapping needed:
  //    ledindex 44 = heat[0]
  //    ledindex 43 = heat[1]
  //    ledindex 42 = heat[2]
  //    ...
  //    ledindex 1 = heat[43]
  //    ledindex 0 = heat[44]

  for ( int j = 0; j <= FIRELEDS; j++) {
    int ledIndex = FIRELEDS - j ;
    CRGB color = HeatColor( heat[j]);
    leds[ledIndex] = color;
  }
  FastLED.setBrightness( maxBright ) ;
  // FastLED.show();

} // end Fire2012
#endif




#ifdef WHITESTRIPE
#define STRIPE_LENGTH 5
// This routines goes "over" other patterns, remembering/copying the
// pattern it is writing over and writing it back behind it.

void FX::whiteStripe() {
  static CRGB patternCopy[STRIPE_LENGTH] ;
  static int startLed = 0 ;

  if ( taskWhiteStripe.getInterval() != WHITESTRIPE_SPEED ) {
    taskWhiteStripe.setInterval( WHITESTRIPE_SPEED ) ;
  }

  if ( startLed == 0 ) {
    for (uint8_t i = 0; i < STRIPE_LENGTH ; i++ ) {
      patternCopy[i] = leds[i];
    }
  }

  // 36 40   44 48 52 56   60

  leds[startLed] = patternCopy[0] ;
  for (uint8_t i = 0; i < STRIPE_LENGTH - 2; i++ ) {
    patternCopy[i] = patternCopy[i + 1] ;
  }
  patternCopy[STRIPE_LENGTH - 1] = leds[startLed + STRIPE_LENGTH] ;

  fill_gradient(leds, startLed + 1, CHSV(0, 0, 255), startLed + STRIPE_LENGTH, CHSV(0, 0, 255), SHORTEST_HUES);

  startLed++ ;

  if ( startLed + STRIPE_LENGTH == _meshNumLeds - 1) { // LED nr 90 is index 89
    for (uint8_t i = startLed; i < startLed + STRIPE_LENGTH; i++ ) {
      leds[i] = patternCopy[i];
    }

    startLed = 0 ;
    taskWhiteStripe.setInterval(random16(4000, 10000)) ;
  }

  FastLED.setBrightness( maxBright ) ;
  // FastLED.show();
}
#endif





#if defined(RT_TWIRL1) || defined(RT_TWIRL2) || defined(RT_TWIRL4) || defined(RT_TWIRL6) || defined(RT_TWIRL2_O) || defined(RT_TWIRL4_O) || defined(RT_TWIRL6_O)
// Counter rotating twirlers with blending
// 1 twirler - 1 white = 120/1
// 2 twirler - 1 white = 120/1
// 4 twirler - 2 white = 120/2
// 6 twirler - 3 white = 120/3

void FX::twirlers(uint8_t numTwirlers, bool opposing ) {
  uint8_t pos = 0 ;
  uint8_t speedCorrection = 0 ;

  if ( numTwirlers == 1 ) {
    speedCorrection = 1 ;
  } else {
    speedCorrection = numTwirlers / 2 ;
  }
  uint8_t clockwiseFirst = lerp8by8( 0, _meshNumLeds, beat8( _tempo / speedCorrection )) ;
  const CRGB clockwiseColor = CRGB::White ;
  const CRGB antiClockwiseColor = CRGB::Red ;

  if ( opposing ) {
    fadeall(map( numTwirlers, 1, 6, 240, 180 ));
  } else {
    fadeall(map( numTwirlers, 1, 6, 240, 180 ));
  }

  for (uint8_t i = 0 ; i < numTwirlers ; i++) {
    if ( (i % 2) == 0 ) {
      // pos = (clockwiseFirst + round( _meshNumLeds / numTwirlers ) * i) % _meshNumLeds ;
      pos = mod((clockwiseFirst + round( _meshNumLeds / numTwirlers ) * i), _meshNumLeds) ;
      if ( leds[pos] ) { // FALSE if currently BLACK - don't blend with black
        leds[pos] = blend( leds[pos], clockwiseColor, 128 ) ;
      } else {
        leds[pos] = clockwiseColor ;
      }

    } else {

      if ( opposing ) {
        // uint8_t antiClockwiseFirst = _meshNumLeds - (lerp8by8( 0, _meshNumLeds, beat8( _tempo / speedCorrection ))) % _meshNumLeds ;
        uint8_t antiClockwiseFirst = _meshNumLeds - mod(lerp8by8( 0, _meshNumLeds, beat8( _tempo / speedCorrection )), _meshNumLeds) ;
        // pos = (antiClockwiseFirst + round( _meshNumLeds / numTwirlers ) * i) % _meshNumLeds ;
        pos = mod((antiClockwiseFirst + round( _meshNumLeds / numTwirlers ) * i),_meshNumLeds) ;
      } else {
        // pos = (clockwiseFirst + round( _meshNumLeds / numTwirlers ) * i) % _meshNumLeds ;
        pos = mod((clockwiseFirst + round( _meshNumLeds / numTwirlers ) * i),_meshNumLeds) ;
      }
      if ( leds[pos] ) { // FALSE if currently BLACK - don't blend with black
        leds[pos] = blend( leds[pos], antiClockwiseColor, 128 ) ;
      } else {
        leds[pos] = antiClockwiseColor ;
      }
    }
  }
} // end twirlers()
#endif


void FX::heartbeat() {
  const uint8_t hbTable[] = {
    25,
    61,
    105,
    153,
    197,
    233,
    253,
    255,
    252,
    243,
    230,
    213,
    194,
    149,
    101,
    105,
    153,
    197,
    216,
    233,
    244,
    253,
    255,
    255,
    252,
    249,
    243,
    237,
    230,
    223,
    213,
    206,
    194,
    184,
    174,
    162,
    149,
    138,
    126,
    112,
    101,
    91,
    78,
    69,
    62,
    58,
    51,
    47,
    43,
    39,
    37,
    35,
    29,
    25,
    22,
    20,
    19,
    15,
    12,
    9,
    8,
    6,
    5,
    3,
  };

  #define NUM_STEPS (sizeof(hbTable)/sizeof(uint8_t *)) //array size

  // beat8 generates index 0-255 (fract8) as per getBPM(). lerp8by8 interpolates that to array index:
  static float hbTempo ;

  if(_alone) {
    hbTempo = _tempo/2 ;
  } else {
    hbTempo = _tempo ;
  }
  uint8_t hbIndex = lerp8by8( 0, NUM_STEPS, beat8( hbTempo )) ;

  uint8_t lerp_maxBright = lerp8by8( 0, ATOM_MAX_BRIGHTNESS, MAX_BRIGHTNESS );

#ifdef ATOMMATRIX
  uint8_t brightness = lerp8by8( 0, ATOM_MAX_BRIGHTNESS, hbTable[hbIndex] ) ;
  drawHeart(brightness);
#else
  uint8_t brightness = lerp8by8( 0, maxBright, hbTable[hbIndex] ) ;
  fill_solid(leds, _meshNumLeds, CHSV(0, 255, brightness));
#endif
}

void FX::drawHeart(int brightness)
{
    memset(_matrixLeds, 0, sizeof(*_matrixLeds));
    CHSV c = CHSV(0, 255, brightness);
    CHSV alt_color = CHSV(0, 255, brightness);
    if (_alone) {
      alt_color = CHSV(180, 255, brightness);
    }
    // DEBUG_PRINTLN(brightness);

    setled(1,0,c);
    setled(3,0,c);

    setled(0,1,c);
    setled(1,1,alt_color);
    setled(2,1,c);
    setled(3,1,alt_color);
    setled(4,1,c);

    setled(0,2,c);
    setled(1,2,alt_color);
    setled(2,2,alt_color);
    setled(3,2,alt_color);
    setled(4,2,c);

    setled(1,3,c);
    setled(2,3,alt_color);
    setled(3,3,c);

    setled(2,4,c);
}

void FX::setled(int x, int y, CHSV c)
{
    _matrixLeds[y * 5 + x] = c;
}


#if defined(RT_FASTLOOP) || defined(RT_FASTLOOP2)

#define FL_LENGHT 20   // how many LEDs should be in the "stripe"
#define FL_MIDPOINT FL_LENGHT / 2
#define MAX_LOOP_SPEED 5

void FX::fastLoop(bool reverse) {
  static int16_t startP = 0 ;
  static uint8_t hue = 0 ;

  if ( ! reverse ) {
    startP = lerp8by8( 0, _meshNumLeds, beat8( _tempo )) ;  // start position
  } else {
    startP += map( sin8( beat8( _tempo / 4 )), 0, 255, -MAX_LOOP_SPEED, MAX_LOOP_SPEED + 1 ) ; // it was hard to write, it should be hard to undestand :grimacing:
  }

  fill_solid(leds, _meshNumLeds, CRGB::Black);
  fillGradientRing(startP, CHSV(hue, 255, 0), startP + FL_MIDPOINT, CHSV(hue, 255, 255));
  fillGradientRing(startP + FL_MIDPOINT + 1, CHSV(hue, 255, 255), startP + FL_LENGHT, CHSV(hue, 255, 0));

  hue++  ;
}
#endif


#if defined(RT_NOISE_LAVA) || defined(RT_NOISE_PARTY)
// FastLED library NoisePlusPalette routine rewritten for 1 dimensional LED strip
// - speed determines how fast time moves forward.  Try  1 for a very slow moving effect,
// or 60 for something that ends up looking like water.

// - Scale determines how far apart the pixels in our noise array are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise will be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// if current palette is a 'loop', add a slowly-changing base value

void FX::fillnoise8(uint8_t speed, uint8_t scale, boolean colorLoop ) {
  static uint8_t noise[MAX_MESH_LENGTH];

  static uint16_t x = random16();
  static uint16_t y = random16();
  static uint16_t z = random16();

  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;

  if ( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint8_t i = 0; i < _meshNumLeds; i++) {
    int ioffset = scale * i;

    uint8_t data = inoise8(x + ioffset, y, z);

    // The range of the inoise8 function is roughly 16-238.
    // These two operations expand those values out to roughly 0..255
    // You can comment them out if you want the raw noise data.
    data = qsub8(data, 16);
    data = qadd8(data, scale8(data, 39));

    if ( dataSmoothing ) {
      uint8_t olddata = noise[i];
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
      data = newdata;
    }

    noise[i] = data;
  }

  z += speed;

  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  static uint8_t ihue = 0;

  for (uint8_t i = 0; i < _meshNumLeds; i++) {
    // We use the value at the i coordinate in the noise
    // array for our brightness, and a 'random' value from _meshNumLeds - 1
    // for our pixel's index into the color palette.

    uint8_t index = noise[i];
    uint8_t bri =   noise[_meshNumLeds - i];
    // uint8_t bri =  sin(noise[_meshNumLeds - i]);  // more light/dark variation

    // if this palette is a 'loop', add a slowly-changing base value
    if ( colorLoop) {
      index += ihue;
    }

    // brighten up, as the color palette itself often contains the
    // light/dark dynamic range desired
    if ( bri > 127 ) {
      bri = 255;
    } else {
      bri = dim8_raw( bri * 2);
    }

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  ihue += 1;

  // FastLED.setBrightness( maxBright ) ;
  // FastLED.show();
}
#endif




#ifdef RT_BOUNCEBLEND
void FX::bounceBlend() {
  uint8_t speed = beatsin8( _tempo, 0, 255);
  static long runCounter = 0;
  static uint8_t startLed = 1 ;
  CHSV endclr = blend(CHSV(0, 255, 255), CHSV(160, 255, 255) , speed);
  CHSV midclr = blend(CHSV(160, 255, 255) , CHSV(0, 255, 255) , speed);
  fillGradientRing(startLed, endclr, startLed + _meshNumLeds / 2, midclr);
  fillGradientRing(startLed + _meshNumLeds / 2 + 1, midclr, startLed + _meshNumLeds, endclr);

  // FastLED.setBrightness( maxBright ) ;
  // FastLED.show();

  if ( (runCounter % 10) == 0 ) {
    startLed++ ;
    if ( startLed + 1 == _meshNumLeds ) startLed = 0  ;
  }
  runCounter++;
} // end bounceBlend()
#endif




/* juggle_pal
Originally by: Mark Kriegsman
Modified by: Andrew Tuline
Modified further by: Costyn van Dongen
Date: May, 2017
*/
#ifdef RT_JUGGLE_PAL
void FX::jugglePal() {                                             // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.

  static uint8_t    numdots =   4;                                     // Number of dots in use.
  static uint8_t   thisfade =   2;                                     // How long should the trails be. Very low value = longer trails.
  static uint8_t   thisdiff =  16;                                     // Incremental change in hue between each dot.
  static uint8_t    thishue =   0;                                     // Starting hue.
  static uint8_t     curhue =   0;                                     // The current hue
  static uint8_t   thisbeat =   35;                                     // Higher = faster movement.

  uint8_t secondHand = ( get_millisecond_timer() / 1000) % 60;                // Change '60' to a different value to change duration of the loop (also change timings below)
  static uint8_t lastSecond = 99;                             // This is our 'debounce' variable.

  if (lastSecond != secondHand) {                             // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: numdots = 1; thisbeat = _tempo / 2; thisdiff = 8;  thisfade = 8;  thishue = 0;   break;
      case  7: numdots = 2; thisbeat = _tempo / 2; thisdiff = 4;  thisfade = 12; thishue = 0;   break;
      case 25: numdots = 4; thisbeat = _tempo / 2; thisdiff = 24; thisfade = 50; thishue = 128; break;
      case 40: numdots = 2; thisbeat = _tempo / 2; thisdiff = 16; thisfade = 50; thishue = 0; break;
      case 52: numdots = 4; thisbeat = _tempo / 2; thisdiff = 24; thisfade = 80; thishue = 160; break;
    }
  }

  curhue = thishue;                                           // Reset the hue values.
  fadeToBlackBy(leds, _meshNumLeds, thisfade);

  for ( uint8_t i = 0; i < numdots; i++) {
    leds[beatsin16(thisbeat + i + numdots, 0, _meshNumLeds - 1)] += ColorFromPalette(RainbowColors_p, curhue, 255, LINEARBLEND); // Munge the values and pick a colour from the palette
    curhue += thisdiff;
  }

  // FastLED.setBrightness( maxBright ) ;
  // FastLED.show();

} // end jugglePal()
#endif


#ifdef RT_PULSE_3
#define PULSE_WIDTH 10
void FX::pulse3() {
  uint8_t width = beatsin8( constrain( _tempo * 2, 0, 255), 0, PULSE_WIDTH ) ; // can't use BPM > 255
  uint8_t hue = beatsin8( 1, 0, 255) ;
  static uint8_t middle = 0 ;

  if ( width == 1 ) {
    middle = taskCurrentPatternRun.getRunCounter() % 60 + taskCurrentPatternRun.getRunCounter() % 2;
  }

  fill_solid(leds, _meshNumLeds, CRGB::Black);
  fillGradientRing(middle - width, CHSV(hue, 255, 0), middle, CHSV(hue, 255, 255));
  fillGradientRing(middle, CHSV(hue, 255, 255), middle + width, CHSV(hue, 255, 0));

  FastLED.setBrightness( maxBright ) ;
  // FastLED.show() ;
}
#endif

#ifdef RT_PULSE_5
void FX::pulse5( uint8_t numPulses, boolean leadingDot) {
  uint8_t spacing = _meshNumLeds / numPulses ;
  uint8_t pulseWidth = (spacing / 2) - 1 ; // leave 1 led empty at max
  uint8_t middle = beatsin8( 5, 0, _meshNumLeds / 2) ;
  uint8_t width = beatsin8( _tempo, 0, pulseWidth) ;
  uint8_t hue = beatsin8( _tempo, 0, 30) ;

  fill_solid(leds, _meshNumLeds, CRGB::Black);

  for ( uint8_t i = 0 ; i < numPulses; i++ ) {
    uint8_t offset = spacing * i ;
    fillGradientRing(middle - width + offset, CHSV(hue, 255, 0), middle + offset, CHSV(hue, 255, 255));
    fillGradientRing(middle + offset, CHSV(hue, 255, 255), middle + width + offset, CHSV(hue, 255, 0));

    if ( leadingDot ) {  // abusing fill gradient since it deals with "ring math"
      fillGradientRing(middle - width + offset, CHSV(0, 255, 255), middle - width + offset, CHSV(0, 255, 255));
      fillGradientRing(middle + width + offset, CHSV(0, 255, 255), middle + width + offset, CHSV(0, 255, 255));
    }
  }

  FastLED.setBrightness( maxBright ) ;
  // FastLED.show() ;
}
#endif



#ifdef RT_THREE_SIN_PAL

/* three_sin_pal_demo
By: Andrew Tuline
Date: March, 2015
3 sine waves, one for each colour. I didn't take this far, but you could change the beat frequency and so on. . .
*/
#define MAXCHANGES 24
// Frequency, thus the distance between waves:
#define MUL1 7
#define MUL2 6
#define MUL3 5
void FX::threeSinPal() {
  static int wave1 = 0;                                                // Current phase is calculated.
  static int wave2 = 0;
  static int wave3 = 0;
  static long runCounter = 0;

  static CRGBPalette16 currentPalette(CRGB::Black);
  static CRGBPalette16 targetPalette(PartyColors_p);

  if ( runCounter % 2 == 0 ) {
    nblendPaletteTowardPalette( currentPalette, targetPalette, MAXCHANGES);

    wave1 += beatsin8(10, -4, 4);
    wave2 += beatsin8(15, -2, 2);
    wave3 += beatsin8(12, -3, 3);

    for (int k = 0; k < _meshNumLeds; k++) {
      uint8_t tmp = sin8(MUL1 * k + wave1) + sin8(MUL2 * k + wave2) + sin8(MUL3 * k + wave3);
      leds[k] = ColorFromPalette(currentPalette, tmp, 255);
    }
  }
  runCounter++;

  uint8_t secondHand = (get_millisecond_timer() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if ( lastSecond != secondHand) {
    lastSecond = secondHand;
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB u = CHSV( HUE_BLUE, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;

    switch (secondHand) {
      case  0: targetPalette = RainbowColors_p; break;
      case  5: targetPalette = CRGBPalette16( u, u, b, b, p, p, b, b, u, u, b, b, p, p, b, b); break;
      case 10: targetPalette = OceanColors_p; break;
      case 15: targetPalette = CloudColors_p; break;
      case 20: targetPalette = LavaColors_p; break;
      case 25: targetPalette = ForestColors_p; break;
      case 30: targetPalette = PartyColors_p; break;
      case 35: targetPalette = CRGBPalette16( b, b, b, w, b, b, b, w, b, b, b, w, b, b, b, w); break;
      case 40: targetPalette = CRGBPalette16( u, u, u, w, u, u, u, w, u, u, u, w, u, u, u, w); break;
      case 45: targetPalette = CRGBPalette16( u, p, u, w, p, u, u, w, u, g, u, w, u, p, u, w); break;
      case 50: targetPalette = CloudColors_p; break;
      case 55: targetPalette = CRGBPalette16( u, u, u, w, u, u, p, p, u, p, p, p, u, p, p, w); break;
      case 60: break;
    }
  }

  // FastLED.setBrightness( maxBright ) ;
  // FastLED.show();

} // threeSinPal()
#endif


#ifdef RT_CYLON
void FX::cylon() {
  //uint8_t ledPos = beatsin8( _tempo, 0, _meshNumLeds - 1 ) ;
  //uint8_t ledPos = beatsin8( 40, 0, _meshNumLeds - 1 ) ;
  //uint8_t ledPos = lerp8by8( 0, _meshNumLeds-1, ease8InOutQuad beatsin8( 40 ))) ;
  uint8_t ledPos = beatsin8( _tempo/2, 0, _meshNumLeds - 1 ) ;
  leds[ledPos] = CRGB::Orange ;
  uint8_t ledPos2 = beatsin8( _tempo/2, 0, _meshNumLeds - 1, 0, 40 ) ;
  leds[ledPos2] = CRGB::Red ;
  FastLED.setBrightness( maxBright ) ;
  // FastLED.show();
  fadeToBlackBy(leds, _meshNumLeds, 255);
}
#endif

#ifdef RT_FIRE_STRIPE
void FX::fireStripe() {
  static uint8_t ledPos = 0 ;
  static uint8_t hue = 0 ;
  int incomingByte = 0;   // for incoming serial data
  int val ;

  if (Serial.available() > 0) {
     // read the incoming byte:
     hue = Serial.parseInt();
     val = Serial.parseInt();

//     if (Serial.read() == '\n') {
       // say what you got:
       Serial.print("hue: ");
       Serial.print(hue);
       Serial.print("val: ");
       Serial.println(val);
  //   }
   }


  //CHSV colorStart = CHSV( beatsin8( 33, 0, 25), 255, beatsin8( 25, 10, 128));
  //CHSV colorEnd = CHSV( beatsin8( 55, 0, 25), 255, beatsin8( 128, 128, 255));
  CHSV colorStart = CHSV( hue, 25, val);
  CHSV colorEnd = CHSV( hue, 255, val);
  //CHSV colorEnd = CHSV( hue+64, 255, 255);

  fill_gradient(leds, ledPos, colorStart, ledPos + 30, colorEnd, SHORTEST_HUES ) ;

  FastLED.setBrightness( maxBright ) ;
  // FastLED.show();

//  hue++ ;
}
#endif


// Fill a gradient on a LED ring with any possible start positions.
// startLed and endLed may be negative (one or both), may be larger than _meshNumLeds (one or both)
// TODO:
// * remove floating point calculation; replace by linear interpolation?

void FX::fillGradientRing( int startLed, CHSV startColor, int endLed, CHSV endColor ) {
  // Determine actual start and actual end (normalize using modulo):
  int actualStart = mod(startLed + _meshNumLeds, _meshNumLeds)  ;
  int actualEnd = mod(endLed + _meshNumLeds, _meshNumLeds) ;

  // If beginning is at say 50, and end at 10, then we split the gradient in 2:
  // * one from 50-59
  // * one from 0-10
  // To determine which color should be at 59 and 0 we use the blend function:
  if ( actualStart > actualEnd ) {
    float ratio = 1.0 - float(actualEnd) / float(endLed - startLed) ; // cast to float otherwise the division won't work
    int normalizedRatio = round( ratio * 255 ) ; // determine what ratio of startColor and endColor we need at LED 0
    CHSV colorAtLEDZero = blend(startColor, endColor, normalizedRatio);

    fill_gradient(leds, actualStart, startColor, _meshNumLeds - 1, colorAtLEDZero, SHORTEST_HUES);
    fill_gradient(leds, 0, colorAtLEDZero, actualEnd, endColor, SHORTEST_HUES);
  } else {
    fill_gradient(leds, actualStart, startColor, actualEnd, endColor, SHORTEST_HUES);
  }
}

// This is a little convoluted and could probably be written better :)
void FX::fillSolidRing( int startLed, int endLed, CHSV color ) {
  // Determine actual start and actual end (normalize using custom modulo):
  int actualStart = mod(startLed + _meshNumLeds, _meshNumLeds)  ;
  int actualEnd = mod(endLed + _meshNumLeds, _meshNumLeds) ;

  // If beginning is at say 50, and end at 10, then we split the fill in 2:
  // * one from 50-59
  // * one from 0-10
  if ( actualStart > actualEnd ) {
    fill_solid(leds + actualStart, _meshNumLeds - actualStart, color);
    fill_solid(leds, actualEnd, color);
  } else {
    fill_solid(leds + actualStart, actualEnd - actualStart, color);
  }
} // end fillSolidRing()


#ifdef USING_MPU

// offset for aligning gyro "bottom" with LED "bottom" - depends on orientation of ring led 0 vs gyro - determine experimentally
#define OFFSET -4

// This routine needs pitch/roll information in floats, so we need to retrieve it separately
//  Suggestions how to fix this/clean it up welcome.
int lowestPoint() {
  Quaternion quat;        // [w, x, y, z]         quaternion container
  VectorFloat gravity;    // [x, y, z]            gravity vector
  float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  mpu.dmpGetQuaternion(&quat, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &quat);
  mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);

  float myYprP = (ypr[1] * 180 / M_PI) ;  // Convert from radians to degrees:
  float myYprR = (ypr[2] * 180 / M_PI) ;

  int targetLedPos = 0 ;
  static int currentLedPos = 0 ;
  int ratio = 0 ;
  int mySpeed = round( (abs(myYprP) + abs(myYprR) ) * 100 );
  taskLedModeSelect.setInterval( map( mySpeed, 0, 9000, 25, 5) ) ; // Why did I do this? So weird

  if ( myYprR < 0 and myYprP < 0 ) {
    ratio =  (abs( myYprP ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 0 , 14 );

  } else if ( myYprR > 0 and myYprP < 0 ) {
    ratio =  (abs( myYprR ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 15 , 29 );

  } else if ( myYprR > 0 and myYprP > 0 ) {
    ratio =  (abs( myYprP ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 30 , 44 );

  } else if ( myYprR < 0 and myYprP > 0 ) {
    ratio =  (abs( myYprR ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 45 , 60 );
  } else {
    DEBUG_PRINTLN(F("\tWTF\t")) ;  // This should never happen
  }
  targetLedPos = mod(targetLedPos + OFFSET, NUM_LEDS) ;

  if ( currentLedPos != targetLedPos ) {
    bool goClockwise = true ;

    // http://stackoverflow.com/questions/7428718/algorithm-or-formula-for-the-shortest-direction-of-travel-between-two-degrees-on

    if ( mod(targetLedPos - currentLedPos + NUM_LEDS, NUM_LEDS) < NUM_LEDS / 2) {  // custom modulo
      goClockwise = true ;
    } else {
      goClockwise = false  ;
    }

    // TODO: add QuadraticEaseInOut for movement
    if ( goClockwise ) {
      currentLedPos++ ;
      if ( currentLedPos > NUM_LEDS - 1 ) {
        currentLedPos = 0 ;
      }
    } else {
      currentLedPos-- ;
      if ( currentLedPos < 0 ) {
        currentLedPos = NUM_LEDS - 1 ;
      }
    }

  }

  return currentLedPos ;
}
#endif

void FX::fadeall(uint8_t fade_all_speed) {
  for (uint8_t i = 0; i < _meshNumLeds; i++) {
    leds[i].nscale8(fade_all_speed);
  }
}

void FX::brightall(uint8_t bright_all_speed) {
  for (uint8_t i = 0; i < _meshNumLeds; i++) {
    //leds[i] += leds[i].scale8(bright_all_speed) ;
    leds[i] += brighten8_video(bright_all_speed) ;
  }
}


void FX::addGlitter( fract8 chanceOfGlitter)
{
  for ( uint8_t i = 0 ; i < 5 ; i++ ) {
    if ( random8() < chanceOfGlitter) {
      leds[ random16(_meshNumLeds) ] += CRGB::White;
    }
  }
}

void FX::addColorGlitter( fract8 chanceOfGlitter)
{
  for ( uint8_t i = 0 ; i < 5 ; i++ ) {
    if ( random8() < chanceOfGlitter) {
      leds[ random16(_meshNumLeds) ] += CHSV(beat8(55), 255, 255);
    }
  }
}


#define LONG_PRESS_MIN_TIME 500  // minimum time for a long press
// #define SHORT_PRESS_MIN_TIME 70   // minimum time for a short press - debounce

// #ifdef BUTTON_PIN
//
// // longPressActive
// // advance ledMode
// // TODO: Replace horror show below with something like https://github.com/mathertel/OneButton
//
// void checkButtonPress() {
//   static unsigned long buttonTimer = 0;
//   static boolean buttonActive = false;
//
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     // Start the timer
//     if (buttonActive == false) {
//       buttonActive = true;
//       buttonTimer = millis();
//     }
//
//     // If timer has passed longPressTime, set longPressActive to true
//     if ((millis() - buttonTimer > LONG_PRESS_MIN_TIME) && (longPressActive == false)) {
//       longPressActive = true;
//     }
//
// #ifndef USING_MPU
//     if( longPressActive == true ) {
//           cycleBrightness() ;
//     }
// #endif
//
//
//   } else {
//     // Reset when button is no longer pressed
//     if (buttonActive == true) {
//       buttonActive = false;
//       if (longPressActive == true) {
//         longPressActive = false;
// //        taskLedModeSelect.enableIfNot() ;
//       } else {
//         if ( millis() - buttonTimer > SHORT_PRESS_MIN_TIME ) {
//           nextLedMode() ;
//         }
//       }
//     }
//   }
//
//   #ifdef BPM_BUTTON_PIN
//   if (digitalRead(BPM_BUTTON_PIN) == LOW) {
//     tapTempo.update(true);
// //    DEBUG_PRINTLN( _tempo ) ;
//   } else {
//     tapTempo.update(false);
//   }
//   #endif
//
//   serialEvent() ;
//   if (stringComplete) {
//     if( inputString.charAt(0) == 'p' ) {
//       inputString.remove(0,1);
//       uint8_t input = inputString.toInt();
//       if( input < NUMROUTINES ) {
//         ledMode = inputString.toInt();
//       } else {
//         DEBUG_PRINT("Ignoring input; value too high. NUMROUTINES: ");
//         DEBUG_PRINTLN( NUMROUTINES ) ;
//       }
//       DEBUG_PRINT("LedMode: ");
//       DEBUG_PRINTLN( ledMode ) ;
//     }
//     if( inputString.charAt(0) == 'b' ) {
//       inputString.remove(0,1);
//       tapTempo.setBPM(inputString.toInt());
//       DEBUG_PRINT("BPM: ");
//       DEBUG_PRINTLN( inputString.toInt() ) ;
//     }
//     if( inputString.charAt(0) == 'm' ) {
//       inputString.remove(0,1);
//       int requestedBrightness = inputString.toInt() ;
//       if( requestedBrightness > MAX_BRIGHTNESS) {
//         DEBUG_PRINT("Ignoring input; value too high. MAX_BRIGHTNESS: ");
//         DEBUG_PRINTLN( MAX_BRIGHTNESS ) ;
//         currentBrightness = MAX_BRIGHTNESS;
//       }
//       DEBUG_PRINT("currentBrightness: ");
//       DEBUG_PRINTLN( currentBrightness ) ;
//     }
//
//     // clear the string:
//     inputString = "";
//     stringComplete = false;
//   }
//
// }
// #endif

// Only use if no MPU present:
// TODO:
// #if !defined(USING_MPU) && defined(BUTTON_PIN)
// void FX::cycleBrightness() {
//   static uint8_t lerpBrightness ;

//   if ( taskCheckButtonPress.getRunCounter() % 100 ) {
//     currentBrightness = lerp8by8( 3, MAX_BRIGHTNESS, quadwave8( lerpBrightness )) ;
//     lerpBrightness++ ;
//     DEBUG_PRINTLN( currentBrightness ) ;
//   }

// //  taskLedModeSelect.disable() ;
// //  fill_solid(leds, _meshNumLeds, CRGB::White );
// //  FastLED.show() ;

// }
// #endif

// Custom modulo which always returns a positive number
uint16_t FX::mod(uint16_t x, uint16_t m) {
  return (x % m + m) % m;
}


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
// void serialEvent() {
//   while (Serial.available()) {
//     // get the new byte:
//     char inChar = (char)Serial.read();
//     Serial.print(inChar); // echo back what we got
//     // add it to the inputString:
//     inputString += inChar;
//     // if the incoming character is a newline, set a flag so the main loop can
//     // do something about it:
//     if (inChar == '\n') {
//       stringComplete = true;
//     }
//   }
// }

// #ifdef AUTOADVANCE
// void autoAdvanceLedMode() {
//   nextLedMode() ;
//   // if ( strcmp(routines[ledMode], "jugglePal") == 0 ) {
//   //   taskAutoAdvanceLedMode.delay( TASK_SECOND * 10 ) ;
//   // }
// }
// #endif
//
// void nextLedMode() {
//   fill_solid(leds, NUM_LEDS, CRGB::Black);
//   FastLED.show() ; // clear the canvas - prevent power fluctuations if a next pattern has lots of brightness
//   ledMode++;
//   if (ledMode == NUMROUTINES ) {
//     ledMode = 0;
//   }
//
//   DEBUG_PRINT(F("ledMode = ")) ;
//   DEBUG_PRINT( routines[ledMode] ) ;
//   DEBUG_PRINT(F(" mode ")) ;
//   DEBUG_PRINTLN( ledMode ) ;
// }

uint8_t FX::QuadraticEaseIn8( uint8_t p ) {
  int     i_100       = map(p, 0, _meshNumLeds, 0, 100); // Map current led p to percentage between 0 - 100
  AHFloat eased_float = QuadraticEaseInOut( (float)i_100 / (float)100); // Convert to value between 0 - 1
  int     eased_100   = (int)(eased_float * 100); // convert back to percentage
  return  map(eased_100, 0, 100, 0, _meshNumLeds);  // convert back to LED position
}

uint8_t FX::CubicEaseIn8( uint8_t p ) {
  int     i_100       = map(p, 0, _meshNumLeds, 0, 100); // Map current led p to percentage between 0 - 100
  AHFloat eased_float = CubicEaseInOut( (float)i_100 / (float)100); // Convert to value between 0 - 1
  int     eased_100   = (int)(eased_float * 100); // convert back to percentage
  return  map(eased_100, 0, 100, 0, _meshNumLeds);  // convert back to LED position
}

uint8_t FX::mappedEase8InOutQuad( uint8_t p ) {
  int     i_255       = map(p, 0, _meshNumLeds, 0, 255); // Map current led p to percentage between 0 - 100
  int     eased_255   = ease8InOutQuad( i_255 );
  return  map(eased_255, 0, 255, 0, _meshNumLeds);  // convert back to LED position
}

void FX::setAlone(boolean gAlone){
  _alone = gAlone;
}

void FX::setCurrentPalette(CRGBPalette16 gCurrentPalette) {
  currentPalette = gCurrentPalette;
}

void FX::setMeshNumLeds(uint16_t meshNumLeds) {
  _meshNumLeds = meshNumLeds;
}

void FX::setTempo(uint16_t tempo) {
  _tempo = tempo;
}

uint16_t FX::getTempo() {
  return _tempo ;
}