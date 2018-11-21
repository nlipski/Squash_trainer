#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <ESP8266WiFi.h>

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
volatile byte interrupt_status = 0;
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
const IPAddress outIp(192, 168, 1, 11); 
const unsigned int outPort = 9999;


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

void supplyOffsets()
{
  mpu.setXGyroOffset(offsets.gx_offset);
  mpu.setYGyroOffset(offsets.gy_offset);
  mpu.setZGyroOffset(offsets.gz_offset);
  mpu.setXAccelOffset(offsets.ax_offset);
  mpu.setYAccelOffset(offsets.ay_offset);
  mpu.setZAccelOffset(offsets.az_offset);
}

void meansensors(){
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
	Wire.begin();
	Wire.setClock(400000);

	mpu.initialize();

	reset_offsets();

	devStatus = ;

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
}

void clear_buffer()
{
	while (Serial.available() && Serial.read());
}

void setup() {

  Serial.begin(115200);

}

void loop() {
  while(!interrupt_status) {
    Serial.println("Interrupt");
    Serial.println("num_of_interrupts = ");
    Serial.print(num_of_interrupts);
  }
  interrupt_status = 0;
}
