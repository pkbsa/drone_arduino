/* DHT22 = 9

 JST Pin 1 (Black Wire)  => Arduino GND                        DUST 
 JST Pin 3 (Red wire)    => Arduino 5VDC
 JST Pin 4 (Blue wire) => Arduino Digital Pin 8
  
Any Arduino pins labeled:  SDA  SCL                           BMP180
Uno, Redboard, Pro:        A4   A5
Mega2560, Due:             20   21
Leonardo:                   2    3

 ** DI - pin 11                                              SD card
 ** DO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
                           RX 5                             GPS 
                           TX 6 */

#include "DHT.h"
#include <SFE_BMP180.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include "TinyGPS++.h"
#include "SoftwareSerial.h"

#define dislog true
#define FILE_NAME "Hello.txt"

DHT dht;
SFE_BMP180 pressure;
SdFat SD;
File myFile;
SoftwareSerial serial_connection(5, 6); //RX=pin 5, TX=pin 6
TinyGPSPlus gps;

double baseline;
const int chipSelect = 10;
int redpin = 31; // select the pin for the red LED       //Dust Monitor
int greenpin = 32 ;// select the pin for the green LED
int bluepin = 33; // select the pin for the blue LED
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 1000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float smallConcentration = 0;
char tmp ;

void dht22() {
      float humidity = dht.getHumidity(); // คำสั่งดึงค่าความชื้นจาก DHT22
      float temperature = dht.getTemperature(); // คำสั่งดึงค่าอุณหภูมิจาก DHT22
      Serial.print(dht.getStatusString());
      myFile.print(dht.getStatusString());
      Serial.print("\tHumidity :");
      myFile.print("\tHumidity :");
      Serial.print(humidity, 1);
      myFile.print(humidity, 1);
      Serial.print("\t\tTemp C:");
      myFile.print("\t\tTemp C:");
      Serial.print(temperature, 1);
      myFile.print(temperature, 1);
      Serial.print("\t\tTemp F:");
      myFile.print("\t\tTemp F:");
      Serial.println(dht.toFahrenheit(temperature), 1); // แปลงองศาเซลเซียสเป็นฟาเรนไฮน์
      myFile.println(dht.toFahrenheit(temperature), 1);
      delay(1000);
    }

/*void bmp180()
{
  double a,P;
  P = getPressure();
  a = pressure.altitude(P,baseline);
  
  Serial.print("relative altitude: ");
  myFile.print("relative altitude: ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a,1);
  myFile.print(a,1);
  Serial.print(" meters, ");
  myFile.print(" meters, ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a*3.28084,0);
  myFile.print(a*3.28084,0);
  Serial.println(" feet");
  myFile.println(" feet");
  
}

double getPressure()
{
  char status;
  double T,P,p0,a;
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

}
*/
void dust() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    smallConcentration = concentration*4;
    Serial.print(lowpulseoccupancy);
    
    myFile.print(lowpulseoccupancy);
    Serial.print(",");
    myFile.print(",");
    Serial.print(ratio);
    myFile.print(ratio);
    Serial.print(",");
    myFile.print(",");
    Serial.print(concentration);
    myFile.print(concentration);
    Serial.print(",");
    myFile.print(",");
    Serial.println(smallConcentration);//roughly multiply by 4 to get particles > 0.5 micron
    myFile.println(smallConcentration);
    lowpulseoccupancy = 0;
    starttime = millis();
    if (smallConcentration > 1000.0) { // air quality is VERY POOR
      analogWrite(redpin, 255);
      analogWrite(greenpin, 0);
      analogWrite(bluepin, 0);
      Serial.println("very poor");
      myFile.println("very poor");
    }
    else if (smallConcentration > 1050.0) { // air quality is POOR
      analogWrite(redpin, 255);
      analogWrite(greenpin, 255);
      analogWrite(bluepin, 0);
      Serial.println("poor");
      myFile.println("poor");
    }
    else if (smallConcentration > 300.0) { // air quality is FAIR
      analogWrite(redpin, 255);
      analogWrite(greenpin, 0);
      analogWrite(bluepin, 255);
      Serial.println("fair");
      myFile.println("fair");
    }
    else if (smallConcentration > 150.0) { // air quality is GOOD
      analogWrite(redpin, 0);
      analogWrite(greenpin, 255);
      analogWrite(bluepin, 255);
      Serial.println("good");
      myFile.println("good");
    }
    else if (smallConcentration > 75.0) { // air quality is VERY GOOD
      analogWrite(redpin, 0);
      analogWrite(greenpin, 0);
      analogWrite(bluepin, 255);
      Serial.println("very good");
      myFile.println("very good");
    }
    else { // air quality is EXCELLENT (<75)
      analogWrite(redpin, 0);
      analogWrite(greenpin, 255);
      analogWrite(bluepin, 0);
      Serial.println("excellent");
      myFile.println("excellent");
    }
  }
delay(1000);
}

void collecting()
{
  while(serial_connection.available())//While there are characters to come from the GPS
  {
    tmp = serial_connection.read();
    gps.encode(tmp);
  }
  if(gps.location.isUpdated())
  {
  Serial.print("Date: ");
  myFile.print("Date: ");
  Serial.print(gps.date.day()); Serial.print("/");
  myFile.print(gps.date.day()); myFile.print("/");
  Serial.print(gps.date.month()); Serial.print("/");
  myFile.print(gps.date.month()); myFile.print("/");
  Serial.print(gps.date.year()); Serial.print("\t");
  myFile.print(gps.date.year()); myFile.print("\t");
  Serial.print("Time: ");
  myFile.print("Time: ");
  Serial.print(gps.time.hour()); Serial.print(":");
  myFile.print(gps.time.hour()); myFile.print(":");
  Serial.print(gps.time.minute()); Serial.print(":");
  myFile.print(gps.time.minute()); myFile.print(":");
  Serial.print(gps.time.second()); Serial.print(":");
  myFile.print(gps.time.second()); myFile.print(":");
  Serial.print(gps.time.centisecond()); Serial.print("\n");
  myFile.print(gps.time.centisecond()); myFile.print("\n");
  Serial.print(gps.location.lat(), 6); Serial.print("\t");
  myFile.print(gps.location.lat(), 6); myFile.print("\t");
  Serial.print(gps.location.lng(), 6); Serial.print("\n");
  myFile.print(gps.location.lng(), 6); myFile.print("\n");
  }
}


void setup() {
  
  Serial.begin(9600);
  Serial.println();
  serial_connection.begin(9600);//This opens up communications to the GPS
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started

  Serial.print("HELLO MY NAME IS PKBSSSSSSSSSSSSSSSSSSSSSSSSSS");    
  if(!SD.begin(chipSelect)) return ; //SD CARD
  SD.remove(FILE_NAME);
   myFile = SD.open(FILE_NAME, O_RDWR | O_CREAT | O_APPEND);
   if (!myFile) {
    Serial.println("opening test.txt failed");
  }
  Serial.print("SD card is ready.\n");
  myFile.print("SD card is ready.\n"); 
  
  
  Serial.println();  //dht22
  myFile.println("Status\tHumidity (%)\tTemperature (C)\t(F)");
  dht.setup(9);
  
 
 /* Serial.println("REBOOT"); //BMP180
  myFile.println("REBOOT");
  if (pressure.begin())
  Serial.println("BMP180 init success");
  else{
  Serial.println("BMP180 init fail (disconnected?)\n\n");
  myFile.println("BMP180 init fail (disconnected?)\n\n");
  while(1);}
  baseline = getPressure();
  Serial.print("baseline pressure: ");
  myFile.print("baseline pressure: ");
  Serial.print(baseline);
  myFile.print(baseline);
  Serial.println(" mb");  
  myFile.println(" mb");  */
  
  pinMode(redpin, OUTPUT); //DUST
  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(8,INPUT);
  starttime = millis();//get the current time;
    
    myFile.seek(0);
   int c;
   // print the current file.
    while ((c = myFile.read()) >= 0) {
   Serial.write(c);
    }
  
}


void loop() {
   
   
   dht22();
   
   //bmp180();
   
   dust();
   
   myFile.println("==========================");
   Serial.println("==========================");
   
}
