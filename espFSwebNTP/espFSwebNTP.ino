#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>

#define VERSION   1010

const char* urlbase = "http://raspberrypiled.local/hygrotemp/";

const char* ssid = "SSIDdeAPwifi";
const char* password = "mot_de_passe_wifi";
char myhostname[32];
const char* otapass = "123456";

unsigned long previousMillis = 0;

//WiFiUDP ntpUDP;
// serveur, offset (s), update (ms)
//NTPClient timeClient(ntpUDP, "raspberrypiled.local", 0, 60000);

void confOTA() {
  // Port 8266 (défaut)
  ArduinoOTA.setPort(8266);

  // Hostname défaut : esp8266-[ChipID]
  EEPROM.get(0, myhostname);
  ArduinoOTA.setHostname(myhostname);

  // mot de passe pour OTA
  ArduinoOTA.setPassword(otapass);

  ArduinoOTA.onStart([]() {
    Serial.println("/!\\ Maj OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n/!\\ MaJ terminee");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression: %u%%\r", (progress / (total / 100)));
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

void maj() {
  // Accès au fichier "version"
  String urlversion = String(urlbase);
  urlversion.concat("version");
  Serial.println("Vérification version...");
  HTTPClient httpClient;
  httpClient.begin(urlversion);
  int httpCode = httpClient.GET();

  // réponse du serveur ok ?
  if(httpCode == HTTP_CODE_OK) {
    // lecture du fichier
    String nVersion = httpClient.getString();
    Serial.print("Version actuelle: ");
    Serial.println(VERSION);
    Serial.print( "Version disponible: " );
    nVersion.trim();
    Serial.println(nVersion.toInt());
    // La version dispo est supérieure à la mienne ?
    if(nVersion.toInt() > VERSION) {
      Serial.print("Mise à jour disponible: ");
      // composition de l'URL
      String urlfirmware = String(urlbase);
      urlfirmware.concat("firmware.");
      urlfirmware.concat(nVersion);
      Serial.println(urlfirmware);
      Serial.println("Mise à jour...");
      // mise à jour
      t_httpUpdate_return ret = ESPhttpUpdate.update(urlfirmware);
      if(ret == HTTP_UPDATE_FAILED) {
        // on n'arrive pas ici si ça marche car l'ESP reboot
        Serial.print("Erreur mise à jour (");
        Serial.print(ESPhttpUpdate.getLastError());
        Serial.print(") : ");
        Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
      }
    } else {
      // VERSION >= au contenu de "version"
      Serial.println("Déjà à jour");
    }
  } else {
    // Mais où est "version" ??
    Serial.println("Erreur récupération de version !!!");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Go Go Go...");

  // activation émulation EEPROM 48 octets
  EEPROM.begin(48);

  /*
  strcpy(myhostname, "espOTAdatafs");
  EEPROM.put(0, myhostname);
  EEPROM.commit();
  */

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  // Affichage adresse MAC
  Serial.print("info: MAC = ");
  if(WiFi.macAddress(mac) != 0) {
    for(int i=0; i<WL_MAC_ADDR_LENGTH; i++) {
      if(mac[i]<16) Serial.print ("0");
      Serial.print(mac[i],HEX);
      Serial.print((i < WL_MAC_ADDR_LENGTH-1) ? ":" : "\n\r");
    }
  Serial.println("");
  }

  // connexion Wifi client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Erreur connexion Wifi! Reboot...");
    delay(5000);
    ESP.restart();
  }

  // Configuration OTA
  confOTA();

  // affichage infos de base
  Serial.print(">>> Nom host: ");
  Serial.println(myhostname);
  Serial.print(">>> Adresse IP: ");
  Serial.println(WiFi.localIP());

  /*
  timeClient.begin();
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  */

  maj();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 10000) {
    previousMillis = currentMillis;
    //Serial.println("pouet !");
  }
  ArduinoOTA.handle();
}
