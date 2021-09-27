#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(2,3);
 
#include <String.h>


#include <OneWire.h>
OneWire ds(53);                     // Αρχικοποιούμε την OneWire ορίζοντας το είδος σήματος (ψηφιακό) 
                                    // και το pin που έχουμε συνδέσει τον αισθητήρα (ή το αντίστοιχο του terminal της dfrobot)


#include "DFRobot_PH.h"
#include <EEPROM.h>

#define PH_PIN A0
float phVoltage,phValue,phV;
float temperature = 28;
float phOffset=-0.03;
DFRobot_PH ph;



#define TDS_PIN A1
#define TDS_samples 30
int TDSanalogBuffer[TDS_samples];
int tdsValue = 0;
float TDSaverageVoltage = 0, tdsV = 0;


#include "DFRobot_EC10.h"
#define EC_PIN A3
float ecVoltage,ecValue,ecV;
DFRobot_EC10 ec;


#define DO_PIN  A2
#define DO_samples 30
int DOanalogBuffer[DO_samples];
float doValue, doV, DOaverageVoltage;
#define SaturationDoVoltageAddress 12          //the address of the Saturation Oxygen voltage stored in the EEPROM
#define SaturationDoTemperatureAddress 16      //the address of the Saturation Oxygen temperature stored in the EEPROM
float SaturationDoVoltage = 1127.6;   //default voltage:1127.6mv
float SaturationDoTemperature = 25.0; // default value 

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}


const float SaturationValueTab[41] PROGMEM = {      //saturation dissolved oxygen concentrations at various temperatures
14.46, 14.22, 13.82, 13.44, 13.09,
12.74, 12.42, 12.11, 11.81, 11.53,
11.26, 11.01, 10.77, 10.53, 10.30,
10.08, 9.86,  9.66,  9.46,  9.27,
9.08,  8.90,  8.73,  8.57,  8.41,
8.25,  8.11,  7.96,  7.82,  7.69,
7.56,  7.43,  7.30,  7.18,  7.07,
6.95,  6.84,  6.73,  6.63,  6.53,
6.41,
};



 
void setup()
{
  gprsSerial.begin(115200);
  Serial.begin(115200);
  ph.begin();
  pinMode(TDS_PIN,INPUT);
  ec.begin();
  pinMode(DO_PIN,INPUT);
  //readDoCharacteristicValues();
  delay(10000);
}
 
void loop()
{

  temperature = getTemp();
  phValue = getPh();
  tdsValue = int(getTDS());
  ecValue = getEC();
  doValue = getDO();
      
   
  if (gprsSerial.available())
    Serial.write(gprsSerial.read());
 
  gprsSerial.println("AT");
  delay(1000);
 
  gprsSerial.println("AT+CPIN=7786");
  delay(1000);
 
  gprsSerial.println("AT+CREG?");
  delay(1000);
 
  gprsSerial.println("AT+CGATT?");
  delay(1000);
 
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
 
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
 
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"myq\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  
  String str="GET https://api.thingspeak.com/update?api_key=4QJNOA7PKP33YGFR&field1=" + String(temperature) + "&field2=" + String(phValue) + "&field3=" + String(tdsValue) + "&field4=" + String(ecValue) + "&field5=" + String(doValue);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
} 




void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}
