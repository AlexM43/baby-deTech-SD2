Device notifies phone of heat index every second, but only if the FSR detects pressure.
DHT22 must be hooked up to pin 4 by default, A5 on the Huzzah feather
Check https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/ for info how to hook up DHT22/configure DHT22 libraries.
Check https://www.makerguides.com/fsr-arduino-tutorial/ for info to hook up the FSR, I'm using a 4K pull down resistor.
Use with nrfConnect app, must connect to device then look under "Unknown Service"
Set baud rate to 115200 in the Serial Monitor 
Fixed the "Failed to read temperature error" that kept occurring
Also need to implement sleep modes during connection, might not be possible with Arduino IDE, may need to use ESP-IDF
