#include <SmartMatrix3.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 128;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 64;       // known working: 32, 64, 96, 128
const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 32;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels, or use SMARTMATRIX_HUB75_64ROW_MOD32SCAN for common 64x64 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

void setup() {
  matrix.addLayer(&backgroundLayer);
  matrix.setBrightness(254);
  matrix.begin();

  Serial.begin(115200);
}

unsigned char pix[3] = { 0 };
rgb24 *buffer;

void loop() {}
#define BLK 28

void serialEvent()
{
  switch (Serial.read())
  {
    case 0x05: //request for matrix definition
      Serial.println(kMatrixWidth);
      Serial.println(kMatrixHeight);
      break;

    case 0x41: //24bpp frame data
      while (backgroundLayer.isSwapPending());
      buffer = backgroundLayer.backBuffer();
      for (int i = 0; i < kMatrixWidth * kMatrixHeight; i++) {
        Serial.readBytes(pix, 3);
        *buffer++ = rgb24{map(pix[0], 0, 255, BLK, 255), map(pix[1], 0, 255, BLK, 255), map(pix[2], 0, 255, BLK, 255)};
      }
      Serial.write(0x06); //acknowledge
      backgroundLayer.swapBuffers(false);
      break;

    case 0x42: //16bpp frame data
      while (backgroundLayer.isSwapPending());
      buffer = backgroundLayer.backBuffer();
      for (int i = 0; i < kMatrixWidth * kMatrixHeight; i++) {
        Serial.readBytes(pix, 2);
        *buffer++ = rgb24{map(((pix[0] & B11111000) >> 3), 0, 31, BLK, 255), map((((pix[0] & B00000111) << 3) | ((pix[1] & B11100000) >> 5)), 0, 63, BLK, 255), map((pix[1] & B00011111), 0, 31, BLK, 255)}; //,
      }
      Serial.write(0x06); //acknowledge
      backgroundLayer.swapBuffers(false);
      break;

    case 0x43: //8bpp frame data
      while (backgroundLayer.isSwapPending());
      buffer = backgroundLayer.backBuffer();
      for (int i = 0; i < kMatrixWidth * kMatrixHeight; i++) {
        Serial.readBytes(pix, 1);
        *buffer++ = rgb24{map(((pix[0] & B11100000) >> 5), 0, 7, BLK, 255), map(((pix[0] & B00011100) >> 2), 0, 7, BLK, 255), map(pix[0] & B00000011, 0, 3, BLK, 255)}; //,
      }
      Serial.write(0x06); //acknowledge
      backgroundLayer.swapBuffers(false);
      break;

    case 0x44: //8bpp monochrome frame data
      while (backgroundLayer.isSwapPending());
      buffer = backgroundLayer.backBuffer();
      for (int i = 0; i < kMatrixWidth * kMatrixHeight; i++) {
        Serial.readBytes(pix, 1);
        byte c = map(pix[0], 0, 255, BLK, 255);
        *buffer++ = rgb24{c, c, c}; //,
      }
      Serial.write(0x06); //acknowledge
      backgroundLayer.swapBuffers(false);
      break;

    case 0x45: //1bpp monochrome frame data
      while (backgroundLayer.isSwapPending());
      buffer = backgroundLayer.backBuffer();
      for (int i = 0; i < ((kMatrixWidth * kMatrixHeight) / 8) + ((kMatrixWidth * kMatrixHeight) % 8); i++) {
        Serial.readBytes(pix, 1);
        byte b = ((pix[0] & B10000000) >> 7) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B01000000) >> 6) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B00100000) >> 5) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B00010000) >> 4) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B00001000) >> 3) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B00000100) >> 2) * 255;
        *buffer++ = rgb24{b, b, b};
        b = ((pix[0] & B00000010) >> 1) * 255;
        *buffer++ = rgb24{b, b, b};
        b = (pix[0] & B00000001) * 255;
        *buffer++ = rgb24{b, b, b};
      }
      Serial.write(0x06); //acknowledge
      backgroundLayer.swapBuffers(false);
      break;


  }
}
