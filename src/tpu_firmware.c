/***********************************************************************************************************************
Polls a group of 1Wire-sensors and makes the temperatures available as JSON-data on the attached LAN-interface.

References:
https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/12-xSemaphoreTake

Todo:
- Add button with timer for setting boiler cleaning interval flag
- Add button with timer and ISR-handler for enabling LCD backlight

aroncbds@yahoo.se
Version: 1.0 - 2025-02-28
***********************************************************************************************************************/

#include <Arduino_FreeRTOS.h>
#include <DallasTemperature.h>
#include <Ethernet2.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <semphr.h>
#include <Wire.h>

// Displays
#define LCD_1_I2C_ADDR    0x27
#define LCD_2_I2C_ADDR    0x21
#define LCD_3_I2C_ADDR    0x22

#define ONE_WIRE_BUS 2  // DS18B20 data pin

// DS18B20 tank sensors
uint8_t sensor1_t1t[8] = { 0x28, 0x9C, 0x50, 0x43, 0xD4, 0xE1, 0x3C, 0x04 };
uint8_t sensor2_t1m[8] = { 0x28, 0xC1, 0xA3, 0x43, 0xD4, 0xE1, 0x3C, 0x88 };
uint8_t sensor3_t2t[8] = { 0x28, 0x0F, 0x68, 0x43, 0xD4, 0xE1, 0x3C, 0x56 };
uint8_t sensor4_t3t[8] = { 0x28, 0x5C, 0x10, 0x43, 0xD4, 0xE1, 0x3C, 0x72 };

#define ONE_WIRE_POLLING_DELAY_MS 10000

void TaskUpdateDisplays(void *pvParameters);
void TaskReadTempSensors(void *pvParameters);
void TaskWebServer(void *pvParameters);

static SemaphoreHandle_t snprintfMutex;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureT1T = 0.0;
float temperatureT1M = 0.0;
float temperatureT2T = 0.0;
float temperatureT3T = 0.0;

LiquidCrystal_I2C lcd_1(LCD_1_I2C_ADDR, 20, 4);
LiquidCrystal_I2C lcd_2(LCD_2_I2C_ADDR, 20, 4);
LiquidCrystal_I2C lcd_3(LCD_3_I2C_ADDR, 20, 4);

// Ethernet setup
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0xA9, 0x1A };
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

// Trying to fetch device addresses of temperature probes (perhaps we need to create a one-off application for this purpose)
DeviceAddress Thermometer;  // Variable to hold device address (https://lastminuteengineers.com/multiple-ds18b20-arduino-tutorial/)
int deviceCount = 0;

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void setup() {

    Serial.begin(115200);

    sensors.begin();

    snprintfMutex = xSemaphoreCreateMutex();
    if (snprintfMutex == NULL) {
        Serial.println("Failed to create mutex!");
    } else {
        Serial.println("Mutex created successfully!");
    }

  // Initialize displays
  lcd_1.init();
  lcd_1.backlight();

  lcd_2.init();
  lcd_2.backlight();

  lcd_3.init();
  lcd_3.noBacklight();

  Ethernet.begin(mac, ip);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  xTaskCreate(TaskReadTempSensors, "ReadTempSensors", 128, NULL, 1, NULL);
  xTaskCreate(TaskWebServer, "WebServer", 256, NULL, 2, NULL);
  xTaskCreate(TaskUpdateDisplays, "UpdateDisplays", 128, NULL, 3, NULL);
}

void loop() {}

void TaskWebServer(void *pvParameters) {
  (void)pvParameters;

  // TODO - Ethernet.h (more recent version): Check for Ethernet hardware present
  // if (Ethernet.hardwareStatus() == EthernetNoHardware) {
  //   Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  //   while (true) {
  //     delay(1); // do nothing, no point running without Ethernet hardware
  //   }
  // }
  // if (Ethernet.linkStatus() == LinkOFF) {
  //   Serial.println("Ethernet cable is not connected.");
  // }

  server.begin();

  for (;;) {
    EthernetClient client = server.available();
    if (client) {
      String request = "";
      unsigned long timeout = millis();
      
      while (client.connected() && (millis() - timeout < 1000)) {  // 1 sec timeout
        while (client.available()) {
          char c = client.read();
          request += c;
          if (c == '\n' && request.endsWith("\r\n\r\n")) {
            break;
          }
        }
      }

      if (request.indexOf("GET /temperatures") >= 0) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.print("{\"t1t\":");
        client.print(temperatureT1T);
        client.print(",\"t1m\":");
        client.print(temperatureT1M);
        client.print(",\"t2t\":");
        client.print(temperatureT2T);
        client.print(",\"t3t\":");
        client.print(temperatureT3T);
        client.println("}");
      }
      client.flush();
      client.stop();
    }

    vTaskDelay(pdMS_TO_TICKS(50));  // Avoid high Ethernet load
  }
}

void TaskUpdateDisplays(void *pvParameters)
{
  (void)pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    char buffer[16];

    // LCD 1, line 1: Tank 1 TOP
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[15];
        dtostrf(temperatureT1T, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T1 Top: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print(buffer);

    // LCD 1, line 2: Tank 1 MIDDLE
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[15];
        dtostrf(temperatureT1M, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T1 Mid: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }
    lcd_1.setCursor(0, 1);
    lcd_1.print(buffer);

    // LCD 2, line 1: Tank 2 TOP
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[15];
        dtostrf(temperatureT2T, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T2 Top: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }
    lcd_2.clear();
    lcd_2.setCursor(0, 0);
    lcd_2.print(buffer);

    // LCD 2, line 2: Tank 3 TOP
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[15];
        dtostrf(temperatureT3T, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T3 Top: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }
    lcd_2.setCursor(0, 1);
    lcd_2.print(buffer);
  }
}

void safe_snprintf(char *buffer, size_t size, const char *format, ...) {
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, size, format, args);
        va_end(args);
        xSemaphoreGive(snprintfMutex);
    }
}

void TaskReadTempSensors(void *pvParameters)
{
  (void)pvParameters;

  for (;;) {
    sensors.requestTemperatures();
    temperatureT1T = sensors.getTempC(sensor1_t1t);
    temperatureT1M = sensors.getTempC(sensor2_t1m);
    temperatureT2T = sensors.getTempC(sensor3_t2t);
    temperatureT3T = sensors.getTempC(sensor4_t3t);
    vTaskDelay(pdMS_TO_TICKS(ONE_WIRE_POLLING_DELAY_MS));
  }
}
