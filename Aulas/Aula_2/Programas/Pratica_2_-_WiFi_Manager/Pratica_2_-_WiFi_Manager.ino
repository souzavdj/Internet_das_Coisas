#include <ESP8266WiFi.h>  // Inclui a biblioteca
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient

// WIFI
const char* myHostname = "vinicius"; // Nome do host na rede
const char* SSID = "viniesp8266"; // SSID / nome da rede WI-FI (AP) do WiFiManager
const char* PASSWORD = "guesp8266"; // Senha da rede WI-FI (AP) do WiFiManager

//wifiManager.resetSettings();

void init_WifiAp()
{
  WiFi.hostname(myHostname);
  WiFiManager wifiManager;

  //wifiManager.resetSettings(); //Usado para resetar sssid e senhas armazenadas

  wifiManager.autoConnect(SSID, PASSWORD);
  Serial.print("Conectado com sucesso na rede via WifiManager na rede: ");
  Serial.println(WiFi.SSID());
  Serial.println();
  Serial.print("IP obtido: ");
  Serial.print(WiFi.localIP());  // mostra o endereço IP obtido via DHCP
  Serial.println();
  Serial.print("Endereço MAC: ");
  Serial.print(WiFi.macAddress()); // mostra o endereço MAC do esp8266

}




void setup() {
  WiFiManager wifiManager;
  wifiManager.setTimeout(3);
  Serial.begin(115200); // observe a velocidade de conexão com o terminal
  init_WifiAp(); // inicia o WiFiManager
}

void loop() {
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  unsigned long tempo = 0;
  
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
    
  WiFi.hostname(myHostname);
  WiFi.begin(); // Conecta na rede WI-FI

  /*if (!wifiManager.startConfigPortal(SSID, PASSWORD)) {
    Serial.println("Sem interação!!!");
    delay(3000);
    ESP.restart();
  }*/
  
  if (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");

    Serial.println();
    Serial.print("Conectado com sucesso na rede: ");
    Serial.print(SSID);
    Serial.println();
    Serial.print("IP obtido: ");
    Serial.print(WiFi.localIP());  // mostra o endereço IP obtido via DHCP
    Serial.println();
    Serial.print("Endereço MAC: ");
    Serial.print(WiFi.macAddress()); // mostra o endereço MAC do esp8266
  }

}
