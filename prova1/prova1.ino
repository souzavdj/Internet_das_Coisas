#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// --- DHT ---
#include <DHT.h>


#define DHTPIN D3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//defines de id mqtt e tópicos para publicação e subscribe

#define TOPICO_SUBSCRIBE_P1 "denis/LED"     //tópico MQTT de escuta luz 1



                                                   
#define ID_MQTT  "ID_MQTT_denis"     //id mqtt (para identificação de sessão)
                               //IMPORTANTE: este deve ser único no broker (ou seja, 
                               //            se um client MQTT tentar entrar com o mesmo 
                               //            id de outro já conectado ao broker, o broker 
                               //            irá fechar a conexão de um deles).

#define USER_MQTT  "DenisLogin"   // usuario no MQTT
#define PASS_MQTT  "teste123"  // senha no MQTT 

                               
 
//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5 
#define D2    4 
#define D3    0 
#define D4    2 
#define D5    14  
#define D6    12 
#define D7    13 
#define D8    15 
#define D9    3  
#define D10   1  




// WIFI Manager
const char* myHostnameManager = "DenisEsp"; // Nome do host na rede
const char* SSIDManager = "Denis-Vinicius-Manager"; // SSID / nome da rede WI-FI (AP) do WiFiManager 
const char* PASSWORDManager = "denis123"; // Senha da rede WI-FI (AP) do WiFiManager

                         
 
// WIFI
const char* SSID = "Denis"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "thiago123"; // Senha da rede WI-FI que deseja se conectar
 
// MQTT
const char* BROKER_MQTT = "mqtt.eclipse.org"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

long last = 0; 
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

 
//Prototypes"
void initSerial();
void init_WifiAp();
void initOTA();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
void getTempUmi(); 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    initSerial();
    init_WifiAp();
    initOTA();
    initMQTT();
    InitOutput();

}
 

//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void init_WifiAp() 
{
  WiFi.hostname(myHostnameManager);
  WiFiManager wifiManager;
   
  //wifiManager.resetSettings(); //Usado para resetar ssid e senhas armazenadas

  wifiManager.setConfigPortalTimeout(60);
  
  if(!wifiManager.autoConnect(SSIDManager, PASSWORDManager)){
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(3000);
  }
  Serial.println("Connected");
}

//Função inicializa OTA - permite carga do novo programa via Wifi
void initOTA()
{
  Serial.println();
  Serial.println("Iniciando OTA....");
  ArduinoOTA.setHostname("Denis-Vinicius-OTA"); // Define o nome da porta

  // No authentication by default
   ArduinoOTA.setPassword((const char *)"denis123"); // senha para carga via WiFi (OTA)
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
 
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
 
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
      
    }

    Serial.println("msg = " +  msg);

   if (msg.equals("ON")) // liga led
   {
      digitalWrite(D1, HIGH);
      Serial.println("Ligado led");
   }

   if (msg.equals("OFF"))
   {
      digitalWrite(D1, LOW);
      Serial.println("Desligado led");
   }  

}

 
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
//        if (MQTT.connect(ID_MQTT, USER_MQTT,PASS_MQTT)) // parameros usados para broker proprietário
                                                          // ID do MQTT, login do usuário, senha do usuário
 
           if (MQTT.connect(ID_MQTT))
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE_P1);
 
        } 
        else 
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

 
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhudm
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    
    if(WiFi.status() != WL_CONNECTED){
      //init_WifiAp();
      Serial.println("Connection dropped");
      delay(3000);
      ESP.restart(); //reinicia o ESP para iniciar o Wifi Manager 
      delay(3000);
    }
  
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

 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{

    reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
    
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
}
 
 
//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void)
{
    //enviar HIGH para o output faz o Led acender / enviar LOW faz o Led apagar)
    
    pinMode(D1, OUTPUT);
    digitalWrite(D1, LOW);
              
}


//Função: Mede a temperatura e umidade para publicar no MQTT
//Parâmetros: nenhum
//Retorno: nenhum
void getTempUmi(){
  int umidade = dht.readHumidity(); //lê a umidade
  
  char humString[8];
  dtostrf(umidade, 1, 2, humString); //passa para string

  int temperatura = dht.readTemperature(false);//lê a temperatura

  char tempString[8];
  dtostrf(temperatura, 1, 2, tempString); //passa para string
  
  MQTT.publish("denis-vini/temp", tempString);//publica a temperatura
  MQTT.publish("denis-vini/humi", humString);//publica a umidade
}
 
 
//programa principal
void loop()
{   
    // keep-alive da comunicação OTA
    ArduinoOTA.handle();

    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();

    long now = millis(); //pega o tempo atual
    if (now - last > 5000) { //compara o atual com a ultima, se passou 5 segundos publica novamente
      last = now; // guarda a ultima vez que publicou

      getTempUmi(); //mede a temperatura e umidade para publicar no MQTT
    }
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
   
}
