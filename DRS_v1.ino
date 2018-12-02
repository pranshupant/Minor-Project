#include <Wire.h>
#include <Servo.h>
void setupMPU();
void recordAccelRegisters();
void processAccelData();

long wheelspeed(unsigned long ti);

float printXData();
float printYData();
float printZData();

const float SPF = 0.429;
const float LateralG = 0.5;
unsigned long ti = 0;
long wspeed = 0;

Servo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position
int i = 0;
float x;
float y;
float z;

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

void setup() {
  Serial.begin(9600);
  Serial.println("START");
  Wire.begin();
  setupMPU();
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(4,INPUT);
  pinMode(3,INPUT);
}


void loop() {
  recordAccelRegisters();
  printXData();
  x=printXData();
  y=printYData();
  z=printZData();
  delay(100);

  ti = millis();
  wspeed = wheelspeed(ti);
  //Serial.println(wspeed);

  Serial.println("Brake");
  Serial.println(digitalRead(4));


  if((x < LateralG) && digitalRead(4) == HIGH && wspeed > 30 )
  {
  Serial.println("DRS ON");
  for (pos=i; pos <= 135; pos += 25) // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
   {
    myservo.write(pos);// tell servo to go to position in variable 'pos'

    delay(10);                       // waits 15ms for the servo to reach the position
   }
   i=pos;
  }
  else if(x >= LateralG || digitalRead(4) == LOW || wspeed <= 30)
 {
   Serial.println("DRS OFF");
   for (pos = i; pos >= 0; pos -= 25)
    {
    myservo.write(pos);// tell servo to go to position in variable 'pos'
    delay(10); // waits 15ms for the servo to reach the position
   }
  }
  i=pos;

  Serial.println("Speed= ");
  Serial.print(wspeed);

  Serial.println();

  Serial.print(digitalRead(3));

 // Serial.println(deltime);

  //Serial.println(digitalRead(3));
}

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission();
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData(){
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;
}

float printXData() {

  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
  return abs(gForceX);
}
float printYData()
{
  return abs(gForceY);

}
float printZData()
{
  return abs(gForceZ);

}
long wheelspeed(unsigned long ti)
{
    unsigned long totaltime = 0;
    unsigned long t1 = 0;
    unsigned long t2 = 0;
    unsigned long tf = 0;

    long lspeed = 0;
    double deltime = 0;
    double dspeed = 0;
    int count = 1;
    while(totaltime < 100)
    {
        tf = millis();
        totaltime = tf - ti;

        t1 = t2;

        if (digitalRead(3) == HIGH && count != 0)
        {
            t2 = millis();
            deltime = t2 - t1;
            deltime /= 1000.00;
            Serial.println();
            //Serial.print("Deltime=");
            //Serial.print(deltime);
            if (deltime > 0 && deltime < 0.5)
            {
                dspeed = SPF / deltime;
            }
            count = 0;
        }
        else if (digitalRead(3) == LOW)
        {
            count = 1;
        }
    }
    lspeed = dspeed;
    return lspeed;
}
