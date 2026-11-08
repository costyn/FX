#ifndef FX_H
#define FX_H

#include <FastLED.h>
#include <Arduino.h>
#include <easing.h>

// Clock used for all animation timing. LEDswarm passes the mesh-synchronised
// clock here so every node computes the same frame. Defaults to millis().
typedef uint32_t (*FXClock)();

class FX {
  public:
    FX(CRGB * leds, uint16_t meshNumLeds );
    FX(CRGB * leds, CRGB * matrixLeds, uint16_t meshNumLeds );

    void spin();
    void FillLEDsFromPaletteColors() ;
    void fadeGlitter() ;
    void discoGlitter() ;
    void strobe1();
    void Fire2012();
    void twirlers(uint8_t numTwirlers, bool opposing );
    void heartbeat();
    void drawHeart(int brightness);
    void fastLoop(bool reverse);
    void fillnoise8(uint8_t speed, uint8_t scale, boolean colorLoop );
    void bounceBlend();
    void jugglePal();
    void pulse3();
    void threeSinPal();
    void cylon();
    void fireStripe() ;

    void fillGradientRing( int startLed, CHSV startColor, int endLed, CHSV endColor );
    void fillSolidRing( int startLed, int endLed, CHSV color );
    void fadeall(uint8_t fade_all_speed);
    void brightall(uint8_t bright_all_speed);
    void addGlitter( fract8 chanceOfGlitter);
    void addColorGlitter( fract8 chanceOfGlitter);

    void setled(int x, int y, CHSV c);
    void setMeshNumLeds(uint16_t meshNumLeds);
    void setTempo(uint16_t tempo);
    uint16_t getTempo();
    uint16_t mod(uint16_t x, uint16_t m);
    void setCurrentPalette(CRGBPalette16 currentPalette);
    void setAlone(boolean gAlone);

    // Time source. All FX patterns use these instead of FastLED's beat
    // functions, because current FastLED hard-wires those to its own millis()
    // and ignores USE_GET_MILLISECOND_TIMER.
    void setClock(FXClock clock);
    uint32_t now();
    uint8_t beat8(accum88 bpm, uint32_t timebase = 0);
    uint8_t beatsin8(accum88 bpm, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0);
    uint16_t beatsin16(accum88 bpm, uint16_t lowest = 0, uint16_t highest = 65535, uint32_t timebase = 0, uint16_t phase_offset = 0);

    uint8_t QuadraticEaseIn8( uint8_t p );
    uint8_t CubicEaseIn8( uint8_t p );
    uint8_t mappedEase8InOutQuad( uint8_t p );

  private:
    uint32_t clockOffset();

    CRGB * _leds = nullptr;
    CRGB * _matrixLeds = nullptr;
    CRGBPalette16 _currentPalette;
    uint16_t _meshNumLeds = 0;
    uint16_t _tempo = 120;
    boolean _alone = true;
    FXClock _clock = nullptr;
};
#endif
