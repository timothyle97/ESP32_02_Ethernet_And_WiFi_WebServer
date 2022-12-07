#include <Arduino.h>
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include <WiFi.h>
#include <aWOT.h>
#include <UIPEthernet.h>  /*< For Enc28J60 Module */
#include <Ethernet.h>     /*< For W5500 Module */
#include "MimeTypes.h"

#define ETH_SPI_MOSI      GPIO_NUM_21
#define ETH_SPI_MISO      GPIO_NUM_4
#define ETH_SPI_SCk       GPIO_NUM_19
#define ETH_SPI_CS        GPIO_NUM_18

#define ETH_INT           GPIO_NUM_2
#define ETH_RST           GPIO_NUM_5

#define WIFI_SSID         "Deneme_WiFi"
#define WIFI_PASSWORD     "123456789"

#define PORT              80


EthernetServer EthWebServer(PORT);
WiFiServer WiFiWebServer(PORT);

Application WiFi_WebApp;
Application ETH_WebApp;



/***********************************************
 * @brief Static function definitions
 ***********************************************/
static void ResetEthModule(void);
static void Ethernet_index_Cmd(Request &Request, Response &Response);
static void WiFi_index_Cmd(Request &Request, Response &Response) ;
static void MainJS_Cmd(Request &Request, Response &Response);
static void StyleCSS_Cmd(Request &Request, Response &Response);
static void getInfo_WiFi_Cmd(Request &Request, Response &Response);
static void getInfo_Eth_Cmd(Request &Request, Response &Response); 

IPAddress WiFi_Ip;
IPAddress Eth_Ip;

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR mounting SPIFFS");
    ESP.restart();
  }

Serial.println("**********************************************");
Serial.println("    ESP32 #02 Ethernet And WiFi Web Server    ");
Serial.println("**********************************************\n\n");


  SPI.begin(ETH_SPI_SCk, ETH_SPI_MISO, ETH_SPI_MOSI); /*< SPI Pin initialization.  */

  ResetEthModule();  /*< Reset Ethernet module */

  uint8_t Eth_MAC[6];
  esp_read_mac(Eth_MAC, ESP_MAC_ETH); /*< Set Ethernet MAC address.  */

  Ethernet.init(ETH_SPI_CS);  /*< Initialize Ethernet module */

  Serial.println("ETHERNET Connection...");
  Ethernet.begin(Eth_MAC);  /*< Start Ethernet module. */

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet module not found:(");
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is disconnected.");
  }

  Serial.println("WIFI Connection...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    static uint16_t Count = 0;
    if (++Count > 20) { break; }
    vTaskDelay(500);
  }
  Serial.println(" ");

  Serial.print("Ethernet IP : ");
  Serial.println(Ethernet.localIP());
  Eth_Ip = Ethernet.localIP();
  Serial.print("WiFi IP : ");
  Serial.println(WiFi.localIP());
  WiFi_Ip = WiFi.localIP();

  WiFi_WebApp.get("/", &WiFi_index_Cmd);
  WiFi_WebApp.get("/main.js", &MainJS_Cmd);
  WiFi_WebApp.get("/style.css", &StyleCSS_Cmd);
  WiFi_WebApp.get("/getInfo", &getInfo_WiFi_Cmd);

  ETH_WebApp.get("/", &Ethernet_index_Cmd);
  ETH_WebApp.get("/main.js", &MainJS_Cmd);
  ETH_WebApp.get("/style.css", &StyleCSS_Cmd);
  ETH_WebApp.get("/getInfo", &getInfo_Eth_Cmd);

  EthWebServer.begin();
  WiFiWebServer.begin();
}


void loop() {

    WiFiClient NewWiFiClient = WiFiWebServer.available();

    if (NewWiFiClient.connected()) {
      WiFi_WebApp.process(&NewWiFiClient);
      NewWiFiClient.stop();
    }

    EthernetClient NewEthClient = EthWebServer.available();

    if (NewEthClient.connected()) {
      ETH_WebApp.process(&NewEthClient);
      NewEthClient.stop();
    }
}

/*
 * @brief Reset Ethernet Module
 * 
 */
static void ResetEthModule(void)
{
  pinMode(ETH_RST, OUTPUT);

  digitalWrite(ETH_RST, HIGH);
  delay(250);
  digitalWrite(ETH_RST, LOW);
  delay(50);
  digitalWrite(ETH_RST, HIGH);
  delay(250);
} 


/*
 * @brief HTML sent over Ethernet
 * 
 * @param Request 
 * @param Response 
 */
static void Ethernet_index_Cmd(Request &Request, Response &Response) 
{
  char FilePath[] = "/Eth_index.html";

  if (SPIFFS.exists(FilePath)) {
    File File = SPIFFS.open(FilePath);
    Response.set("Content-Type", MimeTypes::getType(File.name()));

    while (File.available()) {
      Response.write(File.read());
    }
    Response.end();
  }
  else {
    Response.sendStatus(404);
  }
}

/*
 * @brief HTML sent over WiFi
 * 
 * @param Request 
 * @param Response 
 */
static void WiFi_index_Cmd(Request &Request, Response &Response) 
{
  char FilePath[] = "/WiFi_index.html";

  if (SPIFFS.exists(FilePath)) {
    File File = SPIFFS.open(FilePath);
    Response.set("Content-Type", MimeTypes::getType(File.name()));

    while (File.available()) {
      Response.write(File.read());
    }
    Response.end();
  }
  else {
    Response.sendStatus(404);
  }
}


/*
 * @brief Send the main.js file
 * 
 * @param Request 
 * @param Response 
 */
static void MainJS_Cmd(Request &Request, Response &Response) 
{
  char FilePath[] = "/main.js";

  if (SPIFFS.exists(FilePath)) {
    File File = SPIFFS.open(FilePath);
    Response.set("Content-Type", MimeTypes::getType(File.name()));

    while (File.available()) {
      Response.write(File.read());
    }
    Response.end();
  }
  else {
    Response.sendStatus(404);
  }
}

/*
 * @brief Send the style.css file
 * 
 * @param Request 
 * @param Response 
 */
static void StyleCSS_Cmd(Request &Request, Response &Response) 
{
  char FilePath[] = "/style.css";

  if (SPIFFS.exists(FilePath)) {
    File File = SPIFFS.open(FilePath);
    Response.set("Content-Type", MimeTypes::getType(File.name()));

    while (File.available()) {
      Response.write(File.read());
    }
    Response.end();
  }
  else {
    Response.sendStatus(404);
  }
}

/*
 * @brief Return WiFi and Ethernet information (via WiFi)
 * 
 * @param Request 
 * @param Response 
 */
static void getInfo_WiFi_Cmd(Request &Request, Response &Response) 
{
  DynamicJsonDocument InfoJSON(256);
  String InfoString;

  JsonArray Info_IP_WiFi = InfoJSON.createNestedArray("WiFi_IP");
  Info_IP_WiFi.add(WiFi_Ip.operator[](0));
  Info_IP_WiFi.add(WiFi_Ip.operator[](1));
  Info_IP_WiFi.add(WiFi_Ip.operator[](2));
  Info_IP_WiFi.add(WiFi_Ip.operator[](3));

  JsonArray Info_IP_Eth = InfoJSON.createNestedArray("Eth_IP");
  Info_IP_Eth.add(Eth_Ip.operator[](0));
  Info_IP_Eth.add(Eth_Ip.operator[](1));
  Info_IP_Eth.add(Eth_Ip.operator[](2));
  Info_IP_Eth.add(Eth_Ip.operator[](3));

  serializeJson(InfoJSON, InfoString);

  Response.set("Content-Type", "application/json");
  Response.print(InfoString);
  Response.end();
}

/*
 * @brief Return WiFi and Ethernetn information (via Ethernet)
 * 
 * @param Request 
 * @param Response 
 */
static void getInfo_Eth_Cmd(Request& Request, Response& Response)
{
  DynamicJsonDocument InfoJSON(256);
  String InfoString;

  JsonArray Info_IP_WiFi = InfoJSON.createNestedArray("WiFi_IP");
  Info_IP_WiFi.add(WiFi_Ip.operator[](0));
  Info_IP_WiFi.add(WiFi_Ip.operator[](1));
  Info_IP_WiFi.add(WiFi_Ip.operator[](2));
  Info_IP_WiFi.add(WiFi_Ip.operator[](3));

  JsonArray Info_IP_Eth = InfoJSON.createNestedArray("Eth_IP");
  Info_IP_Eth.add(Eth_Ip.operator[](0));
  Info_IP_Eth.add(Eth_Ip.operator[](1));
  Info_IP_Eth.add(Eth_Ip.operator[](2));
  Info_IP_Eth.add(Eth_Ip.operator[](3));

  serializeJson(InfoJSON, InfoString);

  Response.set("Content-Type", "application/json");
  Response.print(InfoString);
  Response.end();
}
