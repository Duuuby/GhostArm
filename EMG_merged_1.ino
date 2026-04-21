#include <WiFi.h>        // WLAN-Funktionalität (wird für ESP-NOW benötigt)
#include <esp_now.h>     // ESP-NOW Protokoll
#include <esp_wifi.h>    // Low-Level WiFi (für neue Callback-Struktur)

const int sensorPin = A0; // Sensor pin
double threshHigh = 90;  //threshold when LED lights up on certain muscle activity, upper hysteresis border
double threshLow = 10;  //lower hysteresis border
double avrg;

// MAC-Adresse des Empfängers (Zielgerät)
uint8_t broadcastAddress[] = {0x88, 0x56, 0xA6, 0x2A, 0x3F, 0x98};

// Datenstruktur → wird über Funk gesendet
// Muss beim Empfänger IDENTISCH sein (Speicherlayout!)
typedef struct struct_message {
  bool strength;    // einziger übertragener Zustand
} struct_message;

// Instanz der Datenstruktur
struct_message myData;

// Struktur für Peer-Konfiguration (Empfänger)
esp_now_peer_info_t peerInfo;

// Callback: wird aufgerufen, nachdem ein Paket gesendet wurde
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Sende-Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FEHLER");
}

void setup() {
  Serial.begin(115200); 
  delay(1000);            // kurze Stabilisierung

  Serial.println("Sender startet");

  // WLAN in Station-Modus setzen (Voraussetzung für ESP-NOW)
  WiFi.mode(WIFI_STA);

  // ESP-NOW initialisieren
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Fehler");
    return;
  }

  // Callback registrieren (Sendestatus)
  esp_now_register_send_cb(OnDataSent);

  // Peer-Struktur zurücksetzen
  memset(&peerInfo, 0, sizeof(peerInfo));

  // Ziel-MAC-Adresse setzen
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);

  // Peer (Empfänger) registrieren
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer Fehler");
    return;
  }
}

void loop() {
  double readValue = analogRead(sensorPin);  //reads the sensors current output value
  // avrg = avrgValue(30000,readValue);
  avrg = 80;    //Hard coded!!!

  Serial.println(readValue);
  Serial.println(",0,600"); //Serial Plotter min and max borders to stop plotter from adjusting y-axis to every input value

  if(avrg>=threshHigh){    //if-condition for hysteresis
    myData.strength = true;
  }else if(avrg<=threshLow){
    myData.strength = false;
  }

  // Daten senden
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}

double avrgValue(int n, double value){

  double avrg; 
  double sum;
  int i=1;

  while(i<=n){  //sum up value until threshold is reached, then divide by number of values
    sum += value;
    i++;
    if(i==n){
      avrg = sum/n;
      sum=0;
    }
  }
  return avrg;
}