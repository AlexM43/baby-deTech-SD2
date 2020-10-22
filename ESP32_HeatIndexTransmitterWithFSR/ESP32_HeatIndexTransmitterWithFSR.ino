#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"
#include <sstream>

bool deviceConnected = 0;
#define ADC_PIN A0 //FSR pin, change as needed
int ADC_VALUE = 0;
#define VMEASURE_PIN 39//Pin used to measure input voltage for battery life considerations
int VMEASURE = 0;
#define SLEEP_TIME 10
#define SERVICE_UUID        "37fc19ab-98ca-4544-a68b-d183da78acdc"
#define DHTPIN 4 //DHT22 pin change this to whatever pin you have connected
#define DHTTYPE DHT22
#define POWERPIN 13

DHT dht(DHTPIN, DHTTYPE);
BLECharacteristic heatIndex(BLEUUID((uint16_t)0x2A7A), BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic batteryLevel(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic temperatureCharacteristic(BLEUUID((uint16_t)0x2A1F), BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic humidityCharacteristic(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
int heatIndexDefault = 80;
void goToLightSleep(){//not currently used
  Serial.println("Going to light sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME*1000000);
  esp_light_sleep_start();
}
void goToDeepSleep(){
  Serial.print("Going to deep sleep for ");
  Serial.print(SLEEP_TIME);
  Serial.println(" seconds");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME*1000000);
  //disable peripherals
  digitalWrite(POWERPIN, LOW);
  esp_deep_sleep_start();
}
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {

};//not currently used
class MyServerCallbacks: public BLEServerCallbacks{
    void onConnect(BLEServer *pServer){//function called upon successful client connect
    deviceConnected = 1;
  }
  void onDisconnect(BLEServer *pServer){
    deviceConnected = 0;
  }
};
void setup() {
  //initialize serial monitor
  Serial.begin(115200);
  pinMode(POWERPIN, OUTPUT);
  //Turn peripherals back on
  digitalWrite(POWERPIN, HIGH);
  //Check FSR to see if baby is present, if not, go to deepsleep for 10 seconds
  ADC_VALUE=analogRead(ADC_PIN);
  Serial.print("FSR VALUE = ");
  Serial.println(ADC_VALUE);
  if(ADC_VALUE<50)
    goToDeepSleep();
  // Create the BLE Device
  BLEDevice::init("baby deTech");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Create the BLE Service and set Callbacks
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pServer->setCallbacks(new MyServerCallbacks());

  // Create a BLE Characteristic and set Callbacks
  pService->addCharacteristic(&heatIndex);
  heatIndex.setCallbacks(new MyCharacteristicCallbacks());
  heatIndex.addDescriptor(new BLE2902()); //This enables notifications on the client side
  
  // Create a BLE Characteristic and set Callbacks
  pService->addCharacteristic(&batteryLevel);
  batteryLevel.setCallbacks(new MyCharacteristicCallbacks());
  batteryLevel.addDescriptor(new BLE2902()); //This enables notifications on the client side

   // Create a BLE Characteristic and set Callbacks
  pService->addCharacteristic(&temperatureCharacteristic);
  temperatureCharacteristic.setCallbacks(new MyCharacteristicCallbacks());
  temperatureCharacteristic.addDescriptor(new BLE2902()); //This enables notifications on the client side

   // Create a BLE Characteristic and set Callbacks
  pService->addCharacteristic(&humidityCharacteristic);
  humidityCharacteristic.setCallbacks(new MyCharacteristicCallbacks());
  humidityCharacteristic.addDescriptor(new BLE2902()); //This enables notifications on the client side

  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);

  // Start the service
  pService->start();


  // Set advertisement rate and Start advertising
  pServer->getAdvertising()->setMinInterval(0x100);
  pServer->getAdvertising()->setMaxInterval(0x100);
  pServer->getAdvertising()->start();
  //sets default heat index
  heatIndex.setValue(heatIndexDefault);

  //Set connection parameters
  BLEAddress myAddress = BLEDevice::getAddress();
  //device address, min connection Interval*1.25ms max connection Interval*1.25ms, latency, timeout*10ms
  pServer->updateConnParams(*myAddress.getNative(),0x1000,0x1000,0,1000);
  //begin dht temperature sensor
  dht.begin();
}

void loop() {
  //Read voltage level of VIN/USB pin
  VMEASURE=analogRead(VMEASURE_PIN);
  double InputVoltage=(double)VMEASURE/3200.0*2.0*2.7+0.1;
  Serial.print("Input voltage in volts = ");
  Serial.println(InputVoltage);
  Serial.print("Raw input voltage = ");
  Serial.println(VMEASURE);
  if(VMEASURE<2300)
    Serial.println("Battery life is low.");
  //Check FSR to see if baby is present, if not, go to deepsleep for 10 seconds
  ADC_VALUE=analogRead(ADC_PIN);
  Serial.print("FSR VALUE = ");
  Serial.println(ADC_VALUE);
  if(ADC_VALUE<50)//Change this value(0-4095) to adjust pressure pad sensitivity. Lower value means more sensitive
    goToDeepSleep();
  //Notify battery level, only two states, 100 if battery level is good-okay or 0 if battery life is low
  int batteryLife;
  if(VMEASURE<2300)
    batteryLife=0;
  else
    batteryLife=100;
  std::stringstream ss;
  ss<<batteryLife;  
  batteryLevel.setValue(ss.str());
  batteryLevel.notify();
  //Read temp and humidity, calculate heat index, and notify phone
  float humidity = dht.readHumidity();
  float temperatureC = dht.readTemperature();
  if (isnan(humidity) || isnan(temperatureC)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else{
    float heatIndexVal = dht.computeHeatIndex(temperatureC,humidity, 0);
    int heatIndexInt = (int)(heatIndexVal);
    ss.str("");
    ss<<heatIndexInt;
    heatIndex.setValue(ss.str());
    heatIndex.notify();
    Serial.print("Heat index in C is: ");
    Serial.println(ss.str().c_str());
    ss.str("");
    ss<<temperatureC;
    temperatureCharacteristic.setValue(ss.str());
    temperatureCharacteristic.notify();
    Serial.print("Temperature in C is: ");
    Serial.println(ss.str().c_str());
    ss.str("");
    ss<<humidity;
    humidityCharacteristic.setValue(ss.str());
    humidityCharacteristic.notify();
    Serial.print("Humidity is: ");
    Serial.println(ss.str().c_str());
  }
  delay(1000);
  
}
