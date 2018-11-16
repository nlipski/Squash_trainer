#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

#include <ESP8266WiFi.h>
//#include <PubSubClient.h>



const byte interrupt_pin = 13;
volatile byte interrupt_status = 0;
static int num_of_interrupts = 0;

WiFiClient espClient;

MPU6050 mpu;
uint8_t devStatus;

void handle_interrupt() {
  interrupt_status++;
  num_of_interrupts++; 
}

void setup() {
  Serial.begin(115200);
  pinMode(interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin),handle_interrupt, RISING);
  

  devStatus = mpu.dmpInitialize();
  
  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); 



   // make sure it worked (returns 0 if so)
if (devStatus == 0) {
	// turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

	// enable Arduino interrupt detection
	Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));

    Serial.println(F(")..."));

    mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
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
