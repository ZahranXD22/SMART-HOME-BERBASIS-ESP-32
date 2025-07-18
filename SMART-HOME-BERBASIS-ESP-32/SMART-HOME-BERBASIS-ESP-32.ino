#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ----- WiFi Credentials -----
//const char* ssid = "RED (Rebahan Every Day)";
//const char* password = "TungtungtungSahur";
const char* ssid = "TER (Turu Every Day)";
const char* password = "TraBisa*22";

// ----- Pins configuration -----
// Relay pins
const int relayLedPins[4] = {16, 17, 18, 19};  // 4 relays for LEDs
const int relayFanPin = 21;                    // 1 relay for AC

// LEDs pins (optional, if direct led control separate from relay)
const int ledPins[4] = {4, 5, 12, 13};         // LEDs for manual indication (optional)

// Buzzer pin
const int buzzerPin = 22;

// Servo (door lock) pin
const int servoPin = 23;
Servo doorServo;

// DHT11 sensor pin and type
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Webserver
WebServer server(80);

// States to track
bool relayLedStates[4] = {false, false, false, false};
bool relayFanState = false;
bool doorLocked = true;  // door initially locked

// Timing for buzzer beep
const int buzzerBeepDuration = 100; // ms

// Function to beep buzzer
void beepBuzzer() {
  digitalWrite(buzzerPin, HIGH);
  delay(buzzerBeepDuration);
  digitalWrite(buzzerPin, LOW);
}

// HTML web page for control with toggle switches
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Smart Home Simulation</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; background: #121212; color: #e0e0e0; margin: 0; padding: 20px; }
    .header { display: flex; align-items: center; margin-bottom: 20px; }
    .logo-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      background: #1a237e;
      color: white;
      padding: 8px 12px;
      border-radius: 8px;
      margin-right: 15px;
      min-width: 60px;
      height: 60px;
      text-align: center;
      box-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    .logo-main {
      font-weight: bold;
      font-size: 14px;
      line-height: 1.2;
      text-transform: uppercase;
    }
    .logo-sub {
      font-size: 10px;
      margin-top: 3px;
    }
    .title-container { display: flex; flex-direction: column; }
    h1 { color: #00bfa5; margin: 0; }
    .subtitle { font-size: 12px; color: #888; margin-top: 5px; }
    .section { background: #1e1e1e; padding: 15px; margin-bottom: 20px; border-radius: 10px; }
    .status { margin-top: 10px; font-size: 18px; }
    .sensor { font-style: italic; }
    .grid-container {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 15px;
    }
    .grid-item {
      background: #252525;
      padding: 12px;
      border-radius: 8px;
    }

    /* Toggle switch styling */
    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
      margin: 5px;
    }
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .slider {
      position: absolute;
      cursor: pointer;
      background-color: #d32f2f;
      border-radius: 34px;
      top: 0; left: 0; right: 0; bottom: 0;
      transition: 0.4s;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      border-radius: 50%;
      transition: 0.4s;
    }
    input:checked + .slider {
      background-color: #00bfa5;
    }
    input:checked + .slider:before {
      transform: translateX(26px);
    }

    /* Button styling */
    .btn {
      background-color: #00bfa5;
      border: none;
      color: white;
      padding: 12px 24px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 10px 5px;
      cursor: pointer;
      border-radius: 8px;
      transition: background-color 0.3s;
    }
    .btn:hover {
      background-color: #008c7a;
    }

    /* Label text next to switch */
    .label-text {
      display: inline-block;
      vertical-align: middle;
      margin-left: 8px;
      font-size: 16px;
    }
  </style>
</head>
<body>
  <div class="header">
    <div class="logo-container">
      <div class="logo-main">SMAN 5</div>
      <div class="logo-sub">Parepare</div>
    </div>
    <div class="title-container">
      <h1>ESP32 Smart Home Simulation</h1>
      <div class="subtitle">by: Muhammad Zahran Farzanah</div>
    </div>
  </div>

  <div class="section">
    <h2>Lighting Control</h2>
    <div class="grid-container">
      <div class="grid-item">
        <label class="switch">
          <input type="checkbox" id="led1" onchange="toggleDevice('led1')">
          <span class="slider"></span>
        </label>
        <span class="label-text" id="led1Label">Lampu Kamar</span>
      </div>
      
      <div class="grid-item">
        <label class="switch">
          <input type="checkbox" id="led2" onchange="toggleDevice('led2')">
          <span class="slider"></span>
        </label>
        <span class="label-text" id="led2Label">Lampu Ruang Tamu</span>
      </div>
      
      <div class="grid-item">
        <label class="switch">
          <input type="checkbox" id="led3" onchange="toggleDevice('led3')">
          <span class="slider"></span>
        </label>
        <span class="label-text" id="led3Label">Lampu Teras</span>
      </div>
      
      <div class="grid-item">
        <label class="switch">
          <input type="checkbox" id="led4" onchange="toggleDevice('led4')">
          <span class="slider"></span>
        </label>
        <span class="label-text" id="led4Label">Lampu Ruang Keluarga</span>
      </div>
    </div>
  </div>

  <div class="section">
    <h2>Fan Control</h2>
    <label class="switch">
      <input type="checkbox" id="fan" onchange="toggleDevice('fan')">
      <span class="slider"></span>
    </label>
    <span class="label-text" id="fanLabel">AC</span>
  </div>

  <div class="section">
    <h2>Door Lock (Servo)</h2>
    <button class="btn" onclick="openDoor()">Buka Pintu</button>
    <div id="doorStatus" class="status"></div>
    <p class="sensor">Click button to open door for 5 seconds</p>
  </div>

  <div class="section">
    <h2>Room Conditions (DHT11)</h2>
    <div>Temperature: <span id="temperature">--</span> &deg;C</div>
    <div>Humidity: <span id="humidity">--</span> %</div>
  </div>

<script>
  // Fetch device states periodically
  setInterval(() => {
    fetch('/status').then(response => response.json()).then(data => {
      updateToggles(data);
      document.getElementById('temperature').textContent = data.temperature.toFixed(1);
      document.getElementById('humidity').textContent = data.humidity.toFixed(1);
      document.getElementById('doorStatus').textContent = data.doorLocked ? 'Locked' : 'Unlocked';
    });
  }, 2000);

  function updateToggles(data) {
    for(let i=0; i<4; i++) {
      let cb = document.getElementById('led'+(i+1));
      cb.checked = data.ledStates[i];
    }
    let fanCb = document.getElementById('fan');
    fanCb.checked = data.fanState;
  }

  function toggleDevice(device) {
    fetch('/toggle?device=' + device)
      .then(response => response.text())
      .then(data => console.log(data))
      .catch(e => console.error(e));
  }

  function openDoor() {
    fetch('/openDoor')
      .then(response => response.text())
      .then(data => {
        console.log(data);
        document.getElementById('doorStatus').textContent = 'Unlocked (auto-locking in 5s)';
      })
      .catch(e => console.error(e));
  }
</script>
</body>
</html>
)rawliteral";

// Function prototypes
void handleRoot();
void handleToggle();
void handleStatus();
void handleOpenDoor();
void updateDoorServo();

// Setup
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize pins
  for(int i=0; i<4; i++) {
    pinMode(relayLedPins[i], OUTPUT);
    digitalWrite(relayLedPins[i], LOW);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  pinMode(relayFanPin, OUTPUT);
  digitalWrite(relayFanPin, LOW);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Setup servo
  doorServo.attach(servoPin);
  // Lock door initially
  doorServo.write(0);  // 0deg locked
  doorLocked = true;

  // Initialize DHT sensor
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  server.on("/openDoor", handleOpenDoor);
  server.begin();
  Serial.println("Webserver started");
}

void loop() {
  server.handleClient();
}

// Handle root page
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

// Handle toggle commands for devices
void handleToggle() {
  if (!server.hasArg("device")) {
    server.send(400, "text/plain", "Device not specified");
    return;
  }
  String device = server.arg("device");
  Serial.println("Toggle request: " + device);
  if(device.startsWith("led")) {
    int ledIndex = device.substring(3).toInt() - 1;
    if(ledIndex >=0 && ledIndex <4) {
      relayLedStates[ledIndex] = !relayLedStates[ledIndex];
      digitalWrite(relayLedPins[ledIndex], relayLedStates[ledIndex] ? HIGH : LOW);
      digitalWrite(ledPins[ledIndex], relayLedStates[ledIndex] ? HIGH : LOW); // led on/off indicator
      beepBuzzer();
      server.send(200, "text/plain", "LED "+String(ledIndex+1)+" toggled");
      return;
    }
  } else if(device == "fan") {
    relayFanState = !relayFanState;
    digitalWrite(relayFanPin, relayFanState ? HIGH : LOW);
    beepBuzzer();
    server.send(200, "text/plain", "AC toggled");
    return;
  }

  server.send(400, "text/plain", "Invalid device");
}

// Handle door open request
void handleOpenDoor() {
  doorLocked = false;
  updateDoorServo();
  beepBuzzer();
  server.send(200, "text/plain", "Door unlocked for 5 seconds");
  
  // Auto-lock after 5 seconds
  delay(5000);
  doorLocked = true;
  updateDoorServo();
}

// Return JSON with current device states and sensor data
void handleStatus() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  String json = "{";
  json += "\"ledStates\":[";
  for(int i=0; i<4; i++) {
    json += (relayLedStates[i] ? "true" : "false");
    if(i<3) json += ",";
  }
  json += "],";
  json += "\"fanState\":" + String(relayFanState ? "true" : "false") + ",";
  json += "\"doorLocked\":" + String(doorLocked ? "true" : "false") + ",";
  json += "\"temperature\":" + (isnan(temperature) ? "null" : String(temperature)) + ",";
  json += "\"humidity\":" + (isnan(humidity) ? "null" : String(humidity));
  json += "}";

  server.send(200, "application/json", json);
}

// Update servo position based on doorLocked state
void updateDoorServo() {
  if(doorLocked) {
    doorServo.write(0);     // Locked position
  } else {
    doorServo.write(100);    // Unlocked position (door open)
  }
}