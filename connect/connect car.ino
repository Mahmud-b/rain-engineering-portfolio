
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials
const char* ssid = "King Bicep";
const char* password = "Mubarak@4484";

ESP8266WebServer server(80);

// ESP8266 RX, TX
SoftwareSerial arduino(D5, D6);

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>

<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ROBO COMMAND</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 20px;
  background: #080e1b;
  color: white;
  font-family: Arial, sans-serif;
  text-align: center;
}

h1 {
  color: #38bdf8;
  letter-spacing: 3px;
  margin-bottom: 5px;
}

.subtitle {
  color: #94a3b8;
  font-size: 12px;
}

.panel {
  max-width: 420px;
  margin: 25px auto;
  padding: 22px;
  background: #111c30;
  border: 1px solid #263b5c;
  border-radius: 22px;
  box-shadow: 0 10px 30px #0008;
}

.connection {
  display: inline-block;
  padding: 9px 15px;
  border-radius: 20px;
  background: #263449;
  color: #facc15;
  font-size: 12px;
}

.section-title {
  color: #94a3b8;
  font-size: 12px;
  margin-top: 28px;
}

/* Joystick */

.joystick {
  width: 230px;
  height: 230px;
  margin: 25px auto;
  border-radius: 50%;
  position: relative;

  background:
    radial-gradient(circle, #263b5c 0%, #14243d 65%, #0b1527 100%);

  border: 3px solid #31547d;
  box-shadow: inset 0 0 25px #0008;
  touch-action: none;
  user-select: none;
}

.direction {
  position: absolute;
  color: #64748b;
  font-size: 15px;
  font-weight: bold;
}

.up {
  top: 12px;
  left: 50%;
  transform: translateX(-50%);
}

.down {
  bottom: 12px;
  left: 50%;
  transform: translateX(-50%);
}

.left {
  left: 12px;
  top: 50%;
  transform: translateY(-50%);
}

.right {
  right: 12px;
  top: 50%;
  transform: translateY(-50%);
}

.knob {
  width: 78px;
  height: 78px;
  border-radius: 50%;

  position: absolute;
  left: 50%;
  top: 50%;

  transform: translate(-50%, -50%);

  background: linear-gradient(145deg, #38bdf8, #1d4ed8);

  border: 3px solid #7dd3fc;
  box-shadow: 0 5px 20px #0008;

  pointer-events: none;
}

.stop {
  width: 150px;
  height: 55px;
  border: 0;
  border-radius: 15px;

  background: linear-gradient(145deg, #ef4444, #991b1b);

  color: white;
  font-size: 18px;
  font-weight: bold;

  box-shadow: 0 5px 0 #641313;
}

.stop:active {
  transform: translateY(4px);
  box-shadow: none;
}

.command {
  color: #38bdf8;
  font-size: 23px;
  font-weight: bold;
  margin-top: 10px;
}

.footer {
  color: #64748b;
  font-size: 11px;
  margin-top: 25px;
}

</style>
</head>

<body>

<h1>ROBO COMMAND</h1>

<p class="subtitle">
WIRELESS ROBOT CONTROL SYSTEM
</p>

<div class="panel">

  <div id="connection" class="connection">
    CHECKING CONNECTION...
  </div>

  <p class="section-title">
    DRAG THE JOYSTICK TO MOVE
  </p>

  <div class="joystick" id="joystick">

    <span class="direction up">FRONT</span>
    <span class="direction down">BACK</span>
    <span class="direction left">LEFT</span>
    <span class="direction right">RIGHT</span>

    <div class="knob" id="knob"></div>

  </div>

  <button class="stop" onclick="stopJoystick()">
    EMERGENCY STOP
  </button>

  <p class="section-title">
    CURRENT MOVEMENT
  </p>

  <div id="command" class="command">
    STOPPED
  </div>

  <p class="footer">
    MOVE JOYSTICK • RELEASE TO STOP
  </p>

</div>

<script>

const joystick = document.getElementById('joystick');
const knob = document.getElementById('knob');
const commandDisplay = document.getElementById('command');

let activeDirection = null;
let moveTimer = null;
let activePointer = null;

function send(command) {

  fetch('/cmd?move=' + command, {
    cache: 'no-store'
  }).catch(() => {
    document.getElementById('connection').innerText =
      'CONNECTION ERROR';
  });

  const names = {
    F: 'FRONT / FORWARD',
    B: 'BACK / REVERSE',
    L: 'LEFT',
    R: 'RIGHT',
    S: 'STOPPED'
  };

  commandDisplay.innerText = names[command];
}

function moveJoystick(event) {

  const rect = joystick.getBoundingClientRect();

  const centerX = rect.width / 2;
  const centerY = rect.height / 2;

  let x = event.clientX - rect.left - centerX;
  let y = event.clientY - rect.top - centerY;

  const maxDistance = 75;

  const distance = Math.sqrt(x*x + y*y);

  if (distance > maxDistance) {
    x = x * maxDistance / distance;
    y = y * maxDistance / distance;
  }

  knob.style.left = (centerX + x) + 'px';
  knob.style.top = (centerY + y) + 'px';

  let direction = null;

  // Small movements remain in the neutral zone.
  if (Math.sqrt(x*x + y*y) > 18) {

    if (Math.abs(x) > Math.abs(y)) {

      direction = x > 0 ? 'R' : 'L';

    } else {

      direction = y > 0 ? 'B' : 'F';

    }
  }

  if (direction !== activeDirection) {

    activeDirection = direction;

    if (direction !== null) {
      send(direction);
    } else {
      send('S');
    }
  }
}

function startJoystick(event) {

  if (activePointer !== null) return;

  event.preventDefault();

  activePointer = event.pointerId;

  joystick.setPointerCapture(event.pointerId);

  moveJoystick(event);

  clearInterval(moveTimer);

  moveTimer = setInterval(() => {

    if (activeDirection !== null) {
      send(activeDirection);
    }

  }, 180);
}

function resetJoystick() {

  knob.style.left = '50%';
  knob.style.top = '50%';

  clearInterval(moveTimer);
  moveTimer = null;

  activeDirection = null;
  activePointer = null;

  send('S');
}

function stopJoystick() {
  resetJoystick();
}

joystick.addEventListener('pointerdown', startJoystick);

joystick.addEventListener('pointermove', function(event) {

  if (event.pointerId === activePointer) {
    moveJoystick(event);
  }

});

joystick.addEventListener('pointerup', function(event) {

  if (event.pointerId === activePointer) {
    resetJoystick();
  }

});

joystick.addEventListener('pointercancel', function(event) {

  if (event.pointerId === activePointer) {
    resetJoystick();
  }

});

window.addEventListener('blur', resetJoystick);
window.addEventListener('pagehide', resetJoystick);

async function checkConnection() {

  try {

    await fetch('/ping', {
      cache: 'no-store'
    });

    document.getElementById('connection').innerText =
      '● ESP8266 CONNECTED';

    document.getElementById('connection').style.color =
      '#4ade80';

  } catch {

    document.getElementById('connection').innerText =
      '● CONNECTION LOST';

    document.getElementById('connection').style.color =
      '#f87171';

    resetJoystick();

  }
}

setInterval(checkConnection, 2000);
checkConnection();

</script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

void handleCommand() {

  if (!server.hasArg("move")) {
    server.send(400, "text/plain", "Missing command");
    return;
  }

  String command = server.arg("move");

  if (command != "F" && command != "B" &&
      command != "L" && command != "R" &&
      command != "S") {

    server.send(400, "text/plain", "Invalid command");
    return;
  }

  arduino.println(command);

  server.send(200, "text/plain", "OK");
}

void setup() {

  Serial.begin(115200);
  arduino.begin(9600);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);

  server.on("/ping", []() {
    server.send(200, "text/plain", "ONLINE");
  });

  server.begin();

  Serial.println();
  Serial.println("ROBO COMMAND READY!");
  Serial.println(WiFi.localIP());

}

void loop() {
  server.handleClient();
}
