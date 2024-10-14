#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h> // standard library
#include <web_page.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* CONSTANTS ===============================*/
#define MOIST_SENSE A4
#define VBAT_SENSE A6

#define SENSOR_UPD_INTERVAL 50
#define SERVER_UPD_INTERVAL 1000
#define IP_ADR_UPD_INTERVAL 10000
#define LCD_UPD_INTERVAL 1000

#define LCD_ADDRESS 0x27
#define LCD_COL 16
#define LCD_ROW 2
#define KB_SIZE 1024

#define PR_UPD_WEBPAGE 4
#define PR_UPD_IP_ADR 2
#define PR_UPD_COUNTER 1
#define PR_UPD_SENSOR 3

/* WIFI CREDENTIALS*/
const char *ssid = "BINHPHAM";
const char *password = "8534779a";

/* VARIABLES ===============================*/
WebServer server(80);
char XML[2048];
char buf[32];

uint16_t moist_val;
uint16_t vbat;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COL, LCD_ROW);
/* I/O STA1TES ===============================*/
bool led_builtin_state = false;

void SendWebsite(void);
void SendXML(void);
void turnon_led_builtin(void);
void lcd_print_ip_adr(void *parameter);
void lcd_print_vbat(void *parameter);
void upd_sensor(void *parameter);
void upd_webpage(void *parameter);

void setup()
{
  setCpuFrequencyMhz(80);

  lcd.init();
  lcd.setBacklight(1);

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOIST_SENSE, INPUT);

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

    lcd.setCursor(0, 0);
    lcd.print("Connecting");

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

  server.begin();

  xTaskCreate(
      lcd_print_ip_adr,   // Function name of the task
      "lcd_print_ip_adr", // Name of the task (e.g. for debugging)
      KB_SIZE * 5,        // Stack size (bytes)
      NULL,               // Parameter to pass
      PR_UPD_IP_ADR,      // Task priority
      NULL                // Task handle
  );

  xTaskCreate(
      lcd_print_vbat,   // Function name of the task
      "lcd_print_vbat", // Name of the task (e.g. for debugging)
      KB_SIZE * 5,      // Stack size (bytes)
      NULL,             // Parameter to pass
      PR_UPD_COUNTER,   // Task priority
      NULL              // Task handle
  );

  xTaskCreate(
      upd_sensor,    // Function name of the task
      "upd_sensor",  // Name of the task (e.g. for debugging)
      KB_SIZE * 2,   // Stack size (bytes)
      NULL,          // Parameter to pass
      PR_UPD_SENSOR, // Task priority
      NULL           // Task handle
  );

  xTaskCreate(
      upd_webpage,    // Function name of the task
      "upd_webpage",  // Name of the task (e.g. for debugging)
      KB_SIZE * 10,   // Stack size (bytes)
      NULL,           // Parameter to pass
      PR_UPD_WEBPAGE, // Task priority
      NULL            // Task handle
  );
}

void lcd_print_ip_adr(void *parameter)
{
  for (;;)
  {
    delay(IP_ADR_UPD_INTERVAL);
  }
}

void lcd_print_vbat(void *parameter)
{
  for (;;)
  {
    if (vbat != 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(WiFi.localIP());
      lcd.setCursor(0, 1);
      float vbat_sense = ((float)vbat * ((float)SENSOR_UPD_INTERVAL / (float)LCD_UPD_INTERVAL))*0.00525;
      lcd.print(vbat_sense);
      vbat = 0;
    }
    delay(LCD_UPD_INTERVAL);
  }
}

void upd_sensor(void *parameter)
{
  pinMode(VBAT_SENSE, INPUT);
  pinMode(MOIST_SENSE, INPUT);

  for (;;)
  {
    moist_val = analogRead(MOIST_SENSE) >> 9;
    uint16_t vbat_temp = analogRead(VBAT_SENSE);
    Serial.println(vbat_temp);
    vbat += vbat_temp;
    delay(SENSOR_UPD_INTERVAL);
  }
}

void upd_webpage(void *parameter)
{
  for (;;)
  {
    server.handleClient();
    delay(SERVER_UPD_INTERVAL);
  }
}

void loop()
{

  // delay(SENSOR_UPD_INTERVAL);
}

/* Web API handling*/
void SendWebsite(void)
{
  Serial.println("User connects, updating web page");
  server.send(200, "text/html", PAGE_MAIN);
}

void turnon_led_builtin(void)
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

void SendXML(void)
{
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // // send bitsA0
  // sprintf(buf, "<B0>%d</B0>\n", BitsA0);
  // strcat(XML, buf);
  // send Volts0
  sprintf(buf, "<V0>%d.%d</V0>\n", moist_val, 0);
  strcat(XML, buf);

  if (led_builtin_state)
  {
    strcat(XML, "<LED>1</LED>\n");
  }
  else
  {
    strcat(XML, "<LED>0</LED>\n");
  }

  strcat(XML, "</Data>\n");
  /* Send XML to server with value of 200ms*/
  server.send(300, "text/xml", XML);
}
