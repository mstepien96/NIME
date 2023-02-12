#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=454.20001220703125,215.1999969482422
AudioAnalyzeNoteFrequency notefreq1;      //xy=635.2000122070312,20
AudioSynthWaveformSine   sine2;          //xy=642.2000122070312,243.1999969482422
AudioSynthWaveformSine   sine1;          //xy=644.2000122070312,163.1999969482422
AudioEffectMultiply      multiply1;      //xy=743.2000122070312,69.19999694824219
AudioEffectMultiply      multiply2;      //xy=746.1999969482422,318.1999969482422
AudioMixer4              mixer1;         //xy=882.800048828125,224.1999969482422
AudioFilterStateVariable filter1;        //xy=901.2000732421875,77.19999694824219
AudioFilterStateVariable filter2;        //xy=914.2000732421875,333.20001220703125
AudioMixer4              mixer2;         //xy=1024.800048828125,227.1999969482422
AudioOutputI2S           i2s1;           //xy=1165.7999267578125,231.1999969482422
AudioConnection          patchCord1(i2s2, 0, notefreq1, 0);
AudioConnection          patchCord2(i2s2, 0, multiply1, 0);
AudioConnection          patchCord3(i2s2, 0, multiply2, 1);
AudioConnection          patchCord4(i2s2, 0, mixer1, 2);
AudioConnection          patchCord5(sine2, 0, multiply2, 0);
AudioConnection          patchCord6(sine1, 0, multiply1, 1);
AudioConnection          patchCord7(multiply1, 0, filter1, 0);
AudioConnection          patchCord9(multiply2, 0, filter2, 0);
AudioConnection          patchCord11(mixer1, 0, mixer2, 0);
AudioConnection          patchCord12(filter1, 0, mixer1, 0);
AudioConnection          patchCord13(filter1, 2, mixer1, 1);
AudioConnection          patchCord14(filter2, 0, mixer2, 2);
AudioConnection          patchCord15(filter2, 2, mixer2, 3);
AudioConnection          patchCord16(mixer2, 0, i2s1, 0);
AudioConnection          patchCord17(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=469.1999969482422,386.1999969482422
// GUItool: end automatically generated code

// Gyro variables
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x;
double prevX = 180.00;
double xDiff;
double y;
double z;
const int baseAngle = 180;

// Switch variables
int switchValue;

// System variables
double Fcut = 400.00;


void setup() {
  Serial.begin(9600);
  AudioMemory(50);
  // Switch
  pinMode(3, INPUT_PULLUP);
  // Gyro (MPU6050)
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  // Block diagram setup
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(36);
  mixer1.gain(0, 0.0);  
  mixer1.gain(1, 1.0);
  mixer1.gain(2, 1.0);
  mixer1.gain(3, 0.0);
  mixer2.gain(0, 1.0);
  mixer2.gain(1, 0.0);
  mixer2.gain(2, 0.0);
  mixer2.gain(3, 1.0);
  filter1.frequency(Fcut);
  filter1.resonance(0.707);
  filter2.frequency(Fcut);
  filter2.resonance(0.707);
  notefreq1.begin(.15);
  delay(1000);
}

void gyro_loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);
 
  x = RAD_TO_DEG * (atan2(-yAng, zAng)+PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

  xDiff = x - prevX;
  prevX = x;

  Serial.print("Angle X: ");
  Serial.println(x);
}

void switch_loop() {
  switchValue = digitalRead(3);
  Serial.println("-----------------------------------------");
  if (switchValue == 0){
    Serial.println("Distortion OFF");
  }
  else {
    Serial.println("Distortion ON");
  }
}

void distortion_loop() {
  Fcut = Fcut + 4 * xDiff;
  Serial.printf("Fcut: %3.2f", Fcut);
  Serial.println();
  
  if (switchValue == 1) {
    if (notefreq1.available()) {
          float note = notefreq1.read();
          float prob = notefreq1.probability();
          filter1.frequency(max(50.00, Fcut));
          filter2.frequency(max(50.00, Fcut));
          sine1.frequency(note*0.76);
          sine2.frequency(note*0.85);
          sine1.amplitude(0.76);
          sine2.amplitude(0.85);
          Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);
      }
    else {
      delay(80);
      sine1.amplitude(0);
      sine2.amplitude(0);
    }
  }
  else {
     sine1.amplitude(0);
     sine2.amplitude(0);
  }
}

void loop() {
  switch_loop();
  gyro_loop();
  distortion_loop();
}
