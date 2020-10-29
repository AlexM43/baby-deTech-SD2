Device notifies phone of heat index every second, but only if the FSR detects pressure.
DHT22 must be hooked up to pin 4 by default, A5 on the Huzzah feather
FSR is hooked up to 32 by default you can change this to another pin on ADC1 if needed.
You can hook up both the DHT22 and FSR to pin 13 instead of the 3.3v pin. This saves power during sleep mode since I can shut the power off by driving the pin low.
Check https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/ for info how to hook up DHT22/configure DHT22 libraries.
Check https://www.makerguides.com/fsr-arduino-tutorial/ for info to hook up the FSR, I'm using a 4K pull down resistor.
Use with nrfConnect app, must connect to device then look under "Unknown Service"
Set baud rate to 115200 in the Serial Monitor 
Fixed the "Failed to read temperature error" that kept occurring
Also need to implement sleep modes during connection, might not be possible with Arduino IDE, may need to use ESP-IDF
To measure battery level and get low battery warnings, connect a 1/2 voltage divider between VIN/USB and ground with the midpoint between the two resistors connected to pin 13