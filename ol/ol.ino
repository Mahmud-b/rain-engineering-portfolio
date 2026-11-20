
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// ESP RX = D7, TX = D2
SoftwareSerial uno(D7, D2);

ESP8266WebServer server(80);

const char* ssid = "King Bicep";
const char* password = "        ";

float distanceCm = -1;
int waterLevel = 0;
int pumpState = 0;

bool resetAck = false;
bool configured = false;
bool configurationPending = false;
bool dataReceived = false;

unsigned long lastUpdate = 0;
unsigned long lastResetSend = 0;
unsigned long lastHeightSend = 0;

float pendingHeight = 0;

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SmartTank Dashboard</title>

<style>
body {
  margin: 0;
  padding: 20px;
  font-family: Arial, sans-serif;
  background: #101827;
  color: white;
  text-align: center;
}

h1 { color: #38bdf8; }

.card {
  background: #1e293b;
  padding: 22px;
  margin: 15px auto;
  max-width: 420px;
  border-radius: 16px;
}

.value {
  font-size: 38px;
  font-weight: bold;
}

.tank {
  width: 130px;
  height: 230px;
  border: 5px solid #94a3b8;
  border-radius: 12px;
  margin: 20px auto;
  position: relative;
  overflow: hidden;
  background: #334155;
}

.water {
  position: absolute;
  bottom: 0;
  width: 100%;
  height: 0%;
  background: #0ea5e9;
  transition: height 0.7s, background 0.7s;
}

.status {
  font-size: 19px;
  font-weight: bold;
}

.small {
  color: #94a3b8;
  font-size: 13px;
}

input {
  width: 85%;
  padding: 13px;
  margin: 12px 0;
  border: none;
  border-radius: 8px;
  font-size: 18px;
  text-align: center;
}

button {
  background: #0284c7;
  color: white;
  border: none;
  border-radius: 8px;
  padding: 13px 22px;
  font-size: 16px;
  cursor: pointer;
}

#dashboard, #setup {
  display: none;
}
</style>
</head>

<body>

<h1>SMARTTANK</h1>
<p class="small">IoT Water Monitoring System</p>

<div class="card" id="setup">
  <h2>Tank Configuration</h2>
  <p>Enter the distance from the ultrasonic sensor face to the bottom of the tank.</p>

  <input
    type="number"
    id="tankHeight"
    min="5"
    max="200"
    step="0.1"
    placeholder="Tank height in cm"
  >

  <br>
  <button onclick="saveTankHeight()">Save Tank Height</button>

  <p id="setupMessage" class="small">
    Waiting for Arduino connection...
  </p>
</div>

<div id="dashboard">

  <div class="card">
    <h3>Water Level</h3>

    <div class="tank">
      <div class="water" id="water"></div>
    </div>

    <div class="value" id="level">--%</div>
    <p id="condition" class="status">Connecting...</p>
  </div>

  <div class="card">
    <h3>Sensor Distance</h3>
    <div class="value" id="distance">-- cm</div>
  </div>

  <div class="card">
    <h3>Water Pump</h3>
    <div class="value" id="pump">--</div>
    <p id="connection" class="small">Waiting for data...</p>
  </div>

</div>

<script>
async function saveTankHeight() {
  const height = document.getElementById('tankHeight').value;
  const message = document.getElementById('setupMessage');

  if (!height || Number(height) < 5 || Number(height) > 200) {
    message.innerText = 'Enter a height between 5 and 200 cm.';
    return;
  }

  message.innerText = 'Sending tank configuration...';

  try {
    const response = await fetch('/configure', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      body: 'height=' + encodeURIComponent(height)
    });

    const result = await response.json();
    message.innerText = result.message;

  } catch (error) {
    message.innerText = 'Could not send configuration.';
  }
}

async function updateDashboard() {
  try {
    const response = await fetch('/data');
    const data = await response.json();

    const setup = document.getElementById('setup');
    const dashboard = document.getElementById('dashboard');
    const message = document.getElementById('setupMessage');

    if (!data.configured) {
      setup.style.display = 'block';
      dashboard.style.display = 'none';

      if (!data.resetReady) {
        message.innerText = 'Connecting to Arduino...';
      } else if (data.pending) {
        message.innerText = 'Waiting for Arduino to accept tank height...';
      } else {
        message.innerText = 'Please enter the tank height to begin.';
      }

      return;
    }

    setup.style.display = 'none';
    dashboard.style.display = 'block';

    if (!data.online) {
      document.getElementById('connection').innerText =
        'Waiting for Arduino data...';
      return;
    }

    const level = data.level;

    document.getElementById('level').innerText =
      level + '%';

    document.getElementById('distance').innerText =
      data.distance.toFixed(1) + ' cm';

    document.getElementById('pump').innerText =
      data.pump ? 'RUNNING' : 'OFF';

    document.getElementById('connection').innerText =
      'System online';

    const water = document.getElementById('water');
    water.style.height = level + '%';

    let color;
    let condition;

    if (level <= 40) {
      color = '#ef4444';
      condition = 'LOW WATER LEVEL';
    }
    else if (level >= 91) {
      color = '#22c55e';
      condition = 'TANK FULL';
    }
    else {
      color = '#0ea5e9';
      condition = 'WATER LEVEL NORMAL';
    }

    water.style.background = color;
    document.getElementById('condition').innerText = condition;

  } catch (error) {
    document.getElementById('setupMessage').innerText =
      'Dashboard connection error';
  }
}

setInterval(updateDashboard, 1000);
updateDashboard();
</script>

</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(9600);
  uno.begin(9600);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("Dashboard IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send_P(200, "text/html", PAGE);
  });

  server.on("/configure", HTTP_POST, []() {
    if (!resetAck) {
      server.send(409, "application/json",
                  "{\"message\":\"Waiting for Arduino connection.\"}");
      return;
    }

    if (!server.hasArg("height")) {
      server.send(400, "application/json",
                  "{\"message\":\"Tank height is missing.\"}");
      return;
    }

    float height = server.arg("height").toFloat();

    if (height < 5.0 || height > 200.0) {
      server.send(400, "application/json",
                  "{\"message\":\"Height must be between 5 and 200 cm.\"}");
      return;
    }

    pendingHeight = height;
    configurationPending = true;
    configured = false;
    lastHeightSend = 0;

    uno.print("H,");
    uno.println(pendingHeight, 1);
    lastHeightSend = millis();

    server.send(200, "application/json",
                "{\"message\":\"Tank height sent. Waiting for confirmation...\"}");
  });

  server.on("/data", []() {
    bool online =
      configured &&
      dataReceived &&
      (millis() - lastUpdate < 5000) &&
      distanceCm >= 0;

    String json = "{";

    json += "\"distance\":" + String(distanceCm, 1) + ",";
    json += "\"level\":" + String(waterLevel) + ",";
    json += "\"pump\":" + String(pumpState) + ",";
    json += "\"online\":" + String(online ? "true" : "false") + ",";
    json += "\"configured\":" + String(configured ? "true" : "false") + ",";
    json += "\"pending\":" + String(configurationPending ? "true" : "false") + ",";
    json += "\"resetReady\":" + String(resetAck ? "true" : "false");

    json += "}";

    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  // Ask the Arduino to reset its tank configuration after ESP startup
  if (!resetAck && millis() - lastResetSend >= 1000) {
    uno.println("R");
    lastResetSend = millis();
  }

  // Retry tank height until the Arduino confirms it
  if (configurationPending &&
      millis() - lastHeightSend >= 1000) {

    uno.print("H,");
    uno.println(pendingHeight, 1);

    lastHeightSend = millis();
  }

  // Receive Arduino messages
  static String line = "";

  while (uno.available()) {
    char c = uno.read();

    if (c == '\n') {
      line.trim();

      if (line == "ACK,RESET") {
        resetAck = true;
        configured = false;
        configurationPending = false;
        dataReceived = false;

        Serial.println("Arduino reset acknowledged");
      }

      else if (line == "ACK,HEIGHT") {
        configured = true;
        configurationPending = false;

        Serial.println("Tank height accepted");
      }

      else {
        float newDistance;
        int newLevel;
        int newPump;
        int newConfigured;

        if (sscanf(line.c_str(), "%f,%d,%d,%d",
                   &newDistance, &newLevel,
                   &newPump, &newConfigured) == 4) {

          if (newConfigured == 1 && resetAck) {
            distanceCm = newDistance;
            waterLevel = constrain(newLevel, 0, 100);
            pumpState = newPump;

            configured = true;
            lastUpdate = millis();
            dataReceived = true;
          }

          else if (newConfigured == 0) {
            configured = false;
            dataReceived = false;
            pumpState = 0;
          }
        }
      }

      line = "";
    }

    else if (c != '\r' && line.length() < 60) {
      line += c;
    }
  }
}
