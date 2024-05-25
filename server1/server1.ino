#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "LEDControl.h"
#include "PIDController.h"
#include <Preferences.h>

Preferences preferences; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* ssid = "TP-Link_84AB";
const char* password = "18363784";

unsigned long previousMillis = 0;
const long interval = 5000;  

ESP8266WebServer server(80);

// LED pins
LEDControl leds(5, 4, 0);  // GPIO5, GPIO4, GPIO0 for RGB

// Temperature and PID settings (replace with actual GPIO pins)
PIDController<2, 14> tempController;  // Example: GPIO2 for sensor, GPIO14 for heater

// Pump speed variable
int pumpSpeed = 0, r = 0, b = 0, g = 0;
double temp;
String ledOnTime = "00:00", ledOffTime = "00:00", currentTemp = "";

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  preferences.begin("esp-settings", false);  // Инициализация под "esp-settings" namespace
  // Загрузка сохраненных значений
  r = preferences.getInt("red", 0);         // Значение по умолчанию - 0
  g = preferences.getInt("green", 0);
  b = preferences.getInt("blue", 0);
  pumpSpeed = preferences.getInt("pumpSpeed", 0);
  temp = preferences.getDouble("temp", 25.0);  // Предположим, что начальная температура 25.0
  ledOnTime = preferences.getString("ledOnTime", "06:00");
  ledOffTime = preferences.getString("ledOffTime", "18:00");
  timeClient.begin();
  timeClient.setTimeOffset(28800); 
  
  leds.setColor(r*4, g*4, b*4);
  if (pumpSpeed >= 0 && pumpSpeed <= 100) {
  int pwmValue = map(pumpSpeed, 0, 100, 0, 1023);
  analogWrite(pumpPin, pwmValue);

  server.on("/", handleRoot);
  server.on("/setLED", handleSetLED);
  server.on("/setPumpSpeed", handleSetPumpSpeed);
  server.on("/setTemperature", handleSetTemperature);
  server.on("/setOnTime", handleSetOnTime);
  server.on("/setOffTime", handleSetOffTime);
  server.on("/getCurrentTemperature", handleGetCurrentTemperature);

  
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  // Загрузка сохраненных значений
  r = preferences.getInt("red", 0);
  g = preferences.getInt("green", 0);
  b = preferences.getInt("blue", 0);
  pumpSpeed = preferences.getInt("pumpSpeed", 0);
  temp = preferences.getDouble("temp", 25.0);
  ledOnTime = preferences.getString("ledOnTime", "06:00");
  ledOffTime = preferences.getString("ledOffTime", "18:00");

String html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Control Server</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f4f4f4;
      color: #333;
    }
    h1 { text-align: center; margin: 20px 0; }
    .container { display: flex; flex-wrap: wrap; justify-content: center; }
    .settings, .current-settings {
      background: #fff;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.1);
      margin: 10px;
    }
    .settings { flex-grow: 1; max-width: 300px; }
    .current-settings { flex-grow: 1; max-width: 300px; }
    label { display: block; margin: 15px 0; }
    input[type='range'], input[type='number'], input[type='time'] {
      width: 100%;
      margin-top: 5px;
    }
    input[type='range'] {
      -webkit-appearance: none;
      height: 15px;
      border-radius: 7.5px;
      outline: none;
      background: #ddd;
    }
    input[type='range']::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: #727272;
      cursor: pointer;
    }
    input[type='range']#red::-webkit-slider-thumb { background: red; }
    input[type='range']#green::-webkit-slider-thumb { background: green; }
    input[type='range']#blue::-webkit-slider-thumb { background: blue; }
    
    .value-display { margin-left: 10px; }
    input[type='submit'] {
      width: 100%;
      background-color: #4CAF50;
      color: white;
      padding: 10px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      margin-top: 20px;
    }
    input[type='submit']:hover { background-color: #45a049; }
    .spinner {
      border: 8px solid #f3f3f3;
      border-top: 8px solid #3498db;
      border-radius: 50%;
      width: 30px;
      height: 30px;
      animation: spin 2s linear infinite;
    }
    @keyframes spin {
      0% { transform: rotate(0deg); }
      100% { transform: rotate(360deg); }
    }
    @media (max-width: 600px) {
      .container { flex-direction: column; align-items: center; }
    }
  </style>
</head>
<body>
  <h1>ESP8266 Control Server</h1>
  <div class="container">
    <div class="settings">
      <!-- Color Control -->
      <form action="/setLED">
        <label>Red: <span id='redValue' class='value-display'></span><input type='range' name='r' min='0' max='255' id='red' oninput='updateValueDisplay("red", this.value)'></label>
        <label>Green: <span id='greenValue' class='value-display'></span><input type='range' name='g' min='0' max='255' id='green' oninput='updateValueDisplay("green", this.value)'></label>
        <label>Blue: <span id='blueValue' class='value-display'></span><input type='range' name='b' min='0' max='255' id='blue' oninput='updateValueDisplay("blue", this.value)'></label>
        <input type='submit' value='Set LED'>
      </form>
      <!-- Pump Speed Control -->
      <form action="/setPumpSpeed">
        <label>Pump Speed: <span id='pumpSpeedValue'></span><input type='range' name='speed' id='pumpSpeed' min='0' max='100' oninput='updatePumpSpeedDisplay(this.value)'></label>
        <input type='submit' value='Set Pump Speed'>
      </form>
      <!-- Temperature Setpoint Control -->
      <form action="/setTemperature">
        <label>Temperature Setpoint: <input type='number' name='temp' style='border: 1px solid #ccc; padding: 8px; border-radius: 4px;'></label>
        <input type='submit' value='Set Temperature'>
      </form>
      <!-- Time Control -->
      <form action="/setOnTime">
        <label>LED On Time: <input type='time' name='onTime'></label>
        <input type='submit' value='Set On Time'>
      </form>
      <form action="/setOffTime">
        <label>LED Off Time: <input type='time' name='offTime'></label>
        <input type='submit' value='Set Off Time'>
      </form>
    </div>

    <div class="current-settings">
        <p>Current LED Settings - Red: )=====" + String(r) + R"=====(
        , Green: )=====" + String(g) + R"=====(
        , Blue: )=====" + String(b) + R"=====(
        </p>
        <p>Current Pump Speed: )=====" + String(pumpSpeed) + R"=====(
        </p>
        <p id='currentTemp'>Loading current temperature...</p>
        <p>Current Temperature Setpoint: )=====" + String(temp) + R"=====(&deg;C</p>
        <p>Current LED On Time: )=====" + ledOnTime + R"=====(
        </p>
        <p>Current LED Off Time: )=====" + ledOffTime + R"=====(
        </p>
      </div>
  </div>
  <script>
    function updateValueDisplay(color, value) {
      document.getElementById(color + 'Value').innerText = value;
      var input = document.getElementById(color);
      var percent = (value - input.min) / (input.max - input.min) * 100;
      var colorStop = `linear-gradient(90deg, ${color} ${percent}%, #ddd ${percent}%)`;
      input.style.background = colorStop;
    }

    function updatePumpSpeedDisplay(value) {
      document.getElementById('pumpSpeedValue').innerText = value + '%';
    }
    setInterval(function(){
    fetch('/getCurrentTemperature').then(response => 
    response.text()).then(data => 
    document.getElementById('currentTemp').innerHTML = 'Current Temperature: ' + data + '&deg;C');}, 1000);

    window.onload = function() {
      updateValueDisplay('red', document.getElementById('red').value);
      updateValueDisplay('green', document.getElementById('green').value);
      updateValueDisplay('blue', document.getElementById('blue').value);
      updatePumpSpeedDisplay(document.getElementById('pumpSpeed').value);
    };
  </script>
</body>
</html>
)=====";
  server.send(200, "text/html", html);
}

void handleSetOnTime() {
  ledOnTime = server.arg("onTime");
  preferences.putString("ledOnTime", ledOnTime);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetOffTime() {
  ledOffTime = server.arg("offTime");
  preferences.putString("ledOffTime", ledOffTime);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetLED() {
  r = server.arg("r").toInt();
  g = server.arg("g").toInt();
  b = server.arg("b").toInt();
  leds.setColor(r, g, b);
  preferences.putInt("red", r);
  preferences.putInt("green", g);
  preferences.putInt("blue", b);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetPumpSpeed() {
  pumpSpeed = server.arg("speed").toInt();
  if (pumpSpeed >= 0 && pumpSpeed <= 100) {
  int pwmValue = map(pumpSpeed, 0, 100, 0, 1023);
  analogWrite(pumpPin, pwmValue);
  preferences.putInt("pumpSpeed", pumpSpeed);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetTemperature() {
  temp = server.arg("temp").toDouble();
  preferences.putDouble("temp", temp);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleGetCurrentTemperature() {
  currentTemp = tempController.readTemperature();
  server.send(200, "text/plain", currentTemp);
}
void loop() {

  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *timeinfo;
    timeinfo = localtime(&epochTime);
    String currentRealTime = String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min);
    leds.onTime(currentRealTime, ledOnTime);
    leds.offTime(currentRealTime, ledOffTime);
    tempController.heating(preferences.getDouble("temp", temp);
    /*currentTemp = tempController.readTemperature(); 
    Serial.print("RGB LED Values: R=");
    Serial.print(r);
    Serial.print(", G=");
    Serial.print(g);
    Serial.print(", B=");
    Serial.print(b);
    Serial.print("; Pump Speed: ");
    Serial.print(pumpSpeed);
    Serial.print("; ledOnTime: ");
    Serial.print(ledOnTime);
    Serial.print("; ledOffTime: ");
    Serial.print(ledOffTime);
    Serial.print("; Current Temperature: ");
    Serial.print(currentTemp);
    Serial.print("; Temperature Setpoint: ");
    Serial.println(temp);*/
  }
  server.handleClient();
}
void end() {
  preferences.end();  // Закрытие Preferences при завершении работы
}