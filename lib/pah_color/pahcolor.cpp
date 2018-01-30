
#include "pahcolor.h"


pah_color::pah_color (int16_t startValue, int16_t midValue, int16_t endValue,
           uint32_t startColor, uint32_t mid1Color,
           uint32_t mid2Color, uint32_t mid3Color,
           uint32_t endColor)
{

}
pah_color::pah_color (int16_t startValue, int16_t midValue, int16_t endValue,
                      uint8_t startR, uint8_t startG, uint8_t startB,
                      uint8_t mid1R , uint8_t mid1G , uint8_t mid1B,
                      uint8_t mid2R , uint8_t mid2G , uint8_t mid2B,
                      uint8_t mid3R , uint8_t mid3G , uint8_t mid3B,
                      uint8_t endR  , uint8_t endG  , uint8_t endB)
{
  setColorvalues( startR, startG, startB, mid1R, mid1G, mid1B,
                  mid2R,  mid2G,  mid2B,  mid3R, mid3G, mid3B,
                  endR,   endG,   endB);

  setStepValues(startValue, midValue, endValue);
}
pah_color::pah_color (void)
{
  pah_color (       0,   255, 500,
                    0,   255, 255,
                    30,  80,  255,
                    40,  255, 60,
                    160, 128, 10,
                    255, 69,  0);
}
pah_color::~pah_color()
{

}

void pah_color::setColorvalues( uint8_t startR, uint8_t startG, uint8_t startB,
                  uint8_t mid1R, uint8_t mid1G, uint8_t mid1B,
                  uint8_t mid2R, uint8_t mid2G, uint8_t mid2B,
                  uint8_t mid3R, uint8_t mid3G, uint8_t mid3B,
                  uint8_t endR, uint8_t endG, uint8_t endB)
{
  setColorStart(startR, startG, startB);
  setColorMid1(mid1R, mid1G, mid1B);
  setColorMid2(mid2R, mid2G, mid2B);
  setColorMid3(mid3R, mid3G, mid3B);
  setColorEnd(endR, endG, endB);
}

void pah_color::setColorvalues( uint32_t start, uint32_t mid1, uint32_t mid2,
                  uint32_t mid3, uint32_t end)
{
  setColorvalues  ( getRed(start), getGreen(start), getBlue(start),
                    getRed(mid1),  getGreen(mid1),  getBlue(mid1),
                    getRed(mid2),  getGreen(mid2),  getBlue(mid2),
                    getRed(mid3),  getGreen(mid3),  getBlue(mid3),
                    getRed(end),  getGreen(end),  getBlue(end)
                    );
}

void pah_color::setColorStart (uint8_t r, uint8_t g, uint8_t b)
{
  pahColorValues.startColorR = r;
  pahColorValues.startColorG = g;
  pahColorValues.startColorB = b;
}
void pah_color::setColorMid1  (uint8_t r, uint8_t g, uint8_t b)
{
  pahColorValues.mid1ColorR = r;
  pahColorValues.mid1ColorG = g;
  pahColorValues.mid1ColorB = b;
}
void pah_color::setColorMid2  (uint8_t r, uint8_t g, uint8_t b)
{
  pahColorValues.mid2ColorR = r;
  pahColorValues.mid2ColorG = g;
  pahColorValues.mid2ColorB = b;
}
void pah_color::setColorMid3  (uint8_t r, uint8_t g, uint8_t b)
{
  pahColorValues.mid3ColorR = r;
  pahColorValues.mid3ColorG = g;
  pahColorValues.mid3ColorB = b;
}
void pah_color::setColorEnd   (uint8_t r, uint8_t g, uint8_t b)
{
  pahColorValues.endColorR = r;
  pahColorValues.endColorG = g;
  pahColorValues.endColorB = b;
}

void pah_color::setStepValues (int16_t start, int16_t mid, int16_t end)
{
  pahColorValues.startValue = start;
  pahColorValues.midValue = mid;
  pahColorValues.endValue = end;
}

uint8_t pah_color::interpol(float u, uint8_t c1, uint8_t c2, uint8_t c3)
{
  float c = (float)c1*((1-u)*(1-u)) + ((float)c2*2*(1-u)*u) + ((float)c3*(u*u));
  //Serial.printf("\t\tRaw value\t%.2f \t has been rounded to %.3u \n", c, (uint8_t)round(c));
  return (uint8_t)round(c);
}

uint32_t pah_color::calcColorValue  (int16_t currentValue)
{
  if(currentValue <= pahColorValues.startValue)
  {
    return get32BitColor( pahColorValues.startColorR,
                          pahColorValues.startColorG,
                          pahColorValues.startColorB);
  }

  if(currentValue > pahColorValues.endValue)
  {
    return get32BitColor( pahColorValues.endColorR,
                          pahColorValues.endColorG,
                          pahColorValues.endColorB);
  }

  float delta = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if(currentValue <= pahColorValues.midValue)
  {
    delta = ((float)currentValue - (float)pahColorValues.startValue) /
            ((float)pahColorValues.midValue - (float)pahColorValues.startValue);

    r = interpol(delta, pahColorValues.startColorR,
                        pahColorValues.mid1ColorR,
                        pahColorValues.mid2ColorR);
    g = interpol(delta, pahColorValues.startColorG,
                        pahColorValues.mid1ColorG,
                        pahColorValues.mid2ColorG);
    b = interpol(delta, pahColorValues.startColorB,
                        pahColorValues.mid1ColorB,
                        pahColorValues.mid2ColorB);
    return get32BitColor(r, g, b);
  }
  if(currentValue <= pahColorValues.endValue)
  {
    delta = (float)(currentValue - pahColorValues.midValue) /
            (float)(pahColorValues.endValue - pahColorValues.midValue);

    r = interpol(delta, pahColorValues.mid2ColorR,
                        pahColorValues.mid3ColorR,
                        pahColorValues.endColorR);
    g = interpol(delta, pahColorValues.mid2ColorG,
                        pahColorValues.mid3ColorG,
                        pahColorValues.endColorG);
    b = interpol(delta, pahColorValues.mid2ColorB,
                        pahColorValues.mid3ColorB,
                        pahColorValues.endColorB);
    return get32BitColor(r, g, b);
  }
  return 0x000000;
}
uint32_t pah_color::get32BitColor   (uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t pah_color::getRed  (uint32_t Color)
{
  return (Color >> 16) & 0xFF;
}
uint8_t pah_color::getGreen(uint32_t Color)
{
  return (Color >> 8) & 0xFF;
}
uint8_t pah_color::getBlue (uint32_t Color)
{
  return Color & 0xFF;
}

uint32_t pah_color::getColorStart(void) {
  return (pahColorValues.startColorR << 16) | (pahColorValues.startColorG << 8) | (pahColorValues.startColorB << 0);
}
uint32_t pah_color::getColorMid1(void) {
  return (pahColorValues.mid1ColorR << 16) | (pahColorValues.mid1ColorG << 8) | (pahColorValues.mid1ColorB << 0);
}
uint32_t pah_color::getColorMid2(void) {
  return (pahColorValues.mid2ColorR << 16) | (pahColorValues.mid2ColorG << 8) | (pahColorValues.mid2ColorB << 0);
}
uint32_t pah_color::getColorMid3(void) {
  return (pahColorValues.mid3ColorR << 16) | (pahColorValues.mid3ColorG << 8) | (pahColorValues.mid3ColorB << 0);
}
uint32_t pah_color::getColorEnd(void) {
  return (pahColorValues.endColorR << 16) | (pahColorValues.endColorG << 8) | (pahColorValues.endColorB << 0);
}

uint16_t pah_color::getStepStart(void) {
  return pahColorValues.startValue;
}
uint16_t pah_color::getStepMid(void) {
  return pahColorValues.midValue;
}
uint16_t pah_color::getStepEnd(void) {
  return pahColorValues.endValue;
}
