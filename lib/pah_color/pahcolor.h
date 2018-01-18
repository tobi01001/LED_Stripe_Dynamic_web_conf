#ifndef pah_color_h
#define pah_color_h

#include <Arduino.h>

typedef struct pah_colorvalues {

  uint8_t startColorR;
  uint8_t startColorG;
  uint8_t startColorB;

  uint8_t mid1ColorR;
  uint8_t mid1ColorG;
  uint8_t mid1ColorB;

  uint8_t mid2ColorR;
  uint8_t mid2ColorG;
  uint8_t mid2ColorB;

  uint8_t mid3ColorR;
  uint8_t mid3ColorG;
  uint8_t mid3ColorB;

  uint8_t endColorR;
  uint8_t endColorG;
  uint8_t endColorB;

  int16_t startValue;
  int16_t midValue;
  int16_t endValue;

} pah_colorvalues;

class pah_color {

public:

    pah_color (int16_t startValue=0, int16_t midValue=255, int16_t endValue=500,
               uint32_t startColor=0x00ffff, uint32_t mid1Color=0x1e50ff,
               uint32_t mid2Color= 0x28ff3c, uint32_t mid3Color=0xA0800A,
               uint32_t endColor=  0xFF4500);
    pah_color (int16_t startValue=0, int16_t midValue=255, int16_t endValue=500,
               uint8_t startR=0  , uint8_t startG=255 , uint8_t startB=255,
               uint8_t mid1R=30  , uint8_t mid1G= 80  , uint8_t mid1B= 255,
               uint8_t mid2R=40  , uint8_t mid2G= 255 , uint8_t mid2B= 60,
               uint8_t mid3R=160 , uint8_t mid3G= 128 , uint8_t mid3B= 10,
               uint8_t endR=255  , uint8_t endG=  69  , uint8_t endB=  0);
    pah_color (void);
    ~pah_color();

    void
      setColorvalues( uint8_t startR, uint8_t startG, uint8_t startB,
                      uint8_t mid1R, uint8_t mid1G, uint8_t mid1B,
                      uint8_t mid2R, uint8_t mid2G, uint8_t mid2B,
                      uint8_t mid3R, uint8_t mid3G, uint8_t mid3B,
                      uint8_t endR, uint8_t endG, uint8_t endB),

      setColorvalues( uint32_t start, uint32_t mid1, uint32_t mid2,
                      uint32_t mid3, uint32_t end),

      setColorStart (uint8_t r, uint8_t g, uint8_t b),
      setColorMid1  (uint8_t r, uint8_t g, uint8_t b),
      setColorMid2  (uint8_t r, uint8_t g, uint8_t b),
      setColorMid3  (uint8_t r, uint8_t g, uint8_t b),
      setColorEnd   (uint8_t r, uint8_t g, uint8_t b),

      setStepValues (int16_t start, int16_t mid, int16_t end);

   uint32_t calcColorValue  (int16_t currentValue),
            get32BitColor   (uint8_t r, uint8_t g, uint8_t b);

   uint8_t getRed  (uint32_t Color),
           getGreen(uint32_t Color),
           getBlue (uint32_t Color);


private:


    pah_colorvalues pahColorValues;

    uint8_t interpol(float u, uint8_t c1, uint8_t c2, uint8_t c3);



};

#endif
