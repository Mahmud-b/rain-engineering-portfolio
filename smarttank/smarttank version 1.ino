#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "";
const char* password = "        ";

int Trigpin = D5;
int echopin = D6;

float tankHeight = 16;

ESP8266WebServer server(80);

float getDistance() {
  digitalWrite(Trigpin, LOW);
  delayMicroseconds(2);

  digitalWrite(Trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigpin, LOW);

  long duration = pulseIn(echopin, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return 0.01715 * duration;
}

void handleRoot() {
  String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SmartTank Dashboard</title>
<style>
- {
  box-sizing: border-box;
}

body {
  font-family: Arial, sans-serif;
  background: #101827;
  color: white;
  text-align: center;
  padding: 15px;
  margin: 0;
}

.container {
  max-width: 430px;
  margin: 20px auto;
  background: #1e293b;
  padding: 25px 18px;
  border-radius: 20px;
  box-shadow: 0 10px 30px rgba(0,0,0,0.3);
}

h1 {
  color: #38bdf8;
  margin-bottom: 5px;
}

.subtitle {
  color: #94a3b8;
  margin-top: 0;
}

.tank-wrapper {
  display: flex;
  justify-content: center;
  margin: 35px 0 25px;
}

.tank {
  width: 180px;
  height: 260px;
  position: relative;
  border: 5px solid #94a3b8;
  border-top: 9px solid #cbd5e1;
  border-radius: 15px 15px 22px 22px;
  overflow: hidden;
  background: #0f172a;
  box-shadow: inset 0 0 15px rgba(0,0,0,0.5);
}

.water {
  position: absolute;
  bottom: 0;
  left: 0;
  width: 100%;
  height: 0%;
  background: #38bdf8;
  transition: height 1s ease, background 0.5s ease;
  z-index: 1;
}

.water::before {
  content: "";
  position: absolute;
  top: -12px;
  left: -50%;
  width: 200%;
  height: 25px;
  background: inherit;
  border-radius: 40%;
  opacity: 0.8;
  animation: wave 4s linear infinite;
}

.water::after {
  content: "";
  position: absolute;
  top: -8px;
  left: -50%;
  width: 200%;
  height: 22px;
  background: inherit;
  border-radius: 40%;
  opacity: 0.5;
  animation: wave 6s linear infinite reverse;
}

@keyframes wave {
  from {
    transform: translateX(0) rotate(0deg);
  }
  to {
    transform: translateX(25%) rotate(360deg);
  }
}

.marks {
  position: absolute;
  inset: 0;
  z-index: 2;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  padding: 20px 8px;
  pointer-events: none;
}

.mark {
  text-align: left;
  color: white;
  font-size: 11px;
  font-weight: bold;
  text-shadow: 1px 1px 3px black;
}

.card {
  background: #334155;
  padding: 15px;
  margin: 13px 0;
  border-radius: 12px;
}

.value {
  font-size: 30px;
  font-weight: bold;
  margin: 10px;
}

.status {
  font-size: 22px;
  font-weight: bold;
}

.low {
  color: #f87171;
}

.normal {
  color: #38bdf8;
}

.full {
  color: #4ade80;
}

.connection {
  color: #94a3b8;
  font-size: 13px;
  margin-top: 20px;
}

.footer {
  color: #64748b;
  font-size: 12px;
  margin-top: 20px;
}
</style>
</head>
<body>
<div class="container">
  <h1>SmartTank</h1>
  <p class="subtitle">Water Level Monitoring System</p>
  <div class="tank-wrapper">
    <div class="tank">
      <div class="water" id="water"></div>
      <div class="marks">
        <div class="mark">100%</div>
        <div class="mark">75%</div>
        <div class="mark">50%</div>
        <div class="mark">25%</div>
        <div class="mark">0%</div>
      </div>
    </div>
  </div>
  <div class="card">
    <p>WATER LEVEL</p>
    <div class="value" id="percentage">--%</div>
  </div>
  <div class="card">
    <p>DISTANCE TO WATER</p>
    <div class="value" id="distance">-- cm</div>
  </div>
  <div class="card">
    <p>TANK STATUS</p>
    <div class="status normal" id="status">Loading...</div>
  </div>
  <p class="connection" id="connection">Connecting to sensor...</p>
  <p class="footer">SmartTank | IoT Water Monitoring</p>
</div>
<script>
async function updateDashboard() {
  try {
    const response = await fetch('/data');
    const data = await response.json();
    if (data.status === "SENSOR ERROR") {
      document.getElementById('status').innerText = "SENSOR ERROR";
      document.getElementById('status').className = "status low";
      document.getElementById('connection').innerText = "Check sensor connection";
      return;
    }
    const percentage = Number(data.percentage);
    document.getElementById('percentage').innerText = percentage.toFixed(1) + "%";
    document.getElementById('distance').innerText = data.distance + " cm";
    const water = document.getElementById('water');
    water.style.height = percentage + "%";
    const status = document.getElementById('status');
    if (percentage < 40) {
      water.style.background = "#ef4444";
      status.innerText = "LOW";
      status.className = "status low";
    }
    else if (percentage < 80) {
      water.style.background = "#38bdf8";
      status.innerText = "NORMAL";
      status.className = "status normal";
    }
    else {
      water.style.background = "#22c55e";
      status.innerText = "HIGH / FULL";
      status.className = "status full";
    }
    document.getElementById('connection').innerText = "● Live sensor connection";
  }
  catch (error) {
    document.getElementById('connection').innerText = "Connection lost. Retrying...";
  }
}
updateDashboard();
setInterval(updateDashboard, 1000);
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", webpage);
}

void handleData() {
  float distance = getDistance();

  if (distance < 0) {
    server.send(200, "application/json", "{\"distance\":\"--\",\"percentage\":\"--\",\"status\":\"SENSOR ERROR\"}");
    return;
  }

  float percentage = ((tankHeight - distance) / tankHeight) * 100;
  percentage = constrain(percentage, 0, 100);

  String status;

  if (percentage >= 80) {
    status = "HIGH";
  }
  else if (percentage >= 40) {
    status = "MEDIUM";
  }
  else {
    status = "LOW";
  }

  String json = "{";
  json += "\"distance\":\"" + String(distance, 1) + "\",";
  json += "\"percentage\":\"" + String(percentage, 1) + "\",";
  json += "\"status\":\"" + status + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);

  pinMode(Trigpin, OUTPUT);
  pinMode(echopin, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("SmartTank server started!");
}

void loop() {
  server.handleClient();
}
