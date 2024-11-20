#include <WiFi.h>
#include <esp_now.h>
#include "DHT.h"

#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Alınan veri yapısı
typedef struct struct_message {
  float yükseklik1;  // Görev yükünden alınacak yükseklik1
  float yükseklik2;  // BMP085 sensöründen gelen yükseklik
  float basınç1;     // Görev yükünden alınacak basınç1
  float basınç2;     // BMP085 sensöründen gelen basınç
  int paketno;       // Paket numarası
  float sıcaklık;    // Görev yükünden alınacak sıcaklık
  float irtifafarkı; 
  float inişhızı;
  float pilgerilimi;
  float pitch;
  float roll;
  float yaw;
  char rhrh[10];     // Karakter dizisi
  float iotdata;     // Yer istasyonundan alınacak sıcaklık (DHT11)
  int takımno;
  int uydustatüsü;
  int hatakodu;
  char göndermesaati[25]; // Karakter dizisi
} struct_message;

struct_message mySensorData;
struct_message incomingData;  // Gelen veri için kullanılacak yapı

uint8_t broadcastAddress[] = { 0xA0, 0xA3, 0xB3, 0x8A, 0x71, 0xE0 };

// Veri alındığında çalışacak callback fonksiyonu
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataRaw, int len) {
  // Gelen verinin boyutunu kontrol et
  if (len == sizeof(incomingData)) {
    // Gelen veriyi struct_message yapısına kopyala
    memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

    // Gelen verileri seri port üzerinden yazdır
    Serial.printf("%d, %d, %d, %s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %s, %.2f, %d \n", 
                  incomingData.paketno, incomingData.uydustatüsü, incomingData.hatakodu, incomingData.göndermesaati,  
                  incomingData.basınç1, incomingData.basınç2, incomingData.yükseklik1, incomingData.yükseklik2, 
                  incomingData.irtifafarkı, incomingData.inişhızı, incomingData.sıcaklık, incomingData.pilgerilimi, 
                  incomingData.pitch, incomingData.roll, incomingData.yaw, incomingData.rhrh, incomingData.iotdata, incomingData.takımno);
  } else {
    // Serial.println("Veri boyutu uyuşmazlığı!");
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}

void setup() {
  // Seri haberleşmeyi başlat
  Serial.begin(115200);

  // DHT sensörünü başlat
  dht.begin();

  // Wi-Fi modülünü başlat
  WiFi.mode(WIFI_STA);



  // ESP-NOW'u başlat
  if (esp_now_init() != ESP_OK) {
    return;
  }

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));           // Yapıyı sıfırla
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);  // Yer istasyonunun MAC adresi
  peerInfo.channel = 0;                             // Varsayılan kanal
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    return;
  }

  // Callback fonksiyonlarını ayarla
  esp_now_register_recv_cb(OnDataRecv);  // Görev yükünden veri almak için
  esp_now_register_send_cb(OnDataSent);  // Veri göndermek için

}

void loop() {
  // DHT11 sensöründen sıcaklık verisini oku
  incomingData.iotdata = dht.readTemperature();

  // ESP-NOW ile verileri gönder
  esp_now_send(broadcastAddress, (uint8_t *)&incomingData, sizeof(incomingData));

  // 1 saniye bekle
  delay(1000);
}
