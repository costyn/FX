#ifndef FX_H
#define FX_H

#include <FastLED.h>
#include <Arduino.h>
#include <easing.h>

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

    uint8_t QuadraticEaseIn8( uint8_t p );
    uint8_t CubicEaseIn8( uint8_t p );
    uint8_t mappedEase8InOutQuad( uint8_t p );

  private:
    CRGB * _leds;
    CRGB * _matrixLeds;
    CRGBPalette16 _currentPalette;
    uint16_t _meshNumLeds;
    uint16_t _tempo;
    boolean _alone;
};
#endif
