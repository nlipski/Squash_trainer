#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>  

#define AZ_OFFSET 16384
#define GYRO_DEADZONE 1
#define ACCEL_DEADZONE 8

struct offsets{
	int ax_offset;
	int ay_offset;
	int az_offset;
	int gx_offset;
	int gy_offset;
	int gz_offset;
};

struct means{
	long mean_ax;
	long mean_ay;
	long mean_az;
	long mean_gx;
	long mean_gy;
	long mean_gz;
};

// Pin definition 
const byte interrupt_pin = 15; // pin D8

// Interrupt variables
volatile byte interrupt_flag = 0;
static int num_of_interrupts = 0;
byte mpuIntStatus = 0;


MPU6050 mpu;
byte devStatus;
VectorInt16 aa;  
bool dmpReady;
uint16_t packetSize;

//ESP Wifi initialization
WiFiClient espClient;
WiFiUDP Udp;
WiFiManager wifiManager; 
const IPAddress outIp(192, 168, 1, 11); 
const unsigned int outPort = 9999;
const char DEVICE_NAME[] = "ESP8266_squash";

struct offsets = {0};
struct means = {0};


void reset_offsets()
{
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
}

void setOffsets()
{
  mpu.setXGyroOffset(offsets.gx_offset);
  mpu.setYGyroOffset(offsets.gy_offset);
  mpu.setZGyroOffset(offsets.gz_offset);
  mpu.setXAccelOffset(offsets.ax_offset);
  mpu.setYAccelOffset(offsets.ay_offset);
  mpu.setZAccelOffset(offsets.az_offset);
}

void meansensors()
{
  int i = 0;
  long buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

  for(i = 0; i < (buffersize+101); i++) {
    
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (i>100 && i<= (buffersize+100)) { 
      buff_ax += ax;
      buff_ay += ay;
      buff_az += az;

      buff_gx += gx;
      buff_gy += gy;
      buff_gz += gz;
    }

    delay(2);
  }

  means.mean_ax= buff_ax/buffersize;
  means.mean_ay= buff_ay/buffersize;
  means.mean_az= buff_az/buffersize;
  means.mean_gx= buff_gx/buffersize;
  means.mean_gy= buff_gy/buffersize;
  means.mean_gz= buff_gz/buffersize;
}

void mpu_calibration()
{ 
	if (means.mean_ax != 0) {
  	offsets.ax_offset = -1*(means.mean_ax/8);
  	offsets.ay_offset = -1*(means.mean_ay/8);
  	offsets.az_offset = (AZ_OFFSET - mean_az)/8;

  	offsets.gx_offset = -1*(means.mean_gx/4);
  	offsets.gy_offset = -1*(means.mean_gy/4);
  	offsets.gz_offset = -1*(means.mean_gz/4);
	}
	while (true) {

    int ready=0;
    
    setOffsets();
    meansensors();

    if (abs(means.mean_ax) <= ACCEL_DEADZONE) 
      ready++;
    else 
      offsets.ax_offset = offsets.ax_offset - means.mean_ax/ACCEL_DEADZONE;

    if (abs(means.mean_ay) <= ACCEL_DEADZONE)
      ready++;
    else 
      offsets.ay_offset = offsets.ay_offset - means.mean_ay/ACCEL_DEADZONE;

    if (abs(AZ_OFFSET-means.mean_az) <= ACCEL_DEADZONE) 
      ready++;
    else 
      offsets.az_offset = offsets.az_offset + (AZ_OFFSET-means.mean_az)/ACCEL_DEADZONE;

    if (abs(means.mean_gx) <= GYRO_DEADZONE) 
      ready++;
    else 
      offsets.gx_offset = offsets.gx_offset - means.mean_gx/(GYRO_DEADZONE + 1);

    if (abs(means.mean_gy) <= GYRO_DEADZONE)
      ready++;
    else 
      offsets.gy_offset = offsets.gy_offset-means.mean_gy/(GYRO_DEADZONE + 1);

    if (abs(means.mean_gz) <= GYRO_DEADZONE) ready++;
    else offsets.gz_offset = offsets.gz_offset - means.mean_gz/(GYRO_DEADZONE + 1);

    if (ready==6) break;
  }

  delay(1000);
}

void handle_interrupt() {
  interrupt_flag++;
  num_of_interrupts++; 
}

bool setupInterrupt()
{
  pinMode(interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin),handle_interrupt, RISING);
  delay(100);
  return mpu.getIntStatus();
}

void setup_mpu()
{
	Wire.begin();
	Wire.setClock(400000);

	clear_buffer();
	mpu.initialize();
	reset_offsets();

	if (0 != (devStatus = mpu.dmpInitialize()))
	{
		Serial.println("DMP Initialization FAILURE with code: ");
		Serial.print(devStatus);
		return;
	}

	mpu.setDMPEnabled(true);
	mpu.dmpInitialize();
	mpuIntStatus = setupInterrupt();
	dmpReady = true;
	packetSize = mpu.dmpGetFIFOPacketSize();
	Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  meansensors();
  delay(1000);
}

void clear_buffer()
{
	while (Serial.available() && Serial.read());
}
void setup_wifi()
{
  wifiManager.resetSettings();
  wifiManager.autoConnect(DEVICE_NAME);
  Serial.print(F("WiFi connected! IP address: "));
  Serial.println(WiFi.localIP());
}
void setup()
{
  Serial.begin(115200);
  clear_buffer();
  
  setup_wifi();

  setup_mpu();
  mpu_calibration();
}

void loop()
{
  while(!interrupt_flag) {
    Serial.println("Interrupt");
    Serial.print(" - num_of_interrupts = ");
    Serial.print(num_of_interrupts);
  }
  interrupt_flag = 0;
}
