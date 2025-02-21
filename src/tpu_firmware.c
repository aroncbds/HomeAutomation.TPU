#include <Arduino_FreeRTOS.h>
#include <Ethernet2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <semphr.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define LCD_1_I2C_ADDR     0x27
#define LCD_2_I2C_ADDR  0x21
#define LCD_3_I2C_ADDR     0x22

#define ONE_WIRE_BUS 2  // DS18B20 data pin

// https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/12-xSemaphoreTake

void TaskUpdateDisplays(void *pvParameters);
void TaskReadTempSensors(void *pvParameters);

static SemaphoreHandle_t snprintfMutex;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureT1T = 0.0;
float temperatureT1M = 0.0;
float tempProbe = 0.0;

LiquidCrystal_I2C lcd_1(LCD_1_I2C_ADDR, 20, 4);
LiquidCrystal_I2C lcd_2(LCD_2_I2C_ADDR, 20, 4);
LiquidCrystal_I2C lcd_3(LCD_3_I2C_ADDR, 20, 4);

// Ethernet setup
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
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


    // // locate devices on the bus
    // Serial.println("Locating devices...");
    // Serial.print("Found ");
    // deviceCount = sensors.getDeviceCount();
    // Serial.print(deviceCount, DEC);
    // Serial.println(" devices.");
    // Serial.println("");
    
    // Serial.println("Printing addresses...");
    // for (int i = 0;  i < deviceCount;  i++)
    // {
    //   Serial.print("Sensor ");
    //   Serial.print(i+1);
    //   Serial.print(" : ");
    //   sensors.getAddress(Thermometer, i);
    //   printAddress(Thermometer);
    // }


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
  lcd_3.backlight();

  // The following doesn't seem to work
  // lcd_t1_top.setCursor(0, 0);
  // lcd_t1_top.print("FreeRTOS: 11.1.0");
  // lcd_t1_top.setCursor(0, 1);
  // lcd_t1_top.print("Starting TPU...");

  Ethernet.begin(mac, ip);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  xTaskCreate(TaskReadTempSensors, "ReadTempSensors", 128, NULL, 1, NULL);
  xTaskCreate(TaskWebServer, "WebServer", 128, NULL, 2, NULL);
  xTaskCreate(TaskUpdateDisplays, "UpdateDisplays", 128, NULL, 3, NULL);
}


void loop() {}

void TaskWebServer(void *pvParameters)
{
  (void)pvParameters;

  server.begin();
  
  for (;;) {
    EthernetClient client = server.available();
    if (client) {
      String request = "";
      while (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n' && request.endsWith("\r\n\r\n")) {
          break;
        }
      }

      // Serve temperature JSON response
      if (request.indexOf("GET /temperatures") >= 0) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.print("{\"t1t\":");
        client.print(temperatureT1T);
        client.print(",");
        client.print("\"t1m\":");
        client.print(temperatureT1M);
        client.print(",");
        client.print("\"t2t\":");
        client.print(tempProbe);
        client.println("}");
      }

      client.stop();
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Avoid busy loop
  }
}

void TaskUpdateDisplays(void *pvParameters)
{
  (void)pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    char buffer[15];

    // if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
    //     snprintf(buffer, sizeof(buffer), "T1 Topp: %d C", temperatureC);
    //     xSemaphoreGive(snprintfMutex);
    // }

    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[13];
        dtostrf(temperatureT1T, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T1 Top: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print(buffer);

    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[13];
        dtostrf(temperatureT1M, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T1 Mid: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }

    lcd_1.setCursor(0, 1);
    lcd_1.print(buffer);

    lcd_2.clear();
    lcd_2.setCursor(0, 0);
    lcd_2.print("T1 Bot: N/A");
    lcd_2.setCursor(0, 1);
    lcd_2.print("IP:192.168.1.177");
    
    if (xSemaphoreTake(snprintfMutex, portMAX_DELAY) == pdTRUE) {
        char tempStr[13];
        dtostrf(tempProbe, 6, 2, tempStr);
        snprintf_P(buffer, sizeof(buffer), PSTR("T2 Top: %sC"), tempStr);
        xSemaphoreGive(snprintfMutex);
    }

    lcd_3.clear();
    lcd_3.setCursor(0 ,0);
    lcd_3.print(buffer);
    lcd_3.setCursor(0, 1);
    lcd_3.print("T3 Top: N/A");
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

  // TODO: We need to fetch the temperature by address
  for (;;) {
    sensors.requestTemperatures();
    temperatureT1T = sensors.getTempCByIndex(0);
    temperatureT1M = sensors.getTempCByIndex(1);
    tempProbe = sensors.getTempCByIndex(2);
    vTaskDelay(pdMS_TO_TICKS(2000));  // Run every 2 seconds
  }

  // for (;;) {
  //   int sensorValue = analogRead(A0);
  //   Serial.println(sensorValue);
  //   vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  // }
}



  // pinMode(LED_BUILTIN, OUTPUT);

  // for (;;)
  // {
  //   // placeholder, for now...
  //   digitalWrite(LED_BUILTIN, HIGH);
  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  //   digitalWrite(LED_BUILTIN, LOW);
  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }