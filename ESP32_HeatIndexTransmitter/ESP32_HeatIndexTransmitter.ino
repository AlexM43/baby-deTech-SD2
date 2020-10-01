#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"
#include <sstream>

BLECharacteristic *pCharacteristic;

bool deviceConnected = 0;

#define SLEEP_TIME 1
#define SERVICE_UUID        "37fc19ab-98ca-4544-a68b-d183da78acdc"
#define DHTPIN 4 //May need to change this to whatever pin you have connected
#define DHTTYPE DHT22
#define CHARACTERISTIC_UUID "2A7A"
DHT dht(DHTPIN, DHTTYPE);
//BLECharacteristic heatIndex(BLEUUID(, BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic *heatIndex;
int heatIndexDefault = 80;
void goToLightSleep(){//not currently used
  Serial.println("Going to sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME*1000000);
  esp_light_sleep_start();
}
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {

};
class MyServerCallbacks: public BLEServerCallbacks{
    void onConnect(BLEServer *pServer){//function called upon successful client connect
    deviceConnected = 1;
  }
  void onDisconnect(BLEServer *pServer){
    deviceConnected = 0;
  }
};
void setup() {
  Serial.begin(115200);
  // Create the BLE Device
  BLEDevice::init("baby deTech");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Create the BLE Service and set Callbacks
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pServer->setCallbacks(new MyServerCallbacks());

  // Create a BLE Characteristic and set Callbacks
  heatIndex = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pService->addCharacteristic(heatIndex);
  heatIndex->setCallbacks(new MyCharacteristicCallbacks());
  heatIndex->addDescriptor(new BLE2902()); //This enables notifications on the client side

  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);

  // Start the service
  pService->start();


  // Set advertisement rate and Start advertising
  pServer->getAdvertising()->setMinInterval(0x100);
  pServer->getAdvertising()->setMaxInterval(0x100);
  pServer->getAdvertising()->start();
  //sets default heat index
  heatIndex->setValue(heatIndexDefault);

  //Set connection parameters
  BLEAddress myAddress = BLEDevice::getAddress();
  //device address, min connection Interval*1.25ms max connection Interval*1.25ms, latency, timeout*10ms
  pServer->updateConnParams(*myAddress.getNative(),0x1000,0x1000,0,1000);
  //begin dht temperature sensor
  
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperatureC = dht.readTemperature();
  if (isnan(humidity) || isnan(temperatureC)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else{
    float heatIndexVal = dht.computeHeatIndex(temperatureC,humidity, 0);
    int heatIndexInt = (int)(temperatureC);
    std::stringstream ss;
    ss<<heatIndexInt;
    heatIndex->setValue(ss.str());
    heatIndex->notify();
    Serial.print("Heat index in C is: ");
    Serial.println(heatIndexInt);
  }
  delay(1000);
  
}
