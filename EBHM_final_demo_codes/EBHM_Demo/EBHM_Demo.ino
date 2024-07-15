#include <ESP8266WiFi.h>
#include <WiFiClient.h>
using namespace std;
#include <String.h>
// Define pins (replace with actual PWM pin for your board)
#define PWM_PIN D5
#define ANALOG_INPUT A0
#define STATUS_LED D4 // Optional LED for status indication

// Define constants for PWM control
const int PWM_FREQUENCY = 6000; // Frequency in Hz
const int PWM_RESOLUTION = 1024;  // Range of PWM values (0-1023)

// Wi-Fi credentials (replace with your details)
const char* ssid = "EBHM_Demo";
const char* password = "12345678";

// Control variables for ESC (modify based on your setup)
bool reversibleESC = true; // Set to false if ESC only supports forward/break
int dutyCycle = 0;      // Duty cycle for PWM signal (0-PWM_RESOLUTION)

// Variables for feedback loop
float Kp = 0.08;  // Proportional gain for feedback loop
int error = 0;
int feedbackValue = 0;
int previousValue = 0; // Stores previous new_val for smooth transitions
int pwmValue = 0;
WiFiServer server(80);

void setupWifi() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password); 
  server.begin();
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupPins() {
  pinMode(PWM_PIN, OUTPUT);
  pinMode(ANALOG_INPUT, INPUT);

  // Optional: Setup LED for status indication
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW); // Turn off LED initially
}

void setupPWM() {
  analogWriteFreq(PWM_FREQUENCY);
  analogWriteRange(PWM_RESOLUTION);
}

int calculateDutyCycle(int error) {
  // Calculate new duty cycle based on error and proportional gain
  int newDutyCycle = int(dutyCycle + (Kp * error));

  // Limit duty cycle within valid range
  newDutyCycle = constrain(newDutyCycle, 0, PWM_RESOLUTION);

  // // Ensure smooth transitions by considering previous value
  // int change = newDutyCycle - previousValue;
  // if (abs(change) > 5) { // Adjust threshold for smoother transitions
  //   newDutyCycle = previousValue + (change > 0 ? 5 : -5);
  // }

  // previousValue = newDutyCycle;
  return newDutyCycle;
}

void controlESC() {
  do{
  feedbackValue = analogRead(ANALOG_INPUT);
  // Serial.println("Feedback value =");
  // Serial.println(feedbackValue);
  // Serial.println("Received value =");
  // Serial.println(pwmValue);
  error = pwmValue - feedbackValue;

  dutyCycle = calculateDutyCycle(error);
  analogWrite(PWM_PIN, dutyCycle);}
  while(abs(error)>=10);

  // // Optional: Use LED for visual feedback (adjust logic as needed)
  // digitalWrite(STATUS_LED, dutyCycle > PWM_RESOLUTION / 2 ? HIGH : LOW);
}

void rampTest() {
  if (!reversibleESC) {
    Serial.println("Non-reversible ESC detected, skipping ramp test.");
    return;
  }

  Serial.println("Starting ramp test...");
  while (dutyCycle < PWM_RESOLUTION) {
    controlESC();
    delay(10); // Adjust delay for desired ramp speed
  }
  Serial.println("Reached full speed.");
}



// Ensure these functions are defined only once
void setup() {
  setupWifi();
  setupPins();
  setupPWM();
}

void loop() {
  // Perform Wi-Fi connection check or other background tasks (optional)
  String all_command = "";

  WiFiClient client = server.available();

  if (client) {
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\r') {
          // End of line reached, check if next character is newline

          Serial.println(request);  // full HTTP command line including GET  and HTTP 1

          // Extract command from request string
          int start = request.indexOf("GET /") + 5;
          int end = request.indexOf("HTTP/");
          String command = request.substring(start, end);

          //Purify the command
          command.replace("\n", "");
          command.replace("\r", "");
          command.replace(" ", ""); // removes all space characters
          command.replace("\t", ""); // removes all tab characters
          command.trim();
          Serial.println(command);

          // Serial.println(command);  

          // all_command =  command + " is on";  // green is on 

          // Check if the command is a voltage value
          if (command.startsWith("voltage?value=")) {
            // Extract the voltage value from the command string
            float voltageValue = command.substring(14).toFloat(); // Assuming "voltage=" is 8 characters long
            Serial.println(voltageValue);
            // Scale the voltage value to PWM range (0 to 1023)
            pwmValue = int(voltageValue * 1024/3.3);
            dutyCycle = pwmValue;

  controlESC();
  Serial.println("Done");
  delay(50); // Adjust delay for loop frequency
}
        }}}}}