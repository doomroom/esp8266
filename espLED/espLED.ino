/*
 * 
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

// Serial printing ON/OFF
#include "Arduino.h"
#define DEBUG false
#define Serial if(DEBUG)Serial
#define DEBUG_OUTPUT Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// if you use GPIO1 set Serial #define to false
#define LED_PIN   4 // ESP-01 blue LED, GPIO1
// #define LED_PIN  16 // NodeMCU blue LED
#define LED_ON    digitalWrite( LED_PIN, LOW )
#define LED_OFF   digitalWrite( LED_PIN, HIGH )

const char* myssid  = ".....";
const char* mypass  = "1234";

ESP8266WebServer server ( 80 );

char tmpstr[40];

extern "C" {
#include "user_interface.h"
uint32_t readvdd33(void);
}

void setup() {
  uint8_t i;

  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.print(F("Heap: ")); Serial.println(system_get_free_heap_size());
  Serial.print(F("Boot Vers: ")); Serial.println(system_get_boot_version());
  Serial.print(F("CPU: ")); Serial.println(system_get_cpu_freq());
  Serial.print(F("SDK: ")); Serial.println(system_get_sdk_version());
  Serial.print(F("Chip ID: ")); Serial.println(system_get_chip_id());
  Serial.print(F("Flash ID: ")); Serial.println(spi_flash_get_id());
  // Serial.print(F("Flash Size: ")); Serial.println(???());
  Serial.print(F("Vcc: ")); Serial.println(readvdd33());
  Serial.println();

  delay( 5000 );
  pinMode( LED_PIN, OUTPUT );

  // let them know we're alive
  for ( i=0; i<10; i++ ) {
    LED_ON;
    delay(40);
    LED_OFF;
    delay(80);
  }
  LED_OFF;

  initWiFi();

  setupWebServer();

}

void loop() {
  server.handleClient();
  delay(1);
}

void initWiFi(void) {

  WiFi.softAP( myssid, mypass );
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

}

String HTML_Root = "<!doctype html> <html> <head> ESP8266 Web Server </head> <body>\
  <button onclick=\"window.location.href='/ledon'\">LED ON</button>\
  <button onclick=\"window.location.href='/ledoff'\">LED OFF</button>\
</body> </html>";

String HTML_LED_ON = "<!doctype html> <html> <head>\
<script>window.onload = function() { setInterval(function() {window.location.replace('/');}, 1500); };</script>\
</head> <body> <h1>LED is now ON!</h1> </body> </html>";

String HTML_LED_OFF = "<!doctype html> <html> <head>\
<script>window.onload = function() { setInterval(function() {window.location.replace('/');}, 1500); };</script>\
</head> <body> <h1>LED is now OFF!</h1> </body> </html>";

void handleLEDON() {
  Serial.println("LED ON");
  ElapsedStr( tmpstr );
  Serial.println( tmpstr );
  server.send ( 200, "text/html", HTML_LED_ON );
  LED_ON;
}

void handleLEDOFF() {
  Serial.println("LED OFF");
  ElapsedStr( tmpstr );
  Serial.println( tmpstr );
  server.send ( 200, "text/html", HTML_LED_OFF );
  LED_OFF;
}

void handleRoot() {
  server.send ( 200, "text/html", HTML_Root );
}

void setupWebServer(void) {

  server.on ( "/", handleRoot );

  server.on ( "/favicon.ico", []() {
    Serial.println("favicon.ico");
    ElapsedStr( tmpstr );
    Serial.println( tmpstr );
    server.send ( 200, "text/html", "" ); // better than 404
  } );

  server.on ( "/ledon", handleLEDON );

  server.on ( "/ledoff", handleLEDOFF );

  server.onNotFound ( []() {
    Serial.println("Page Not Found");
    server.send ( 404, "text/html", "Page not Found" );
  } );

  server.begin();

}

void ElapsedStr( char *str ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( str, "Elapsed " );
  if ( hour == 0 ) {
    sprintf( str, "%s   ", str );
  } else {
    sprintf( str, "%s%2d:", str, hour );
  }
  if ( minute >= 10 ) {
    sprintf( str, "%s%2d:", str, minute );
  } else {
    if ( hour != 0 ) {
      sprintf( str, "%s0%1d:", str, minute );
    } else {
      sprintf( str, "%s ", str );
      if ( minute == 0 ) {
        sprintf( str, "%s  ", str );
      } else {
        sprintf( str, "%s%1d:", str, minute );
      }
    }
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( str, "%s0%1d", str, ( sec % 60 ) );
  } else {
    sprintf( str, "%s%2d", str, ( sec % 60 ) );
  }

}// - See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3787&sid=718449baba11b6e2e052716e99eecc9b&start=4#sthash.IHC0Q8br.dpuf
