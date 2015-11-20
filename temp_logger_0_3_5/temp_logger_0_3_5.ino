#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

const int chipSelect = 4;

#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,178,72);
IPAddress dns1(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
EthernetServer server(80);
File webFile;

void setup() {
    Ethernet.begin(mac, ip, dns1, gateway, subnet);
    server.begin();
    digitalWrite(10, HIGH);
    if (!SD.begin(4)) {
        return;
    }

  pinMode(3, INPUT);
  
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  
  for(int i=0;i<numberOfDevices; i++)
  {
      if(sensors.getAddress(tempDeviceAddress, i))
	{
	sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
	}
  }
  
Wire.begin();
rtc.begin();
}


void loop() {
  delay(5000);  
  String Time = "";
    DateTime now = rtc.now();
    Time+=String(now.year());
    Time+="/";
    if (now.month() >= 0 && now.month() < 10) {
      Time +="0";
      }
    Time+=String(now.month());
    Time+="/";
    if (now.day() >= 0 && now.day() < 10) {
      Time +="0";
      }
    Time+=String(now.day());
    Time+=";";
    if (now.hour() >= 0 && now.hour() < 10) {
      Time +="0";    
      }
    Time+=String(now.hour());
    Time+=":";
    if (now.minute() >= 0 && now.minute() < 10) {
      Time +="0";
      }
    Time+=String(now.minute());
    Time+=":";
    if (now.second() >= 0 && now.second() < 10) {
      Time +="0";
      }
    Time+=String(now.second());
    sensors.requestTemperatures(); //Send the command to get temperatures
    String dataString = "";
    dataString += Time;

    for(int i=0;i<numberOfDevices; i++)
    {
      if(sensors.getAddress(tempDeviceAddress, i))
      {
  	dataString += ";device:";
  	dataString += String(i,DEC);
  	float tempC = sensors.getTempC(tempDeviceAddress);
        dataString +=";Temp C:;";
        dataString += String(tempC);
    } 
  
    }
     File dataFile = SD.open("templog.csv", FILE_WRITE);
     delay(1000);
     if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
      }  
    EthernetClient client = server.available();
    if (client) {
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) { 
                   code200(client);
                   webFile = SD.open("templog.csv");
                   if (webFile) {
                       while(webFile.available()) {
                           client.write(webFile.read());
                       }
                       webFile.close();
                   }
                   break;
            }
        } 
        delay(1);
        client.stop();
    }
}

void code200(EthernetClient client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
}


