#define DEFAULT_SERIAL_RX_PIN 20
#define DEFAULT_SERIAL_TX_PIN 21

#define DEFAULT_MODBUS_RX_PIN 6
#define DEFAULT_MODBUS_TX_PIN 7

// set TX Power only for ESP32-C3 to 8.5dBm to avoid WIFI issues
// https://forum.arduino.cc/t/no-wifi-connect-with-esp32-c3-super-mini/1324046/13
#define WIFI_TX_POWER WIFI_POWER_8_5dBm