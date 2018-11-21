#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <ESP8266WiFi.h>

// Pin definition 
const byte interrupt_pin = 15; // pin D8

// Interrupt variables
volatile byte interrupt_status = 0;
static int num_of_interrupts = 0;
byte mpuIntStatus = 0;


MPU6050 mpu;
byte devStatus;
VectorInt16 aa;  


//ESP Wifi initialization
WiFiClient espClient;
WiFiUDP Udp; 
const IPAddress outIp(192, 168, 1, 11); 
const unsigned int outPort = 9999;


void reset_offsets()
{
  // Zeroing Gyro and Arduino values
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
}

void supplyOffsets()
{
  //supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788);
}


void handle_interrupt() {
  interrupt_status++;
  num_of_interrupts++; 
}



bool setupInterrupt()
{
  pinMode(interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin),handle_interrupt, RISING);
  return mpu.getIntStatus();
}
void setup_mpu()
{
	
}

void setup() {

  Serial.begin(115200);
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  //packetSize = mpu.dmpGetFIFOPacketSize();

  devStatus = mpu.dmpInitialize();
  if (!devStatus) 
  {
  	//enable DMP
  	mpu.setDMPEnabled(true);
  	mpuIntStatus = setupInterrupt();

  }
}

void loop() {
  while(!interrupt_status) {
    Serial.println("Interrupt");
    Serial.println("num_of_interrupts = ");
    Serial.print(num_of_interrupts);
  }
  interrupt_status = 0;
}
