#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoMatrix matrix =
Adafruit_NeoMatrix(16, 16, 2, 1, PA2,
NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT,
NEO_GRB + NEO_KHZ800);

uint8_t buf[32*16][3];
int brightness = 10;

uint8_t bitmap[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x7f, 0x98, 
  0x3f, 0xfc, 0x0c, 0x18, 
  0x00, 0x0c, 0x0c, 0x18, 
  0x00, 0x0c, 0x0c, 0x1f, 
  0x00, 0x0c, 0x1e, 0x18, 
  0x00, 0x0c, 0x33, 0x18, 
  0x01, 0x8c, 0x61, 0x98, 
  0x01, 0x98, 0x00, 0x00, 
  0x01, 0x80, 0x0f, 0xe0, 
  0x01, 0x80, 0x38, 0x38, 
  0x01, 0x80, 0x30, 0x18, 
  0x7f, 0xfe, 0x38, 0x38, 
  0x00, 0x00, 0x0f, 0xe0, 
  0x00, 0x00, 0x00, 0x00, 
};

int receivingflag = 0;
uint8_t inbyte[2];
uint32_t lastrecv = 0;
int seq = 0;
int changed = 0;

void setup() {
  matrix.begin();
  matrix.setBrightness(brightness);
  /*matrix.drawPixel(0, 0, (uint32_t)matrix.Color(255, 255, 255));
  matrix.show();
  delay(5000);*/

  uint32_t white = matrix.Color(255, 255, 255);
  for(int j = 0; j < 16; j++){
    for(int i = 0; i < 32; i++){
      unsigned char mask = (bitmap[(int)(j*32+i)/8] & (0b10000000>>(i%8)))>>(7-i%8);
      matrix.drawPixel(i, j, (uint32_t)mask*white);
    }
  }
  matrix.show();
  
  Serial.begin(115200);
  lastrecv = millis();
}

void loop() {
  if (receivingflag == 0 && Serial.available() > 1) {
    inbyte[0] = Serial.read();
    inbyte[1] = Serial.read();
    if ((int)inbyte[0] == 66 && (int)inbyte[1] == 77) {
      receivingflag = 1;
      seq = 0;
      Serial.println("Rec");
      lastrecv = millis();
    } else if ((int)inbyte[0] == 66 && (int)inbyte[1] == 78) {
      delay(1);
      brightness = Serial.read();
      brightness -= 65;
      if (brightness > 20) brightness = 20;
      if (brightness < 0) brightness = 0;
      Serial.print("Bri");
      Serial.println(brightness);
      while (Serial.available() > 0) char t = Serial.read();
      matrix.begin();
      matrix.setBrightness(brightness);

      uint32_t white = matrix.Color(255, 255, 255);
      for(int j = 0; j < 16; j++){
        for(int i = 0; i < 32; i++){
          unsigned char mask = (bitmap[(int)(j*32+i)/8] & (0b10000000>>(i%8)))>>(7-i%8);
          matrix.drawPixel(i, j, (uint32_t)mask*white);
        }
      }
      matrix.show();
    } else {
      while (Serial.available() > 0) char t = Serial.read();
      Serial.println("Inv");
    }
  }

  if (receivingflag == 1 && Serial.available() > 0) {
    inbyte[0] = Serial.read();
    *(&buf[0][0]+seq) = inbyte[0];
    seq = seq + 1;
    if (seq == 1536) {
      inbyte[0] = Serial.read();
      receivingflag = 0;
      changed = 1;
      Serial.println(seq);
      seq = 0;
      while (Serial.available() > 0) char t = Serial.read();
      Serial.println("Fin");
    }
  }
  
  if (receivingflag == 1 && millis() - lastrecv > 300) {
    receivingflag = 0;
    seq = 0;
    while (Serial.available() > 0) char t = Serial.read();
    Serial.println("Err");
  }

  if (receivingflag == 0 && changed == 1) {
    matrix.clear();
    for(int i = 0; i < 32; i++) {
      for(int j = 0; j < 16; j++) {
        matrix.drawPixel(i, j, matrix.Color(buf[j*32+i][0], buf[j*32+i][1], buf[j*32+i][2]));
      }
    }
    matrix.show();
    Serial.println("Dis");
    delay(10);
    changed = 0;
  }
}
