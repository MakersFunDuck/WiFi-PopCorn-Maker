/*
 * ESP8266 NodeMCU Popcorn Maker Control over WiFi by Maker's Fun Duck
 * Below I am respecting the copyright notification of the library creator's request. For the rest of the code, you are free to do anything you want. 
 *  * Copyright (c) 2016 Gianni Van Hoecke <gianni.vh@gmail.com>
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * User: Gianni Van Hoecke <gianni.vh@gmail.com>
 * Date: 23/05/16
 * Time: 20:19
 *
 * SmoothThermistor (https://github.com/giannivh/SmoothThermistor)
 * A flexible thermistor reading library.
 *
 * The components:
 * - Thermistor (here a 100K thermistor is used)
 * - Resistor (here a 100K resistor is used)
 * - Some wires
 *
 * The easy circuit:
 *
 *                  Analog pin 0
 *                        |
 *    5V |-----/\/\/\-----+-----/\/\/\-----| GND
 *
 *               ^                ^ 
 *        100K thermistor     100K resistor
 *
 * The advanced circuit:
 *
 *          AREF      Analog pin 0
 *           |              |
 *    3.3V |-+---/\/\/\-----+-----/\/\/\-----| GND
 *
 *                 ^                ^ 
 *          100K thermistor     100K resistor
 */




// include the SmoothThermistor library
#include <SmoothThermistor.h>

// create a SmoothThermistor instance, reading from analog pin 0
// using a common 10K thermistor.
SmoothThermistor smoothThermistor(A0);

// if you have a different type of thermistor, you can override the default values
// example:
// SmoothThermistor smoothThermistor(A0,              // the analog pin to read from
//                                   ADC_SIZE_10_BIT, // the ADC size
//                                   100000,           // the nominal resistance
//                                   100000,           // the series resistance
//                                   3950,            // the beta coefficient of the thermistor
//                                   25,              // the temperature for nominal resistance
//                                   10);             // the number of samples to take for each measurement






//Variables
float temperature_read = 0.0;

float set_temperature = 30; //Start with something small to be on the safe side. and slowly increase the temp if you see un-popped kernels. In my settings, my final temp. was 40 degrees Celcius.
//This temperature setting is not for controlling the heating element, but controlling the temperature levels on the upper part, so it does not melt!
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

//ESP Web Server Library to host a web page
#include <ESP8266WebServer.h>

//---------------------------------------------------------------
//Our HTML webpage contents in program memory
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>
<h1>Popcorn Maker</h1><br>
Ciclk to turn <a href="RelayOn" target="myIframe">Popcorn Maker ON</a><br>
Ciclk to turn <a href="RelayOff" target="myIframe">Popcorn Maker OFF</a><br>
Popcorn Maker State<iframe name="myIframe" width="100" height="25" frameBorder="0"><br>
<hr>

</center>

</body>
</html>
)=====";
//---------------------------------------------------------------
//On board Relay Connected to D4
#define Relay D4  

//SSID and Password of your WiFi router
const char* ssid = "Maker's Fun Duck"; //your ssid
const char* password = "1234567489*"; //your wifi password

//Declare a global object variable from the ESP8266WebServer class.
ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 Serial.println("You calRelay root page");
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleRelayon() { 
 Serial.println("Popcorn Maker ON page");
 digitalWrite(Relay,LOW); //Relay is connected in reverse
 server.send(200, "text/html", "ON"); //Send ADC value only to client ajax request
}

void handleRelayoff() { 
 Serial.println("Popcorn Maker OFF page");
 digitalWrite(Relay,HIGH); //Popcorn Maker OFF
 server.send(200, "text/html", "OFF"); //Send ADC value only to client ajax request
}
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);

  // use the AREF pin, so we can measure on 3.3v, which has less noise on an Arduino
  // make sure your thermistor is fed using 3.3v, along with the AREF pin
  // so the 3.3v output pin goes to the AREF pin and the thermistor
  // see "the advanced circuit" on top of this sketch
  smoothThermistor.useAREF(true);

  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  //Onboard Relay port Direction output
  pinMode(Relay,OUTPUT); 
  //Power on Relay state off
  digitalWrite(Relay,HIGH);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/RelayOn", handleRelayon); //as Per  <a href="RelayOn">, Subroutine to be calRelay
  server.on("/RelayOff", handleRelayoff);

  server.begin();                  //Start server
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){

   // First we read the real value of temperature and print it to the serial
  temperature_read = smoothThermistor.temperature();
  Serial.print("  Temperature = ");
  // print the temperature
  Serial.println(smoothThermistor.temperature());
 if (temperature_read  > set_temperature)
   {   
    handleRelayoff(); 
      Serial.println("Overheat protection= process stopped! Wait device to cool down"); 
   }
  
  server.handleClient();          //Handle client requests
}
