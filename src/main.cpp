#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h> // standard library
#include <web_page.h>

/* CONSTANTS ===============================*/
#define TESTING_NEW_LIB 1
//#define LED_BUILTIN 2

/* WIFI CREDENTIALS*/
const char *ssid = "BINHPHAM";
const char *password = "8534779a";

/* VARIABLES ===============================*/
WebServer server(80);
char XML[2048];

/* I/O STATES ===============================*/
bool led_builtin_state = false;

void SendWebsite();
void SendXML();
void turnon_led_builtin();

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", SendWebsite);
  server.on("/xml", SendXML);
  server.on("/BUTTON_0", turnon_led_builtin);
  // server.on("/xml", SendXML);

  server.begin();
}

void loop()
{
  server.handleClient();
}

void SendWebsite()
{
  Serial.println("sending web page");
  server.send(200, "text/html", PAGE_MAIN);
}

void turnon_led_builtin()
{
  // switch state
  led_builtin_state = !led_builtin_state;
  digitalWrite(LED_BUILTIN, led_builtin_state);

  if (led_builtin_state)
  {
    server.send(200, "text/plain", "1"); // Send web page
  }
  else
  {
    server.send(200, "text/plain", "0"); // Send web page
  }
}


void SendXML() {
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // // send bitsA0
  // sprintf(buf, "<B0>%d</B0>\n", BitsA0);
  // strcat(XML, buf);
  // // send Volts0
  // sprintf(buf, "<V0>%d.%d</V0>\n", (int) (VoltsA0), abs((int) (VoltsA0 * 10)  - ((int) (VoltsA0) * 10)));
  // strcat(XML, buf);

  // // send bits1
  // sprintf(buf, "<B1>%d</B1>\n", BitsA1);
  // strcat(XML, buf);
  // // send Volts1
  // sprintf(buf, "<V1>%d.%d</V1>\n", (int) (VoltsA1), abs((int) (VoltsA1 * 10)  - ((int) (VoltsA1) * 10)));
  // strcat(XML, buf);

  // show led0 status
  if (LED_BUILTIN) {
    strcat(XML, "<LED>1</LED>\n");
  }
  else {
    strcat(XML, "<LED>0</LED>\n");
  }

  // if (SomeOutput) {
  //   strcat(XML, "<SWITCH>1</SWITCH>\n");
  // }
  // else {
  //   strcat(XML, "<SWITCH>0</SWITCH>\n");
  // }

  strcat(XML, "</Data>\n");

  Serial.println(XML);

  /* Send XML to server with value of 200ms*/
  server.send(200, "text/xml", XML);
}
