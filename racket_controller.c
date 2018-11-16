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

void handle_interrupt() {
  interrupt_status++;
  num_of_interrupts++; 
}

void setup() {
  Serial.begin(115200);
  pinMode(interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin),handle_interrupt, FALLING);
  mpu.initialize();
//  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)){
//    println("Connecting MPU6050");
//    delay(500);
//  }
//  
//  mpu.calibrateGyro();
//  mpu.setThreshold(3);
}

void loop() {
  while(!interrupt_status) {
    Serial.println("Interrupt");
    Serial.println("num_of_interrupts = ");
    Serial.print(num_of_interrupts);
  }
  interrupt_status = 0;
}
