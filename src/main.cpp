#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h> // standard library
#include <web_page.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* CONSTANTS ===============================*/
#define MOIST_SENSE A4
#define VBAT_SENSE A5

#define SENSOR_UPD_INTERVAL 50
#define SERVER_UPD_INTERVAL 1000

/* WIFI CREDENTIALS*/
const char *ssid = "BINHPHAM";
const char *password = "8534779a";

/* VARIABLES ===============================*/
WebServer server(80);
char XML[2048];
char buf[32];

uint16_t moist_val;

uint32_t sensor_upd_timer = 0;
uint32_t server_upd_timer = 0;

/* For I2C*/

#define LCD_ADDRESS 0x27
#define LCD_COL 16
#define LCD_ROW 2

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COL, LCD_ROW);
/* I/O STA1TES ===============================*/
bool led_builtin_state = false;

void SendWebsite(void);
void SendXML(void);
void turnon_led_builtin(void);
void lcd_print_ip_adr(void *parameter);
void lcd_counter(void *parameter);
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

    lcd.clear();
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

  lcd.clear();

  xTaskCreate(
      lcd_counter, // Function name of the task
      "lcd_counter",   // Name of the task (e.g. for debugging)
      2048,        // Stack size (bytes)
      NULL,        // Parameter to pass
      1,           // Task priority
      NULL         // Task handle
  );

  xTaskCreate(
      lcd_print_ip_adr, // Function name of the task
      "lcd_print_ip_adr",      // Name of the task (e.g. for debugging)
      2048,             // Stack size (bytes)
      NULL,             // Parameter to pass
      2,                // Task priority
      NULL              // Task handle
  );

  xTaskCreate(
      upd_sensor, // Function name of the task
      "upd_sensor",   // Name of the task (e.g. for debugging)
      2048,          // Stack size (bytes)
      NULL,          // Parameter to pass
      3,             // Task priority
      NULL           // Task handle
  );

  xTaskCreate(
      upd_webpage, // Function name of the task
      "upd_webpage", // Name of the task (e.g. for debugging)
      2048,        // Stack size (bytes)
      NULL,        // Parameter to pass
      4,           // Task priority
      NULL         // Task handle
  );
}
void lcd_counter(void *parameter)
{
  uint32_t count = 0;
  for (;;)
  {
    lcd.setCursor(0, 1);
    lcd.print(count++);
    delay(1000);
  }
}

void lcd_print_ip_adr(void *parameter)
{
  for (;;)
  {
    lcd.setCursor(0, 0);
    lcd.print(WiFi.localIP());
    delay(5000);
  }
}

void upd_sensor(void *parameter)
{
  for (;;)
  {
    moist_val = analogRead(MOIST_SENSE) >> 9;
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
