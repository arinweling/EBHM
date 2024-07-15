

#include <ESP8266WiFi.h>

#include <LiquidCrystal_I2C.h>

const char* ssid = "EBHM_Demo";     // SSID of the access point
const char* password = "12345678";   // Password of the access point
const char* host = "192.168.4.1";    // IP address of NodeMCU B
// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 columns and 2 rows

void setup() {
  Serial.begin(115200);
  
  
  lcd.init();                      // Initialize LCD
  lcd.backlight();                 // Turn on backlight
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read analog value from pin A0
  int analogValue = analogRead(A0);
  float value_sent = analogValue * (3.3 /1024.0);
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println(value_sent);
  lcd.print("Analog Value:");
  lcd.setCursor(0, 1);
  lcd.print(value_sent);

  // Create the URL with the analog value as a parameter
  String url = "/voltage?value=" + String(value_sent);

  // Send HTTP GET request to NodeMCU B
  WiFiClient client;
  if (client.connect(host, 80)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    client.stop();
  } else {
    Serial.println("Connection failed");
  }

  // Wait for a while before sending the next request    
  delay(500);  // Adjust delay as needed
}
