#include <OneWire.h>
#include <DallasTemperature.h>
#include "dht.h"               //DHT library
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
#define SensorPin A5          // the pH meter Analog output is connected with the Arduino’s Analog
#define DHTPIN A2                 //DHT pin
float SeperateData[5] = {0,0,0,0,0}; //Seperate Data [humid,temperature,ph,distance,ec]

dht DHT;

const int TempProbePossitive = 8;  //Temp Probe power connected to pin 9
const int TempProbeNegative = 9;   //Temp Probe Negative connected to pin 8
const int trigPin = 12; //trig pin connection US
const int echoPin = 11;  //echopin connection US
unsigned long int avgValue;  //Store the average value of the sensor feedback ph

int R1 = 1000;
int Ra = 25;    //Resistance of powering Pins
int ECPin = A0;
int ECGround = A1;
int ECPower = A4;
long duration; //US
int distanceCm; //US
int pump1 = 5; //declare pump pin ph
//int pump2 = 1;  //declare small pump ph 
float calibration_value = 21.34-0.7;
float ph_act; // ph
int buf[10],temp; // ph
float PPMconversion = 0.7;
float TemperatureCoef = 0.019;
float K=2.88;

OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

float Temperature = 10;
float EC = 0;
float EC25 = 0;
int ppm = 0;
float raw = 0;
float Vin = 5;
float Vdrop = 0;
float Rc = 0;
float buffer = 0;
float t;                  // temperature in celcius dht11
float h;                 // humidity in percentage dht11


void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); //US sensors
  pinMode(echoPin, INPUT);  //US sensors
  pinMode(pump1, OUTPUT);
  /*pinMode(pump2, OUTPUT);*/
  pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
  pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
  digitalWrite(TempProbePossitive , HIGH );
  pinMode(ECPin,INPUT);
  pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
  pinMode(ECGround,OUTPUT);//setting pin for sinking current
  digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly
 
  delay(1000);// gives sensor time to settle
  sensors.begin();
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance
}

void loop()
{
GetWaterLevel();
GetEC();
GetpH();
GetDht();
sendState();
//PrintReadings();
delay(5000);
} 

void GetEC(){
//*********Reading Temperature Of Solution 
sensors.requestTemperatures();// Send the command to get temperatures
Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable
 
//************Estimates Resistance of Liquid 
digitalWrite(ECPower,HIGH);
raw= analogRead(ECPin);
raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
digitalWrite(ECPower,LOW);
 
//***************** Converts to EC 
Vdrop= (Vin*raw)/1024.0;
Rc=(Vdrop*R1)/(Vin-Vdrop);
Rc=Rc-Ra; //acounting for Digital Pin Resitance
EC = 1000/(Rc*K);

//*************Compensating For Temperaure
EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
ppm=(EC25)*(PPMconversion*1000);
SeperateData[4]=distanceCm;
}


void GetWaterLevel(){
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distanceCm= duration*0.034/2;  
SeperateData[3]=distanceCm;                                                                               
delay(100);
}

void GetpH(){
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { buf[i]=analogRead(SensorPin);
    delay(30);}
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {for(int j=i+1;j<10;j++)
    {if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float volt=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  float ph_act = -5.70 * volt + calibration_value;
  SeperateData[2]= ph_act;
  
  /*digitalWrite(pump1, HIGH);       
  delay(1000);
  digitalWrite(pump1, LOW); 

 if ((phValue >6.99)||(phValue<5)) //condition for ph value
    {digitalWrite(led , HIGH);}
  else
    {digitalWrite(led, LOW);}
  delay(1000);
}*/
}

void GetDht(){
  DHT.read11(DHTPIN);
  h = DHT.humidity ;                                                                                         
  t = DHT.temperature;
  SeperateData[0]=h;
  SeperateData[1]=t;
  delay(1000);
}


/*void PrintReadings(){
Serial.print("Distance : ");
Serial.print(distanceCm);
Serial.println(" Cm ");
  Serial.print("Rc: ");
  Serial.print(Rc);
  Serial.print(" EC: ");
  Serial.print(EC25);
  Serial.print(" Simens  ");
  Serial.print(ppm);
  Serial.println(" ppm  ");
    Serial.print("pH: ");  
    Serial.print(ph_act,2);
    Serial.println(" pH ");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" C.   ");
      Serial.print(" Humidity: ");
      Serial.print(h);
      Serial.println("% ");*/

/*
//********** Used for Debugging ************
Serial.print("Vdrop: ");
Serial.println(Vdrop);
Serial.print("Rc: ");
Serial.println(Rc);
Serial.print(EC);
Serial.println("Siemens");
//********** end of Debugging Prints *********
}
*/

void sendState() {
  for(int i=0; i<5; i++){
    Serial.print(SeperateData[i]);
    if(i<4) { Serial.print(",");}
  }
  Serial.println("");
}
