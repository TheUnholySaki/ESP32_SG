#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h> // standard library
#include <web_page.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* CONSTANTS ===============================*/
#define MOIST_SENSE A4

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
void lcd_print_ip_adr(IPAddress ip_adr);

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

  // initialize LCD
  
  // turn on LCD backlight
  // lcd.backlight();
  sensor_upd_timer = millis();
}
int count = 0;

void loop()
{
  /* Update sensor */
  if ((millis() - sensor_upd_timer) > SENSOR_UPD_INTERVAL)
  {
    sensor_upd_timer = millis();

    moist_val = analogRead(MOIST_SENSE) >> 9;
  }

  /* Refresh webpage */
  if ((millis() - server_upd_timer) > SERVER_UPD_INTERVAL)
  {
    server_upd_timer = millis();

    server.handleClient();
    lcd.clear();
    lcd_print_ip_adr(WiFi.localIP());
    lcd.setBacklight(1);
    
  }
}

void lcd_print_ip_adr(IPAddress ip_adr)
{
  lcd.setCursor(0, 0);
  lcd.print(ip_adr);
}

int testing;

void SendWebsite(void)
{
  Serial.println("User connects, updating web page");
  
  lcd.setCursor(0, 1);
  lcd.print("Hi user");
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
