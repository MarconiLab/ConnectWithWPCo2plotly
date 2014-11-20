/*
 
 This example connects to an unencrypted Wifi network. 
 Then it prints the  MAC address of the Wifi shield,
 the IP address obtained, and other network details.

 Circuit:
 * WiFi shield attached
 
 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe

 RESULTADO
 Attempting to connect to WPA SSID: MarconiLab
 You're connected to the networkSSID: MarconiLab
 BSSID: DC:9F:DB:1C:54:9E
 signal strength (RSSI):-53
 Encryption Type:4

 IP Address: 192.168.88.107
 192.168.88.107
 MAC address: 78:C4:E:1:D5:D1

 */
 #include "plotly_streaming_wifi.h"
 #include <WiFi.h>
 #include <SPI.h>
 #include <Wire.h>             // See note above
 #include <Sensirion.h>        // SHT11 Library from: https://github.com/domhardt/ArduinoLibraries/tree/master/Sensirion
 #include <Adafruit_BMP085.h>  // BMP Library from: https://github.com/adafruit/Adafruit-BMP085-Library

 #define address 0x31          // I2C address of CO2 Sensor
 const uint8_t dataPin  =  16; // SHT11 Data pin connected to Arduino A2
 const uint8_t clockPin =  17; // SHT11 Clock pin connected to Arduino A3
 Sensirion tempSensor = Sensirion(dataPin, clockPin); //Setup SHT11 Sensor
 Adafruit_BMP085 bmp;          // Setup BMP085 Sensor
 
 const int I2Cdelay=100; // Required delay when reading the CO2 Sensor.

 //Assign variables
 int co;                 // CO2 stored as an integer
 float temperature;      // Temperature stored as a float
 float humidity;         // Relative Humidity stored as a float
 float dewpoint;         // Dew Point stored as a float
 float airpres = 0.0;    // Air Pressure stored as a float
 float CalFactor = 8.6;  // Air Pressure Calibration factor - adjsut to allow for alititude
 int sampleInterval = 5; // Sample interval in seconds 

 //Variables used to read CO2 sensor
 const int data_bytes = 7;
 byte data_buffer [data_bytes];

 const int  LED1 = 5;  // LED1 on Arduino pin D5
 const int  LED2 = 8;  // LED2 on Arduino pin D8


 //SerialLCD
//#include <SerialLCD.h>
// #include <SoftwareSerial.h> //this is a must
// SerialLCD slcd(11,12);//this is a must, assign soft serial pins
 
// Sign up to plotly here: https://plot.ly
#define nTraces 5
char *tokens[nTraces] = {"lde5drh1ul", "6qwnjckxre","cun8bwmo1l","a7mbxrmnpo","mzeocnl3r4"};
plotly graph = plotly("rodri16", "6hoeqn60ee", tokens, "Co2", nTraces);


char ssid[] = "MarconiLab";     //  your network SSID (name) 
char pass[] = "marconi-lab";  // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status

// initialize the library instance:
WiFiClient client;

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
 // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  
  }
   
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
  
  graph.timezone = "Europe/Rome";
  graph.maxpoints = 100;
  graph.fileopt = "extend"; // Remove this if you want the graph to be overwritten on initialization
  //graph.fileopt="overwrite"; // See the "Usage" section in https://github.com/plotly/arduino-api for details
  bool success;
  success = graph.init();
  if(!success){
    while(true){
    }
  }
  graph.openStream();

  pinMode(LED1, OUTPUT); // Set Arduino pin D5 to Output 
  pinMode(LED2, OUTPUT); // Set Arduino pin D8 to Output 
  
  Wire.begin(); // Start I2C service
  bmp.begin(); // Start BMP085 service
  
  Serial.println("Starting");  // Serial feedback
  Serial.println("AirSensor CO2 Shield Test");  // Serial feedback
  
//  // set up LCD and hi
//  slcd.begin();
//  slcd.print("hello, world!");
}

unsigned long x;
int y,t=0;

void loop() {
   // Print Update Response to Serial Monitor
  if (client.available() && status == WL_CONNECTED) ///MODIFICADO
  {
    char c = client.read();
    Serial.print(c);
  }
   client.flush();                                  //agregado
   client.stop();                                    //agregado

   //Read value from Analog Input Pin 0
  Serial.println("");  // Linefeed
  LEDon();             // Turn both LEDs on
  ReadCO2();           // Read CO2 Sensor
  ReadSHT();           // Read SHT11 Sensor
  ReadBMP();           // Read BMP085 Sensor
  PrintData();         // Print Sensor values to Serial
  LEDoff();            // Turn both LEDs off
  Serial.println("");  // Linefeed

  graph.plot(millis(), co, tokens[0]);
  graph.plot(millis(), temperature, tokens[1]);
  graph.plot(millis(), airpres, tokens[2]);
  graph.plot(millis(), humidity, tokens[3]);
  graph.plot(millis(), dewpoint, tokens[4]);

  delay(1000);
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
    
  // print your MAC address:
  byte mac[6];  
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
 
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);    
  Serial.print("BSSID: ");
  Serial.print(bssid[5],HEX);
  Serial.print(":");
  Serial.print(bssid[4],HEX);
  Serial.print(":");
  Serial.print(bssid[3],HEX);
  Serial.print(":");
  Serial.print(bssid[2],HEX);
  Serial.print(":");
  Serial.print(bssid[1],HEX);
  Serial.print(":");
  Serial.println(bssid[0],HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption,HEX);
  Serial.println();
}

//Function to read BMP085 Sensor
void ReadBMP()
{
  Serial.println("Read BMP085");  // Serial feedback
  float bmppresval(bmp.readPressure());
  delay (10);
  airpres=((bmppresval/100)+CalFactor);
}

//Function to read SHT11 Sensor
void ReadSHT()
{
  Serial.println("SHT11");
  tempSensor.measure(&temperature, &humidity, &dewpoint);
}

//Function to read CO2 Sensor
void ReadCO2()
{
  Serial.println("Read CO2");
  Wire.beginTransmission(address);
  Wire.write('R');
  Wire.endTransmission();
  
  delay(I2Cdelay);
  Wire.requestFrom(address, data_bytes);
  delay(I2Cdelay);
  while (!Wire.available ());
  for (int i = 0; i < data_bytes; ++i)
  {data_buffer [i] = Wire.read();
  delay(I2Cdelay);
  }
  
  for (int i=0;i<7;i++)
  {
   byte c = data_buffer [i];
   if (i==1) co = c;
   if (i==2) co = (co << 8) | c;
  }
}

//Function to turn both LEDs on
void LEDon()
{
    Serial.println("LED on");
    digitalWrite(LED1, HIGH);
    delay(250);
    digitalWrite(LED2, HIGH);
}


//Function to turn both LEDs off
void LEDoff()
{
    Serial.println("LED off");
    digitalWrite(LED2, LOW);
    delay(250);
    digitalWrite(LED1, LOW);
}

 
//Function to print Sensor valvues to Serial 
void PrintData()
{ 
    Serial.print("CO2: ");
    Serial.println(co);
    Serial.print("Pressure: ");
    Serial.println(airpres,2);
    Serial.print("Temperature: ");
    Serial.println(temperature,2);
    Serial.print("Relative Humidity: ");
    Serial.println(humidity,2);
    Serial.print("Dew Point: ");
    Serial.println(dewpoint,2);   
}

