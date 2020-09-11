Device notifies phone of heat index every second.
DHT22 must be hooked up to pin 4 by default, A5 on the Huzzah Feather
Check https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/ for info how to hook up DHT22/configure DHT22 libraries.
Use with nrfConnect app, must connect to device then look under "Unknown Service"
Set baud rate to 115200 in the Serial Monitor 
Fixed the "Failed to read temperature error" that kept occurring
Also need to implement sleep modes, might not be possible with Arduino IDE, may need to use ESP-IDF
