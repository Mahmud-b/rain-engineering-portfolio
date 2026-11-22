#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials (unchanged)
const char* ssid = "King Bicep";
const char* password = "        ";

// Arduino communication (D5 = RX, D6 = TX, 9600 baud)
SoftwareSerial arduinoSerial(D5, D6);

// Web and WebSocket servers (ports 80 and 81)
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

// Controller values from Arduino (steering,throttle,maxSpeed)
int steering = 512;
int throttle = 512;
int maxSpeed = 0;

String inputData = "";

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CITY DRIVE 3D — FUTA CAMPUS</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  overflow: hidden;
  background: #0b1320;
  color: #f8fafc;
  font-family: "Segoe UI", system-ui, -apple-system, Arial, sans-serif;
  user-select: none;
}

#game {
  position: fixed;
  inset: 0;
}

canvas {
  display: block;
}

.hud-top {
  position: fixed;
  z-index: 2;
  top: 18px;
  left: 18px;
  right: 18px;
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 16px;
  pointer-events: none;
}

.panel {
  background: rgba(9, 16, 29, 0.78);
  border: 1px solid rgba(148, 163, 184, 0.22);
  border-radius: 16px;
  padding: 14px 18px;
  backdrop-filter: blur(10px);
  box-shadow: 0 12px 30px rgba(0, 0, 0, 0.35);
}

.brand {
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 2.2px;
  color: #38bdf8;
  text-transform: uppercase;
}

.road-label {
  margin-top: 5px;
  font-size: 13px;
  font-weight: 700;
  color: #f1f5f9;
  letter-spacing: 0.4px;
}

.status-row {
  margin-top: 8px;
  display: flex;
  align-items: center;
  gap: 10px;
}

.gear-badge {
  display: inline-block;
  padding: 4px 10px;
  border-radius: 8px;
  font-size: 12px;
  font-weight: 800;
  letter-spacing: 1.4px;
  background: rgba(148, 163, 184, 0.18);
  color: #e2e8f0;
}

.gear-forward {
  background: rgba(74, 222, 128, 0.18);
  color: #4ade80;
}

.gear-reverse {
  background: rgba(251, 146, 60, 0.2);
  color: #fb923c;
}

.gear-neutral {
  background: rgba(148, 163, 184, 0.18);
  color: #cbd5e1;
}

#connection {
  margin-top: 8px;
  font-size: 12px;
  font-weight: 600;
  letter-spacing: 0.6px;
  color: #fbbf24;
}

.speed-panel {
  min-width: 190px;
  text-align: right;
}

.speed-readout {
  display: flex;
  align-items: baseline;
  justify-content: flex-end;
  gap: 6px;
  margin-top: 2px;
}

#speed {
  font-size: 46px;
  line-height: 1;
  font-weight: 800;
  font-variant-numeric: tabular-nums;
  color: #38bdf8;
}

.unit {
  font-size: 12px;
  font-weight: 700;
  letter-spacing: 1.5px;
  color: #94a3b8;
}

.limit-row {
  margin-top: 8px;
  font-size: 12px;
  color: #cbd5e1;
  letter-spacing: 0.5px;
}

#speedLimit {
  font-weight: 700;
  color: #facc15;
  font-variant-numeric: tabular-nums;
}

.minimap-panel {
  position: fixed;
  z-index: 2;
  right: 18px;
  bottom: 64px;
  width: 176px;
  height: 176px;
  padding: 10px;
  border-radius: 16px;
  background: rgba(9, 16, 29, 0.82);
  border: 1px solid rgba(148, 163, 184, 0.24);
  backdrop-filter: blur(10px);
  box-shadow: 0 12px 30px rgba(0, 0, 0, 0.4);
  pointer-events: none;
}

.minimap-title {
  font-size: 10px;
  font-weight: 700;
  letter-spacing: 1.6px;
  color: #38bdf8;
  text-transform: uppercase;
  margin-bottom: 6px;
}

#minimapCanvas {
  display: block;
  width: 154px;
  height: 138px;
  border-radius: 10px;
  background: rgba(15, 23, 42, 0.9);
}

.controls {
  position: fixed;
  z-index: 2;
  bottom: 16px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(9, 16, 29, 0.78);
  border: 1px solid rgba(148, 163, 184, 0.2);
  border-radius: 12px;
  padding: 9px 16px;
  color: #cbd5e1;
  font-size: 12px;
  letter-spacing: 0.5px;
  text-align: center;
  white-space: nowrap;
  backdrop-filter: blur(8px);
  pointer-events: none;
}

.controls span {
  color: #38bdf8;
  font-weight: 700;
}
</style>
</head>

<body>

<div id="game"></div>

<div class="hud-top">
  <div class="panel">
    <div class="brand">CITY DRIVE 3D • FUTA</div>
    <div id="currentRoad" class="road-label">FUTA Campus Network</div>
    <div class="status-row">
      <span id="direction" class="gear-badge gear-neutral">NEUTRAL</span>
    </div>
    <div id="connection">● CONNECTING CONTROLLER...</div>
  </div>

  <div class="panel speed-panel">
    <div class="brand">SPEEDOMETER</div>
    <div class="speed-readout">
      <span id="speed">0</span>
      <span class="unit">KM/H</span>
    </div>
    <div class="limit-row">
      SPEED LIMIT: <span id="speedLimit">120</span> KM/H
    </div>
  </div>
</div>

<div class="minimap-panel">
  <div class="minimap-title">FUTA RADAR</div>
  <canvas id="minimapCanvas" width="154" height="138"></canvas>
</div>

<div class="controls">
  <span>JOYSTICK</span>: STEER &amp; THROTTLE &nbsp;|&nbsp;
  <span>POTENTIOMETER</span>: SPEED CAP &nbsp;|&nbsp;
  <span>ARROWS</span>: BACKUP DRIVE &nbsp;|&nbsp;
  <span>R</span>: RESET TO ROAD
</div>

<script type="module">
import * as THREE from 'https://cdn.jsdelivr.net/npm/three@0.160.0/build/three.module.js';

// --- PROCEDURAL TEXTURES ---
function createAsphaltTexture() {
  const canvas = document.createElement('canvas');
  canvas.width = 256;
  canvas.height = 256;
  const ctx = canvas.getContext('2d');

  ctx.fillStyle = '#252932';
  ctx.fillRect(0, 0, 256, 256);

  for (let i = 0; i < 4500; i++) {
    const x = Math.random() * 256;
    const y = Math.random() * 256;
    const v = 28 + Math.floor(Math.random() * 26);
    ctx.fillStyle = `rgb(${v},${v + 2},${v + 6})`;
    ctx.fillRect(x, y, 2, 2);
  }

  const tex = new THREE.CanvasTexture(canvas);
  tex.wrapS = THREE.RepeatWrapping;
  tex.wrapT = THREE.RepeatWrapping;
  tex.repeat.set(2, 16);
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

function createWindowTexture(baseHex, litChance = 0.35) {
  const canvas = document.createElement('canvas');
  canvas.width = 128;
  canvas.height = 256;
  const ctx = canvas.getContext('2d');

  ctx.fillStyle = baseHex;
  ctx.fillRect(0, 0, 128, 256);

  const cols = 4;
  const rows = 10;
  const padX = 8;
  const padY = 8;
  const cellW = (128 - padX * (cols + 1)) / cols;
  const cellH = (256 - padY * (rows + 1)) / rows;

  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      const x = padX + c * (cellW + padX);
      const y = padY + r * (cellH + padY);
      const lit = Math.random() < litChance;
      ctx.fillStyle = lit ? '#fef08a' : '#17253b';
      ctx.fillRect(x, y, cellW, cellH);
    }
  }

  const tex = new THREE.CanvasTexture(canvas);
  tex.wrapS = THREE.RepeatWrapping;
  tex.wrapT = THREE.RepeatWrapping;
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

// --- SCENE & ATMOSPHERE ---
const scene = new THREE.Scene();
const skyColor = new THREE.Color(0x7fb3d5);
scene.background = skyColor;
scene.fog = new THREE.FogExp2(0x89bad8, 0.0024);

const camera = new THREE.PerspectiveCamera(
  62,
  window.innerWidth / window.innerHeight,
  0.1,
  900
);
camera.position.set(0, 5.2, 10.5);

const renderer = new THREE.WebGLRenderer({
  antialias: true,
  powerPreference: 'high-performance'
});
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;
renderer.toneMapping = THREE.ACESFilmicToneMapping;
renderer.toneMappingExposure = 1.05;

document.getElementById('game').appendChild(renderer.domElement);

// --- LIGHTING ---
const hemiLight = new THREE.HemisphereLight(0xdbeafe, 0x3b4252, 1.35);
hemiLight.position.set(0, 80, 0);
scene.add(hemiLight);

const sunLight = new THREE.DirectionalLight(0xfffbeb, 2.2);
sunLight.position.set(45, 75, 38);
sunLight.castShadow = true;
sunLight.shadow.mapSize.width = 1024;
sunLight.shadow.mapSize.height = 1024;
sunLight.shadow.camera.near = 5;
sunLight.shadow.camera.far = 220;
const shadowBound = 55;
sunLight.shadow.camera.left = -shadowBound;
sunLight.shadow.camera.right = shadowBound;
sunLight.shadow.camera.top = shadowBound;
sunLight.shadow.camera.bottom = -shadowBound;
sunLight.shadow.bias = -0.0005;
scene.add(sunLight);
scene.add(sunLight.target);

// --- CAMPUS GROUND & SHARED MATERIALS ---
const groundMesh = new THREE.Mesh(
  new THREE.PlaneGeometry(3600, 3600),
  new THREE.MeshStandardMaterial({
    color: 0x2d4f38,
    roughness: 0.92,
    metalness: 0.04
  })
);
groundMesh.rotation.x = -Math.PI / 2;
groundMesh.position.set(0, -0.16, 0);
groundMesh.receiveShadow = true;
scene.add(groundMesh);

const asphaltTex = createAsphaltTexture();
const asphaltMat = new THREE.MeshStandardMaterial({
  map: asphaltTex,
  color: 0x333a45,
  roughness: 0.78,
  metalness: 0.08
});
const unpavedRoadMat = new THREE.MeshStandardMaterial({
  color: 0x6b5b47,
  roughness: 0.92,
  metalness: 0.03
});
const edgeLineMat = new THREE.MeshStandardMaterial({
  color: 0xf8fafc,
  roughness: 0.5
});
const centerLineMat = new THREE.MeshStandardMaterial({
  color: 0xfde047,
  roughness: 0.45
});
const curbMat = new THREE.MeshStandardMaterial({
  color: 0xcbd5e1,
  roughness: 0.75
});
const sidewalkMat = new THREE.MeshStandardMaterial({
  color: 0x94a3b8,
  roughness: 0.85
});
const roundaboutIslandMat = new THREE.MeshStandardMaterial({
  color: 0x3b6e4c,
  roughness: 0.82
});

const buildingPalettes = [
  createWindowTexture('#334155', 0.32),
  createWindowTexture('#1e3a5f', 0.42),
  createWindowTexture('#475569', 0.25),
  createWindowTexture('#273549', 0.38)
];
const buildingMaterials = buildingPalettes.map(
  (map) =>
    new THREE.MeshStandardMaterial({
      map,
      roughness: 0.55,
      metalness: 0.2
    })
);
const roofMat = new THREE.MeshStandardMaterial({
  color: 0x1e293b,
  roughness: 0.8
});
const poleMat = new THREE.MeshStandardMaterial({
  color: 0x475569,
  metalness: 0.7,
  roughness: 0.35
});
const lampBulbMat = new THREE.MeshBasicMaterial({
  color: 0xfff3c4
});
const trunkMat = new THREE.MeshStandardMaterial({
  color: 0x5c3a21,
  roughness: 0.9
});
const foliageMat = new THREE.MeshStandardMaterial({
  color: 0x2f6f3e,
  roughness: 0.78
});
// --- EMBEDDED FUTA OPENSTREETMAP GEOJSON DATA ---
const FUTA_GEOJSON = {
  "type": "FeatureCollection",
  "features": [
    {"type":"Feature","id":"way/367515228","properties":{"highway":"secondary","name":"FUTA South Gate Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1613245,7.2883804],[5.1612125,7.2883757],[5.1609376,7.2884364],[5.1609102,7.2884434],[5.1608679,7.2884542],[5.1603355,7.2886085],[5.1602114,7.2886451],[5.1595576,7.288816],[5.1576318,7.2893575],[5.1574802,7.2894027],[5.1570953,7.2895064],[5.1569505,7.2895424],[5.1568244,7.2895796],[5.1563001,7.2897273],[5.1560426,7.289835],[5.1558521,7.2899481],[5.1557167,7.2900465],[5.155549,7.2902062],[5.1551883,7.290584],[5.1547511,7.2910269],[5.1547168,7.2910621],[5.1545737,7.2912091],[5.1545086,7.291276],[5.1542254,7.291567],[5.1540698,7.2917147],[5.1536487,7.2921178],[5.153549,7.2921905],[5.1534663,7.2922508],[5.1531921,7.2923821],[5.1529379,7.2924583]]}},
    {"type":"Feature","id":"way/367607842","properties":{"highway":"residential","name":"Usman Dan Fodio Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1395396,7.3061225],[5.1394531,7.3061637],[5.1393804,7.3062021],[5.1389849,7.3063727],[5.1385249,7.3065647],[5.1371348,7.3071451],[5.1366451,7.3073495],[5.1356682,7.3077595],[5.1351501,7.3079769],[5.1348127,7.3081185],[5.1344588,7.3082295],[5.1341102,7.3082588],[5.1338275,7.3082495],[5.1335557,7.3082361],[5.1332601,7.3082215],[5.1329659,7.3082011],[5.1327979,7.308189]]}},
    {"type":"Feature","id":"way/367616953","properties":{"highway":"residential","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1328307,7.3052299],[5.1328302,7.3052624],[5.1328265,7.3055152],[5.1328254,7.3055891],[5.1328167,7.3058238],[5.1328093,7.306024],[5.132812,7.3065774],[5.1328012,7.3073676],[5.1327979,7.308189]]}},
    {"type":"Feature","id":"way/367621314","properties":{"highway":"residential","incline":"down","lanes":"2","maxspeed":"40","motor_vehicle":"yes","name":"Middle Belt Road","sidewalk":"both","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1368768,7.3033968],[5.1367915,7.3031504],[5.1366475,7.3026852],[5.1365394,7.3023304],[5.1365043,7.3022151],[5.1361942,7.3012459],[5.13619,7.3012331],[5.1360808,7.3009037],[5.1357691,7.2999968],[5.1356551,7.299629]]}},
    {"type":"Feature","id":"way/367621328","properties":{"highway":"residential","lanes":"2","name":"Staff Quarters Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1283068,7.3031567],[5.1285223,7.3031431],[5.128722,7.3031179],[5.1288485,7.3031249],[5.1289654,7.3031313],[5.1290201,7.3031343],[5.1299107,7.3031831],[5.1306824,7.3032375],[5.1310779,7.3032625],[5.1315775,7.3032785],[5.1323064,7.3033277],[5.1326269,7.303343],[5.1327234,7.3033736],[5.1327481,7.3033915],[5.1327811,7.3034155],[5.13282,7.303474],[5.1328455,7.3035844],[5.1328441,7.3040819],[5.1328424,7.3042287],[5.1328355,7.304812],[5.1328328,7.3050465],[5.1328307,7.3052299]]}},
    {"type":"Feature","id":"way/367637513","properties":{"foot":"yes","highway":"residential","horse":"yes","lanes":"2","maxspeed":"40","name":"Middle Belt Road","sidewalk":"both","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1354905,7.2990606],[5.1356404,7.2989965],[5.1357767,7.2989993],[5.1358396,7.2990076],[5.136166,7.2990465],[5.1365831,7.2990962],[5.1369012,7.2991371],[5.1373369,7.2992336],[5.1375769,7.2992612],[5.1378035,7.2992814],[5.1379229,7.2992562],[5.1379776,7.2992409],[5.1380893,7.299198],[5.1383668,7.2991135],[5.1385613,7.2990204],[5.138642,7.2989952],[5.1387156,7.2989964],[5.1388211,7.2990869]]}},
    {"type":"Feature","id":"way/367637515","properties":{"highway":"residential","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1395329,7.301037],[5.1394793,7.3010404],[5.1392788,7.301053],[5.1390421,7.3010816],[5.1387009,7.3010921],[5.1386298,7.3010943],[5.1385889,7.3010909],[5.138559,7.3010792],[5.1385304,7.3010603],[5.1384936,7.301051]]}},
    {"type":"Feature","id":"way/367637529","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"2","maxspeed":"40","motor_vehicle":"yes","name":"Bank Road","sidewalk":"both","smoothness":"excellent","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1397274,7.3017361],[5.1397024,7.3016371],[5.1396371,7.3014143],[5.1395329,7.301037],[5.1394518,7.3006805],[5.1393914,7.3003586],[5.139319,7.3000367],[5.1392506,7.2998039],[5.139146,7.299603],[5.1389971,7.2993078],[5.1388885,7.29916],[5.1388235,7.2990895],[5.1388211,7.2990869]]}},
    {"type":"Feature","id":"way/367637552","properties":{"highway":"residential","lanes":"2","name":"Niger Delta Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1383432,7.3037082],[5.1380074,7.3036802],[5.1376735,7.3036616],[5.137428,7.3036403],[5.1370268,7.3036237],[5.13685,7.3036257],[5.1366475,7.3036483],[5.1364128,7.3036842],[5.1361339,7.3037854],[5.1360093,7.3038281],[5.1358536,7.3038757],[5.1358036,7.3038992],[5.1354783,7.3040522],[5.1353104,7.3041311],[5.1352726,7.3041479],[5.1351838,7.3041872],[5.1351162,7.3042171],[5.1340748,7.3046786],[5.1337934,7.3048033],[5.1336134,7.304883],[5.1332481,7.3050449],[5.1328307,7.3052299]]}},
    {"type":"Feature","id":"way/391371328","properties":{"highway":"residential","name":"Oduduwa Road","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1389375,7.304034],[5.1391182,7.3046944],[5.1391795,7.3049153],[5.1391949,7.3049582],[5.1394531,7.305813],[5.1395396,7.3061225],[5.139735,7.3067417],[5.139834,7.307041]]}},
    {"type":"Feature","id":"way/391371329","properties":{"highway":"residential","name":"Redemption road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1457215,7.2898018],[5.1457079,7.2899174],[5.1456915,7.2900447],[5.1456724,7.2901626],[5.1456346,7.2903544],[5.1456352,7.2904034],[5.1456275,7.2904575],[5.1455356,7.290832],[5.1453713,7.2916061],[5.1453498,7.2917366],[5.1453313,7.2919163],[5.1453368,7.2920677],[5.1453458,7.2921028],[5.1453646,7.2921288],[5.1453915,7.2921471],[5.145421,7.2921625],[5.1459715,7.2922882],[5.1460269,7.2923069],[5.1464485,7.2923853],[5.1468241,7.2924628],[5.1469344,7.2924813],[5.1470467,7.2924905],[5.1471821,7.292515],[5.1473155,7.2925533],[5.1474699,7.2925802],[5.1483084,7.2927669],[5.1483969,7.2927855],[5.1484841,7.2927935],[5.1499929,7.2927017]]}},
    {"type":"Feature","id":"way/391371333","properties":{"highway":"residential","junction":"roundabout","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1387769,7.3037986],[5.1387266,7.3038004],[5.1386786,7.3037853],[5.1386505,7.3037665],[5.1386178,7.3037267],[5.1386009,7.3036782],[5.1386019,7.3036269],[5.1386208,7.3035791],[5.1386325,7.3035627],[5.1386778,7.303526],[5.138734,7.3035096],[5.1387874,7.3035139],[5.1388017,7.3035197],[5.1388478,7.303549],[5.13888,7.303593],[5.1388938,7.3036455],[5.1388873,7.3036995],[5.1388616,7.3037472],[5.1388238,7.3037802],[5.1387769,7.3037986]]}},
    {"type":"Feature","id":"way/391371336","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1388616,7.3037472],[5.1388872,7.3038305],[5.1389375,7.304034]]}},
    {"type":"Feature","id":"way/391371337","properties":{"foot":"yes","highway":"residential","lanes":"1","maxspeed":"40","motor_vehicle":"yes","oneway":"yes","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1390844,7.3033331],[5.1388873,7.3036995]]}},
    {"type":"Feature","id":"way/391371338","properties":{"bicycle":"yes","highway":"residential","lanes":"1","maxspeed":"40","motor_vehicle":"yes","oneway":"yes","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1388017,7.3035197],[5.138861,7.3034637],[5.1389539,7.3034075],[5.1390844,7.3033331]]}},
    {"type":"Feature","id":"way/391371339","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","maxspeed":"40","motor_vehicle":"yes","surface":"asphalt","wheelchair":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1337932,7.2985172],[5.1339108,7.2985103],[5.1341998,7.2984935],[5.1342506,7.2984905],[5.1346649,7.2984663],[5.1347474,7.2984615],[5.1348263,7.2984569],[5.1351482,7.2984378],[5.1353094,7.2984272],[5.1354708,7.2984159]]}},
    {"type":"Feature","id":"way/391371340","properties":{"highway":"residential","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1386168,7.303252],[5.1385118,7.3028969],[5.1382129,7.3019093],[5.1382028,7.3018771],[5.1381918,7.3018488],[5.1381824,7.3018182],[5.1382458,7.3018079],[5.1382964,7.3018032],[5.1385371,7.3017826],[5.1390883,7.3017354]]}},
    {"type":"Feature","id":"way/391371341","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1386505,7.3037665],[5.1383432,7.3037082]]}},
    {"type":"Feature","id":"way/391371342","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1388033,7.3040739],[5.1387403,7.3039176],[5.1386786,7.3037853]]}},
    {"type":"Feature","id":"way/391371344","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1383432,7.3037082],[5.1385059,7.303622],[5.1386208,7.3035791]]}},
    {"type":"Feature","id":"way/391371345","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1386325,7.3035627],[5.1386225,7.3034422],[5.1386196,7.3033483],[5.1386168,7.303252]]}},
    {"type":"Feature","id":"way/391371346","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1386168,7.303252],[5.1387452,7.3034148],[5.1387688,7.3034524],[5.1387874,7.3035139]]}},
    {"type":"Feature","id":"way/391371347","properties":{"highway":"residential","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1397361,7.3070776],[5.139647,7.3067722],[5.1394531,7.3061637],[5.1390903,7.3049868],[5.1390776,7.3049479],[5.1390075,7.3047286],[5.1388033,7.3040739]]}},
    {"type":"Feature","id":"way/391371350","properties":{"highway":"residential","maxspeed:forward":"40","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1354905,7.2990606],[5.1354546,7.2988792],[5.13545,7.2987198],[5.1354499,7.2986732],[5.1354614,7.2986362],[5.1354622,7.2985531],[5.1354708,7.2984159],[5.1354893,7.2981158],[5.1355035,7.2979687],[5.135516,7.2978399],[5.13553,7.2976957],[5.1355471,7.2975191],[5.1355569,7.2973652],[5.1355589,7.2973336],[5.1355828,7.2969573],[5.1355666,7.2965608],[5.1355532,7.2961936],[5.1355398,7.2957786],[5.135521,7.2952332]]}},
    {"type":"Feature","id":"way/391371351","properties":{"highway":"residential","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1315339,7.2986945],[5.13144,7.2987038],[5.1314548,7.2988249],[5.1314695,7.2989991],[5.131541,7.2997059],[5.1315862,7.3001524]]}},
    {"type":"Feature","id":"way/391371353","properties":{"highway":"residential","lanes":"2","name":"Quarters way","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1315775,7.3032785],[5.1315894,7.3029689],[5.1315871,7.3024734],[5.1315882,7.3023846],[5.1315846,7.3021478],[5.1315828,7.3019097],[5.1315701,7.3013656],[5.1315889,7.3007963],[5.1315862,7.3001524]]}},
    {"type":"Feature","id":"way/391371354","properties":{"bicycle":"yes","foot":"yes","highway":"residential","lanes":"2","maxspeed":"40","motor_vehicle":"yes","sidewalk":"both","surface":"asphalt","wheelchair":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1315339,7.2986945],[5.1318363,7.2986737],[5.1321037,7.2986528],[5.1327388,7.298603],[5.1328155,7.2985965],[5.1328517,7.2985922],[5.1331082,7.2985735],[5.1333864,7.2985504],[5.1334269,7.2985473],[5.1337472,7.298521],[5.1337932,7.2985172]]}},
    {"type":"Feature","id":"way/557039651","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1328254,7.3055891],[5.1331092,7.3055671],[5.133339,7.3055492],[5.1341001,7.3054794],[5.134239,7.3054667],[5.1343784,7.3054568],[5.1344351,7.3054528],[5.1347978,7.305427],[5.1351602,7.3053948],[5.1351724,7.3053937],[5.1352165,7.3054],[5.1354419,7.3052831],[5.13552,7.3052076],[5.135528,7.305189],[5.1355243,7.3051567],[5.1354371,7.3050195],[5.1354102,7.304977],[5.1354239,7.304925],[5.1354556,7.3049107],[5.1354834,7.3049015],[5.1359273,7.3047537],[5.1360045,7.3047411],[5.1360863,7.3047504],[5.1361808,7.304763],[5.136268,7.3047484],[5.1363511,7.3047071],[5.1363927,7.304638],[5.1364073,7.3039461],[5.136408,7.3039112],[5.1364119,7.3037267],[5.1364128,7.3036842]]}},
    {"type":"Feature","id":"way/557039657","properties":{"highway":"service"},"geometry":{"type":"LineString","coordinates":[[5.1407634,7.3035153],[5.1410905,7.3037241],[5.1413065,7.3038664],[5.1415493,7.3040314]]}},
    {"type":"Feature","id":"way/557039659","properties":{"access":"yes","bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"2","motor_vehicle":"yes","name":"Malu Road","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1424344,7.3019961],[5.142484,7.3020161],[5.1425886,7.3020853],[5.1428836,7.3022875],[5.1429521,7.3023141],[5.1430017,7.3023189],[5.1430755,7.3023],[5.1431257,7.302262],[5.1432337,7.3021358],[5.1436333,7.3017274],[5.1438056,7.3015219],[5.1438185,7.3015079],[5.1438386,7.3014813],[5.1439049,7.3014081],[5.1439894,7.301337],[5.144033,7.3013011],[5.1440645,7.3012678],[5.1440839,7.3012485],[5.1441275,7.301214],[5.1442006,7.301182],[5.1442422,7.3011608],[5.1442857,7.3011395],[5.1443401,7.3011115],[5.1444025,7.3010789],[5.1446411,7.3009027],[5.1447009,7.3008129],[5.1447236,7.300747],[5.1447391,7.3006706],[5.1447418,7.3005664],[5.1447438,7.3004897]]}},
    {"type":"Feature","id":"way/557049420","properties":{"highway":"service"},"geometry":{"type":"LineString","coordinates":[[5.1348243,7.3012771],[5.1351804,7.3011658],[5.1351975,7.3011609],[5.1352942,7.3011328],[5.1355556,7.3010568],[5.1357098,7.301012],[5.1357456,7.3010016]]}},
    {"type":"Feature","id":"way/557049421","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1391949,7.3049582],[5.1392244,7.3049516],[5.1393713,7.3049187]]}},
    {"type":"Feature","id":"way/557049424","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1390903,7.3049868],[5.1391949,7.3049582]]}},
    {"type":"Feature","id":"way/557049425","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1264175,7.3035791],[5.1267744,7.3037654],[5.1273666,7.3040506]]}},
    {"type":"Feature","id":"way/557049426","properties":{"highway":"residential","name":"Staff Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1274717,7.3030523],[5.1273989,7.3034333],[5.1273886,7.3037068],[5.1273887,7.3037658],[5.1273891,7.3039065],[5.1273666,7.3040506],[5.1273597,7.3042467],[5.1273537,7.3044158],[5.1273248,7.3048874],[5.1273369,7.3049592],[5.1273671,7.3050098],[5.1274254,7.3050503],[5.127524,7.3050789],[5.127577,7.3050843],[5.1279736,7.3051245],[5.1282455,7.3051521]]}},
    {"type":"Feature","id":"way/557049427","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1292433,7.3053337],[5.1292647,7.3049705],[5.1292782,7.3047204],[5.1293009,7.3046047],[5.1293077,7.304497],[5.1293693,7.3043838]]}},
    {"type":"Feature","id":"way/557049428","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1309586,7.304924],[5.1309815,7.3045735],[5.131003,7.3042458],[5.1310042,7.3042269],[5.1310452,7.3036909],[5.1310779,7.3032625]]}},
    {"type":"Feature","id":"way/557049429","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.13087,7.3072878],[5.131044,7.3065113],[5.1312248,7.3057041]]}},
    {"type":"Feature","id":"way/557049430","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1283696,7.3052851],[5.1292433,7.3053337],[5.1295356,7.3053603],[5.129858,7.3054108],[5.1299069,7.3054185],[5.1303463,7.3055051],[5.1312248,7.3057041],[5.1315057,7.3057633],[5.1319867,7.3058595],[5.132657,7.3059935],[5.132798,7.3060217],[5.1328093,7.306024]]}},
    {"type":"Feature","id":"way/557049431","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1283273,7.3071561],[5.1291788,7.3072047],[5.1296563,7.3072319],[5.1298538,7.307241],[5.1303571,7.3072642],[5.13087,7.3072878],[5.1321822,7.3073637],[5.1322031,7.3073649],[5.1328012,7.3073676]]}},
    {"type":"Feature","id":"way/557049432","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1240297,7.3039948],[5.1242624,7.3035299],[5.1244757,7.303043],[5.1245642,7.3028275],[5.1245987,7.3027054],[5.124654,7.3025096],[5.124709,7.3022608],[5.1247301,7.3021474]]}},
    {"type":"Feature","id":"way/557049433","properties":{"highway":"residential","lanes":"2","name":"Staff Quarters Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1247301,7.3021474],[5.1248404,7.3021558],[5.1249025,7.3021697],[5.125173,7.3022263],[5.1255552,7.3023407],[5.1267019,7.3027876],[5.1271203,7.3029512],[5.1274717,7.3030523],[5.1276594,7.3030577],[5.127988,7.303075],[5.1282101,7.3030887],[5.1283068,7.3031567]]}},
    {"type":"Feature","id":"way/557049434","properties":{"highway":"residential","lanes":"1","name":"Usman Dan Fodio Road","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1240297,7.3039948],[5.1242383,7.3042588],[5.1251127,7.3055106],[5.1258878,7.3066971],[5.1260166,7.3068661],[5.1261346,7.3070097],[5.1263063,7.3071348],[5.1266375,7.3073117],[5.1269639,7.3074909],[5.1277573,7.3079263],[5.1279183,7.3079781],[5.128102,7.3080114],[5.1282535,7.3080207],[5.1284145,7.3080154]]}},
    {"type":"Feature","id":"way/557049435","properties":{"highway":"residential","name":"Usman Dan Fodio Road","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1284145,7.3080154],[5.1287752,7.3080407],[5.1295826,7.3080779],[5.1298154,7.3080859],[5.1304731,7.3081085],[5.1315433,7.3081524],[5.1321343,7.3081696],[5.1327979,7.308189]]}},
    {"type":"Feature","id":"way/557049436","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1283068,7.3031567],[5.1283004,7.3035077],[5.1282937,7.3037334],[5.1282786,7.3039852],[5.1282756,7.3040355],[5.1282578,7.304585],[5.1282563,7.3046206],[5.1282455,7.3051521],[5.1282448,7.3052439],[5.1282517,7.305339],[5.1282415,7.3057287],[5.1282427,7.3057586],[5.1282433,7.305775],[5.1282549,7.3060786],[5.128284,7.3066239],[5.1282857,7.3066559],[5.1283273,7.3071561],[5.1283298,7.3071899],[5.1283447,7.3073888],[5.128389,7.3077154],[5.1284044,7.307883],[5.128412,7.3079828],[5.1284145,7.3080154]]}},
    {"type":"Feature","id":"way/557061198","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1538371,7.2950915],[5.153984,7.2950423],[5.1542321,7.2949465],[5.1544131,7.2948687],[5.1546009,7.2947882],[5.154739,7.2947231],[5.1548718,7.2946585],[5.1550059,7.2945973],[5.1551313,7.2945528],[5.1552419,7.2945102],[5.1553345,7.294461],[5.1554679,7.2943779],[5.1555611,7.2943313],[5.1556516,7.2943073],[5.1557348,7.294304],[5.1557764,7.2943113]]}},
    {"type":"Feature","id":"way/557061199","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1539783,7.2941514],[5.1540822,7.2941923],[5.1541241,7.2942036],[5.1541965,7.2942069],[5.1542602,7.2941976],[5.1543447,7.2941657],[5.1545204,7.294024],[5.1546539,7.2939256],[5.1548879,7.2937839],[5.1549784,7.293742],[5.1555845,7.2933976]]}},
    {"type":"Feature","id":"way/557061200","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1531921,7.2923821],[5.1532752,7.2927702],[5.1533014,7.2928906],[5.1533423,7.2929937],[5.1534127,7.2931234],[5.1537761,7.2936449],[5.1539612,7.2939761],[5.1539954,7.2940619],[5.1539974,7.2941085],[5.1539783,7.2941514],[5.1539303,7.2941777],[5.1535937,7.2943173],[5.1535582,7.2943353],[5.153707,7.2947543],[5.1538371,7.2950915],[5.1538868,7.2952272],[5.1539582,7.2953965],[5.15399,7.2955132],[5.1540074,7.2956196],[5.1540242,7.295891],[5.1540396,7.2960187],[5.1540571,7.29604],[5.1540879,7.2960453]]}},
    {"type":"Feature","id":"way/557068192","properties":{"highway":"tertiary","lanes":"1","maxspeed":"40","name":"Stateline Road","oneway":"no","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1513842,7.292741],[5.151397,7.292578],[5.1514345,7.2923466],[5.1515177,7.2921297],[5.1515688,7.2920425],[5.1516035,7.2919834],[5.1516453,7.2919314],[5.1519978,7.2914886],[5.1523847,7.2910443],[5.1525666,7.2908583],[5.1525812,7.2908434],[5.152722,7.2906465],[5.1527958,7.2904669],[5.152832,7.2903046],[5.1528333,7.2901077],[5.152781,7.2899228],[5.1526228,7.289561],[5.1523666,7.288977],[5.1519857,7.2881884],[5.1519147,7.2880392],[5.1517845,7.2877766],[5.1517135,7.2876282],[5.1514196,7.2869989]]}},
    {"type":"Feature","id":"way/557068193","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1468681,7.2917182],[5.146847,7.2916772],[5.1468359,7.2916322],[5.1468354,7.2915625],[5.1469767,7.2911041],[5.1470468,7.2907247]]}},
    {"type":"Feature","id":"way/557068194","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1461278,7.291596],[5.1467344,7.2917069],[5.1468681,7.2917182],[5.1475681,7.2918796]]}},
    {"type":"Feature","id":"way/557068195","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1459715,7.2922882],[5.146065,7.2919558],[5.1461278,7.291596],[5.1461583,7.2914205],[5.1462377,7.2910894],[5.1463101,7.2905797]]}},
    {"type":"Feature","id":"way/557068196","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1456275,7.2904575],[5.1463101,7.2905797],[5.1465142,7.2906173],[5.1470468,7.2907247],[5.1474456,7.2907814],[5.14752,7.290797],[5.1475692,7.2908077],[5.1476098,7.2908304],[5.1476521,7.2908659]]}},
    {"type":"Feature","id":"way/557068197","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1476521,7.2908659],[5.147667,7.290907],[5.1476644,7.2911146],[5.1476135,7.2915757],[5.1475681,7.2918796],[5.1475306,7.2921803],[5.1474699,7.2925802]]}},
    {"type":"Feature","id":"way/557068198","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1425666,7.29067],[5.1425575,7.2904444],[5.1425413,7.2903332],[5.1425334,7.2902625],[5.1425247,7.2901768],[5.1425078,7.2899802],[5.142489,7.2897671],[5.1424689,7.2895563],[5.1424799,7.2893986],[5.1424733,7.2893588],[5.142473,7.2893078],[5.1424754,7.2892811],[5.1424679,7.2892514],[5.1424494,7.2892179],[5.1424319,7.2891891],[5.1423834,7.2891397],[5.1423649,7.2891062],[5.1423569,7.2890659],[5.1423574,7.2890354],[5.1423609,7.2890049],[5.1423732,7.2889748],[5.1423924,7.2889333],[5.1424026,7.2888812],[5.1424041,7.2888313],[5.142393,7.2887847],[5.1423743,7.2886991],[5.1423654,7.2886401],[5.1423708,7.2885813],[5.1423842,7.2884769],[5.1423787,7.2884159],[5.1423619,7.2882881],[5.1423659,7.2881578],[5.1423674,7.2881014],[5.1423791,7.2880182],[5.1424063,7.2879287],[5.1424091,7.2878614],[5.1423959,7.2877524],[5.142359,7.287685],[5.1423185,7.2876319],[5.1422976,7.2875895],[5.1422793,7.2875398],[5.1422685,7.287458],[5.1422565,7.2873731],[5.1422612,7.2873277],[5.1422659,7.2873047],[5.1422787,7.2872843],[5.1423056,7.2872697],[5.1423909,7.2872452],[5.1424625,7.2872194],[5.1425161,7.2871839],[5.1425757,7.2871355],[5.1426329,7.2870706],[5.1426487,7.2870356],[5.1426544,7.2869815],[5.1426535,7.2869446],[5.1426541,7.2868992],[5.1426336,7.2868584],[5.1425901,7.2868204],[5.142515,7.2867563],[5.1424388,7.2867017],[5.1424116,7.2866628],[5.1423181,7.2865361],[5.1422877,7.286513],[5.1422252,7.2864733]]}},
    {"type":"Feature","id":"way/557068199","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1457215,7.2898018],[5.14573,7.2897061],[5.1457419,7.2896311],[5.1457377,7.2895751],[5.1456913,7.2894675],[5.1456408,7.2894053],[5.1455946,7.2893005],[5.1455095,7.2890947],[5.1454739,7.2890276],[5.1454538,7.2890187],[5.1454292,7.2890161],[5.1453947,7.289022],[5.1453576,7.2890347],[5.1453288,7.2890588],[5.1452899,7.2890801],[5.1452034,7.2891361],[5.1451521,7.2891701],[5.1451167,7.2892057],[5.1450396,7.2892549],[5.1449821,7.2892777],[5.1449274,7.2892905],[5.1448434,7.2893067],[5.1446706,7.289365],[5.1445437,7.2894189],[5.1443232,7.2895196],[5.1442284,7.28957],[5.1440944,7.2896571],[5.1439679,7.2896986],[5.1438527,7.2897409],[5.14377,7.2897747],[5.1437461,7.2897869],[5.1437234,7.28981],[5.1436978,7.2898167],[5.1435222,7.2898386],[5.1434185,7.2898481],[5.1433841,7.2898471],[5.1433501,7.2898407],[5.1433305,7.2898319],[5.1433045,7.2898144],[5.1432782,7.2897801],[5.1432478,7.2897252],[5.1432297,7.2896662],[5.1432082,7.2894785],[5.1431846,7.2893226],[5.1431705,7.2892353],[5.143174,7.2891298],[5.143189,7.2890517],[5.1431922,7.2889798],[5.143196,7.2888878],[5.1432035,7.2888346],[5.1432129,7.2887927],[5.1432032,7.2887621],[5.1431839,7.2887268],[5.1431616,7.2886753],[5.1431544,7.2886388],[5.1431342,7.2886102],[5.1431099,7.2885856],[5.1429928,7.2885428]]}},
    {"type":"Feature","id":"way/557068200","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1440236,7.2928653],[5.143986,7.2925727],[5.1439271,7.2921996],[5.1439138,7.2919828],[5.1439132,7.2918899],[5.1439003,7.2917272],[5.1439866,7.2917136],[5.1440665,7.2916952],[5.1441134,7.2916747],[5.144154,7.2916508],[5.144224,7.2915935],[5.1442539,7.2915593],[5.1442967,7.2915217],[5.1443177,7.2914955],[5.1443363,7.2914797],[5.1443635,7.2914757],[5.1444152,7.2914669],[5.1444946,7.2914664],[5.1446997,7.2915],[5.1448629,7.2915248],[5.1453713,7.2916061]]}},
    {"type":"Feature","id":"way/557068201","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1432591,7.2929704],[5.1437286,7.2928946],[5.1440236,7.2928653],[5.1443884,7.2928081],[5.1447586,7.2927297]]}},
    {"type":"Feature","id":"way/557068202","properties":{"highway":"residential","name":"Apatapiti Road, FUTA."},"geometry":{"type":"LineString","coordinates":[[5.1500814,7.2929798],[5.1499929,7.2927017],[5.1498426,7.29232],[5.1497229,7.2920061]]}},
    {"type":"Feature","id":"way/557068203","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1497229,7.2920061],[5.1493308,7.2920347],[5.1490817,7.2920685],[5.148826,7.2920559],[5.1486355,7.2920504],[5.1485058,7.2920545],[5.1483936,7.2920656]]}},
    {"type":"Feature","id":"way/557068234","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.150756,7.29286],[5.1507546,7.2930682],[5.1507398,7.2932112],[5.1507016,7.2933848],[5.1506521,7.2935584],[5.1506272,7.2936575],[5.150589,7.2937746],[5.1505065,7.2939888],[5.1505038,7.294026],[5.1505102,7.2940446],[5.1505236,7.2940576],[5.1509544,7.2941438]]}},
    {"type":"Feature","id":"way/557068235","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1477291,7.2975824],[5.1477572,7.2970836]]}},
    {"type":"Feature","id":"way/557068236","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1463343,7.2980772],[5.1466569,7.2980686],[5.1466803,7.2980573],[5.1466951,7.2980234],[5.1467574,7.2978578],[5.1467843,7.2978385],[5.1468104,7.2978305],[5.1469051,7.2978131],[5.1469479,7.2978052],[5.1470304,7.2977833],[5.1471249,7.2977334],[5.1472355,7.2976622],[5.1473133,7.297621],[5.1473804,7.2975951],[5.147485,7.2975871],[5.1477291,7.2975824]]}},
    {"type":"Feature","id":"way/557068237","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1472161,7.3074567],[5.1473254,7.307474],[5.1473536,7.307466],[5.1473817,7.3074447],[5.1474421,7.3073556],[5.1475695,7.3071015],[5.1476848,7.3068794],[5.1477613,7.306771],[5.1478672,7.3066453],[5.1479826,7.3064896],[5.1480469,7.3063486],[5.1481274,7.3061066],[5.1481703,7.305911],[5.1482615,7.3055106]]}},
    {"type":"Feature","id":"way/557068238","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1458086,7.2988993],[5.1458475,7.2988502],[5.1459132,7.2986666],[5.1459427,7.2985216],[5.1459588,7.2983699],[5.1459776,7.2982249],[5.1460259,7.2980201]]}},
    {"type":"Feature","id":"way/557068239","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1453278,7.2988555],[5.1454974,7.2988734],[5.1457624,7.2989084],[5.1458086,7.2988993]]}},
    {"type":"Feature","id":"way/557068240","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1455256,7.2978471],[5.1456155,7.2978937],[5.1457107,7.2979429],[5.1459226,7.2979975],[5.1460259,7.2980201],[5.146291,7.2980692],[5.1463343,7.2980772],[5.1463035,7.2982648],[5.1462137,7.2986812]]}},
    {"type":"Feature","id":"way/557068241","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1456879,7.2970437],[5.1457999,7.2971554],[5.1458415,7.297188],[5.1458904,7.2972046],[5.1463015,7.2972346],[5.1464899,7.2972432],[5.1468158,7.2972419],[5.146907,7.2972279],[5.1470203,7.2971946],[5.1472181,7.2971182],[5.1473428,7.2970689],[5.1475078,7.297053],[5.1476372,7.2970689],[5.1477572,7.2970836],[5.1480938,7.2971235],[5.1486987,7.2971727]]}},
    {"type":"Feature","id":"way/557068243","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1509464,7.2959569],[5.151049,7.295883],[5.1511496,7.2957946],[5.1512106,7.2957234],[5.1512656,7.2956456],[5.1513065,7.2955884],[5.151395,7.2954248]]}},
    {"type":"Feature","id":"way/557068244","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1508042,7.2949751],[5.1510651,7.2951946],[5.15122,7.2953084],[5.151395,7.2954248],[5.1515224,7.2955059],[5.1516632,7.295597],[5.1518637,7.2957247],[5.1519844,7.2958092],[5.1520766,7.2958767],[5.1521098,7.2958937],[5.1521443,7.2959023],[5.1522352,7.2959073],[5.1522888,7.2958997],[5.1523344,7.295879],[5.1524471,7.2958205]]}},
    {"type":"Feature","id":"way/557068245","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1504233,7.2955272],[5.1506996,7.2957267],[5.1508391,7.2958398],[5.1509464,7.2959569]]}},
    {"type":"Feature","id":"way/557068246","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1502933,7.2975492],[5.150068,7.2973337],[5.1500197,7.2972618],[5.1500224,7.297174],[5.1500666,7.2970357],[5.150312,7.2967178],[5.1504931,7.2964916],[5.1506647,7.2962868],[5.1509464,7.2959569]]}},
    {"type":"Feature","id":"way/557068247","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1509464,7.2959569],[5.1509759,7.2960207],[5.1510014,7.2960646],[5.1511583,7.2962375],[5.151456,7.296485],[5.1514674,7.2965173],[5.151464,7.2965774],[5.1512253,7.2968761],[5.1511274,7.2969905],[5.1510637,7.2970783],[5.1509035,7.2973363]]}},
    {"type":"Feature","id":"way/557068248","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1510604,7.2937087],[5.151401,7.2937659],[5.1516987,7.2937766],[5.1522164,7.2937447]]}},
    {"type":"Feature","id":"way/557068249","properties":{"highway":"residential","name":"Peace Avenue Street","oneway":"yes","surface":"ground"},"geometry":{"type":"LineString","coordinates":[[5.1504931,7.2964916],[5.1499432,7.2960606],[5.1498762,7.2959848],[5.1498614,7.2959249],[5.1498789,7.2958731],[5.1500706,7.2957467],[5.1504233,7.2955272],[5.1505226,7.2954141],[5.1506379,7.2952611],[5.1506839,7.2951857],[5.1507117,7.2951401],[5.1508042,7.2949751],[5.1508619,7.2948394],[5.1509021,7.2946492],[5.150923,7.2944473],[5.1509544,7.2941438],[5.1510282,7.2938364],[5.1510604,7.2937087],[5.1510966,7.2934626],[5.1511395,7.2931021],[5.1511623,7.2927949]]}},
    {"type":"Feature","id":"way/557068250","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1510631,7.3000766],[5.1517913,7.2997227],[5.1518717,7.2996569],[5.1518898,7.299617],[5.1518919,7.2995711],[5.1518335,7.2993244],[5.1518007,7.2992366],[5.1517671,7.2991767],[5.1517148,7.2991188],[5.151637,7.299039],[5.15158,7.2989652],[5.1515512,7.298912],[5.1515157,7.2987989],[5.1514976,7.2985927],[5.1514533,7.2983313],[5.1514392,7.2981624],[5.1514412,7.2981192],[5.151456,7.2980786],[5.1514915,7.2980214],[5.1516585,7.2978205],[5.1517832,7.2976283],[5.1518623,7.2976456],[5.1519214,7.2976815],[5.151975,7.2977334],[5.1520769,7.2978591],[5.1521373,7.2979868],[5.1521668,7.298177],[5.1521949,7.2983992]]}},
    {"type":"Feature","id":"way/557068251","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1510443,7.2991521],[5.1510416,7.2993982],[5.1510416,7.2995884],[5.1510403,7.2997653],[5.1510389,7.299889],[5.151047,7.2999955],[5.1510631,7.3000766],[5.1510899,7.300183],[5.1511382,7.3003041],[5.1512884,7.3005888],[5.1514292,7.3008375],[5.1516759,7.3012486]]}},
    {"type":"Feature","id":"way/557077694","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1475574,7.2891859],[5.1478564,7.2894652],[5.148055,7.2896249],[5.1481006,7.2896421],[5.1481555,7.2896421],[5.14824,7.2896129],[5.1484607,7.2894606]]}},
    {"type":"Feature","id":"way/557077695","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1465044,7.2891189],[5.1466468,7.2892284],[5.146687,7.2892524],[5.1468345,7.2893974],[5.1470223,7.2896302],[5.1470639,7.2896581],[5.1471229,7.2896528],[5.1471913,7.2896156],[5.1473683,7.2894293],[5.1475574,7.2891859],[5.1476419,7.2890621],[5.1476915,7.288997],[5.1477157,7.2889278],[5.1476902,7.2888746],[5.1476043,7.2888041],[5.1471504,7.2885387]]}},
    {"type":"Feature","id":"way/557077705","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1491759,7.2896197],[5.1495282,7.2895577],[5.1498643,7.2894879],[5.150161,7.2894161],[5.1504534,7.2893685],[5.1506754,7.2893489],[5.15086,7.2893141],[5.1510374,7.2892694],[5.1513354,7.2891858],[5.1514008,7.2891559],[5.1514525,7.2891222],[5.1515119,7.2890854]]}},
    {"type":"Feature","id":"way/557077706","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1484106,7.2890567],[5.1490446,7.2889239],[5.1495347,7.2888034],[5.1496479,7.2888106],[5.1499446,7.2887801],[5.1502041,7.2887229],[5.1502356,7.2887109],[5.1502564,7.2886823],[5.1502597,7.2886318],[5.1502268,7.2886042],[5.1501919,7.2884356],[5.1501612,7.2882773],[5.1500886,7.2878744],[5.1500708,7.2877402],[5.1500686,7.2876755],[5.150083,7.2876337],[5.1501007,7.287604],[5.1501183,7.2875687]]}},
    {"type":"Feature","id":"way/557077707","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1463942,7.2869416],[5.1464582,7.2870367],[5.1465536,7.2871563],[5.1465988,7.2872068],[5.1466556,7.2872496],[5.1469986,7.287375],[5.1472273,7.2874385],[5.1472638,7.2874752],[5.1472771,7.2875596],[5.1472758,7.2876885],[5.147299,7.28775],[5.1473477,7.2878175],[5.1474568,7.287966],[5.1475686,7.2880153],[5.1478256,7.2881351],[5.1479512,7.2881966],[5.148067,7.2882693],[5.1481689,7.2883292],[5.1482373,7.2883797],[5.1482923,7.2884396],[5.1483259,7.2885287],[5.1484106,7.2890567],[5.1484466,7.2892133],[5.1484613,7.2893162],[5.1484657,7.2894045],[5.1484607,7.2894606],[5.1484559,7.2895471],[5.1484629,7.2897766],[5.1485015,7.290004],[5.1485336,7.2901821],[5.1485642,7.2903425],[5.148612,7.2904778],[5.1486724,7.2905095],[5.1487462,7.2905198],[5.1489349,7.2905368],[5.1490994,7.2905409],[5.1493329,7.2905029]]}},
    {"type":"Feature","id":"way/557077708","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1490446,7.2889239],[5.1491759,7.2896197],[5.1492298,7.2899268],[5.1493329,7.2905029],[5.1494162,7.2909684],[5.1494392,7.2912227]]}},
    {"type":"Feature","id":"way/557077709","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.148747,7.2913182],[5.1490943,7.2913116],[5.1494392,7.2912227],[5.1496295,7.2911927],[5.1502047,7.2911507]]}},
    {"type":"Feature","id":"way/557077710","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1481797,7.2911653],[5.1483762,7.2913109],[5.1483849,7.2913249],[5.1483889,7.2913422],[5.1483916,7.2913854],[5.1483912,7.2914144],[5.1483882,7.2914244],[5.1483621,7.2914746],[5.1483393,7.2915371],[5.1483312,7.2916123],[5.1483332,7.2917027],[5.1483433,7.2917812],[5.1483634,7.2918663],[5.1483715,7.2919262],[5.1483936,7.2920656]]}},
    {"type":"Feature","id":"way/557077713","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1517148,7.2898084],[5.1518637,7.2897539],[5.1521132,7.2896727]]}},
    {"type":"Feature","id":"way/557077714","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1517148,7.2898084],[5.1516773,7.2896381],[5.1516478,7.2894625],[5.1515914,7.2892564],[5.1515119,7.2890854],[5.1514585,7.2889658],[5.1513581,7.2887854],[5.1512673,7.2886082]]}},
    {"type":"Feature","id":"way/557077715","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1497229,7.2920061],[5.1497976,7.2919537],[5.1499743,7.2918966],[5.1502617,7.2918346],[5.1503062,7.2917952],[5.1503129,7.2917502],[5.1502835,7.2915714],[5.1502503,7.2913688],[5.1502047,7.2911507],[5.1501364,7.2907835],[5.150017,7.2901955],[5.1499781,7.2900106],[5.1498643,7.2894879]]}},
    {"type":"Feature","id":"way/557397369","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1539793,7.302328],[5.1541449,7.3022489],[5.1543005,7.3021717],[5.1545097,7.3019961],[5.15468,7.3018099],[5.1547511,7.3016689],[5.1547819,7.3015718],[5.1548074,7.3014707],[5.1548249,7.3013044],[5.1548912,7.3010051],[5.15491,7.3009662],[5.1549466,7.3009343],[5.1549945,7.3009246],[5.1550542,7.3009246],[5.15583,7.3010138],[5.1560245,7.3010177],[5.1562806,7.3009991],[5.1567446,7.3009147],[5.1568667,7.3009033],[5.1572214,7.3008728],[5.1574118,7.3008402],[5.1575372,7.3008176]]}},
    {"type":"Feature","id":"way/557397372","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1517631,7.3011794],[5.1521373,7.3015558],[5.1525148,7.3019695],[5.152553,7.3019954],[5.1526107,7.3020088]]}},
    {"type":"Feature","id":"way/557397374","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1526107,7.3020088],[5.1527193,7.3019043],[5.1529567,7.3017175],[5.1533389,7.3014547],[5.1534301,7.3015611],[5.1535629,7.3017327],[5.1538539,7.3021651],[5.1539142,7.3022402],[5.1539793,7.302328],[5.1540242,7.3024218],[5.1540658,7.3025814],[5.1540872,7.3027304],[5.1540846,7.3032439],[5.1540906,7.3033483],[5.1541033,7.3034128],[5.1541409,7.3034734],[5.1541838,7.3035239],[5.1542307,7.3035618],[5.1542965,7.3035931]]}},
    {"type":"Feature","id":"way/557397386","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1451029,7.307865],[5.1451481,7.3078534],[5.1454572,7.307746]]}},
    {"type":"Feature","id":"way/557397387","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.144041,7.3071082],[5.1445506,7.3069246],[5.1447825,7.3068423],[5.1448321,7.3068203]]}},
    {"type":"Feature","id":"way/557397388","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1442529,7.3077041],[5.1449744,7.3074527],[5.1450194,7.3074354]]}},
    {"type":"Feature","id":"way/557397389","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1445386,7.3082814],[5.1451601,7.3080421],[5.145208,7.3080228]]}},
    {"type":"Feature","id":"way/557397390","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1453902,7.3086832],[5.1454747,7.3089386],[5.1455484,7.3091886],[5.1456215,7.3094427],[5.1457281,7.3097752],[5.1458039,7.3100173]]}},
    {"type":"Feature","id":"way/557397392","properties":{"highway":"residential","oneway":"yes"},"geometry":{"type":"LineString","coordinates":[[5.144322,7.3051508],[5.1444071,7.3053695],[5.1445091,7.3055891],[5.1446096,7.3058511],[5.1446512,7.3059895],[5.1446821,7.3061158],[5.1447163,7.3062994],[5.1447833,7.3066093],[5.1448321,7.3068203],[5.1449449,7.3071969],[5.1449872,7.307335],[5.1450194,7.3074354],[5.1451481,7.3078534],[5.145208,7.3080228],[5.1452856,7.3082638],[5.1453828,7.308583],[5.1453902,7.3086832]]}},
    {"type":"Feature","id":"way/557397393","properties":{"highway":"residential","oneway":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1453902,7.3086832],[5.1453413,7.3085974],[5.1452407,7.3082847],[5.1451601,7.3080421],[5.1451029,7.307865],[5.1450053,7.3075445],[5.1449744,7.3074527],[5.1449436,7.3073516],[5.1448993,7.3072116],[5.1447825,7.3068423],[5.1447612,7.306757],[5.144729,7.3066219],[5.144664,7.306334],[5.1445319,7.3058817],[5.144334,7.3053988],[5.1442328,7.3051954]]}},
    {"type":"Feature","id":"way/557397396","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.141741,7.3081351],[5.1418155,7.3081098],[5.141969,7.308054],[5.1421809,7.3079662],[5.1424033,7.3078566],[5.142474,7.3078218],[5.142893,7.307613],[5.1433014,7.3074207]]}},
    {"type":"Feature","id":"way/557397399","properties":{"highway":"unclassified","name":"Yobel Road","surface":"ground"},"geometry":{"type":"LineString","coordinates":[[5.1411624,7.306628],[5.1413052,7.3070084],[5.1414822,7.3074234],[5.1415685,7.3076607],[5.141741,7.3081351],[5.1418621,7.3084337],[5.1419422,7.3086313],[5.1421528,7.3091314],[5.1422024,7.3092092],[5.1423117,7.3093363],[5.1424149,7.3094513],[5.1425014,7.3095797],[5.1425725,7.3097393],[5.1426188,7.3099029],[5.1426422,7.3100639],[5.1427093,7.3103845],[5.1427375,7.3105587],[5.1427764,7.310749],[5.1428743,7.3111194],[5.143058,7.3116109],[5.1432766,7.3121497],[5.1434724,7.3126578],[5.1435515,7.3128656],[5.14364,7.3131752],[5.1436752,7.3133605],[5.1437027,7.3134393],[5.143755,7.3135257],[5.1438734,7.3137126],[5.1439284,7.3138157],[5.1439726,7.3139301],[5.1440269,7.314121],[5.1441108,7.3142826],[5.1441537,7.3143571],[5.1442254,7.314554],[5.1443797,7.3149763],[5.1444071,7.3150949],[5.1444648,7.3153448],[5.1444528,7.3154106],[5.1444219,7.3154598],[5.144377,7.3154894],[5.144328,7.3155217],[5.144143,7.3156348],[5.1440712,7.3157026],[5.1440605,7.3157378],[5.1440665,7.3157711],[5.1442341,7.315956],[5.1442939,7.316067],[5.1443455,7.3161628],[5.1445386,7.3164422],[5.1446901,7.3166218],[5.1448698,7.3167574],[5.1450334,7.3168439],[5.1451914,7.3169151],[5.1452655,7.31696],[5.1453332,7.3170201],[5.1454492,7.3171645],[5.1455216,7.3172576],[5.1456504,7.3174571],[5.1457402,7.3175968],[5.1457543,7.3176347],[5.1457623,7.3176819],[5.1457362,7.3177584],[5.1456597,7.317943],[5.145647,7.3180005],[5.145641,7.3180816],[5.1456584,7.3182698],[5.1457335,7.318611],[5.1458186,7.3188498],[5.1458857,7.3189602],[5.1460098,7.319094]]}},
    {"type":"Feature","id":"way/557397400","properties":{"highway":"unclassified"},"geometry":{"type":"LineString","coordinates":[[5.1411624,7.306628],[5.1410987,7.3064704]]}},
    {"type":"Feature","id":"way/557397401","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1476077,7.3053982],[5.1482615,7.3055106],[5.1487242,7.3055877]]}},
    {"type":"Feature","id":"way/557402710","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1537781,7.3082588],[5.1541241,7.3082814],[5.1543213,7.3082907],[5.1544695,7.3082814],[5.1546492,7.3082561],[5.1548597,7.3082069],[5.1549844,7.3081923],[5.1551239,7.3082216],[5.1551078,7.3084185],[5.1550368,7.3089372],[5.1549107,7.3096968],[5.154786,7.3102793],[5.1547364,7.3104856],[5.1546759,7.310741],[5.1546458,7.3109086],[5.1546539,7.3109352],[5.154674,7.3109545],[5.1548289,7.3109711],[5.1550521,7.311005],[5.1553465,7.3110815],[5.1557127,7.3111689],[5.1560614,7.3112278],[5.1561036,7.3112225],[5.1561277,7.3111946],[5.1561807,7.3109498],[5.1561968,7.3107529],[5.156866,7.3108314]]}},
    {"type":"Feature","id":"way/557402712","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1501228,7.3078291],[5.1506553,7.3078011]]}},
    {"type":"Feature","id":"way/557402713","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1486471,7.308594],[5.1487222,7.3085528],[5.1487832,7.3085149],[5.1488375,7.308463],[5.1488636,7.3084384],[5.1489045,7.3084211],[5.1489341,7.3083978],[5.1489568,7.3083672],[5.1489776,7.3083147],[5.1490594,7.3081424],[5.1490829,7.3080998],[5.1491124,7.3080739],[5.1491681,7.308052],[5.1492351,7.3080433],[5.1492814,7.3080466],[5.1493585,7.3080613],[5.1494108,7.3080653],[5.1494564,7.3080573],[5.1495355,7.3080287],[5.1496475,7.3079735],[5.1497166,7.3079555],[5.1497876,7.3079515],[5.1499432,7.3079848],[5.1500804,7.3080032]]}},
    {"type":"Feature","id":"way/557402714","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1516243,7.3056589],[5.1515492,7.3058112],[5.1514969,7.3059748],[5.1514332,7.3062522],[5.1514151,7.3062775],[5.1513802,7.3062854],[5.1513286,7.3062808],[5.1507412,7.3061677]]}},
    {"type":"Feature","id":"way/557402715","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1516243,7.3056589],[5.15181,7.3056489],[5.1519991,7.3056117],[5.1522462,7.3055402]]}},
    {"type":"Feature","id":"way/557402716","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1521064,7.3042256],[5.151965,7.3043167],[5.1518804,7.3043779],[5.1518423,7.3044178],[5.1518235,7.3044511],[5.1518161,7.3044717],[5.1517993,7.3045741],[5.1517872,7.3047803],[5.1517551,7.3049559],[5.1517054,7.305178],[5.151625,7.3054381],[5.1516166,7.3054903],[5.1516193,7.3055478],[5.1516243,7.3056589]]}},
    {"type":"Feature","id":"way/557402717","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1495704,7.3048122],[5.1497863,7.3047364],[5.1508243,7.3042349],[5.1510429,7.3041032],[5.1511724,7.3040296],[5.1512347,7.3039941],[5.1513755,7.303905],[5.151519,7.3038199],[5.1516424,7.3037441],[5.1517551,7.3036749]]}},
    {"type":"Feature","id":"way/557402718","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1514855,7.3076057],[5.1515351,7.307591],[5.1515579,7.3075551],[5.1515767,7.3073915],[5.1516082,7.3071368],[5.151625,7.3069352]]}},
    {"type":"Feature","id":"way/557402719","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1502718,7.3074048],[5.1505347,7.3074501],[5.1507546,7.3074766],[5.1510376,7.3075232],[5.1512267,7.3075564],[5.1514855,7.3076057]]}},
    {"type":"Feature","id":"way/557402720","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1494282,7.3066213],[5.1498011,7.3067291],[5.1500693,7.3067849],[5.1504099,7.3068461],[5.1505963,7.3068594],[5.1507881,7.3068767],[5.1510309,7.3068967],[5.151177,7.3069073],[5.1513192,7.306918],[5.1515042,7.3069206],[5.151625,7.3069352],[5.1517162,7.3069379],[5.1518241,7.3069446],[5.1519381,7.3069532]]}},
    {"type":"Feature","id":"way/557402728","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1526107,7.3020088],[5.1527354,7.3020866],[5.1528628,7.302205],[5.1529956,7.302354],[5.1531029,7.3025003],[5.1533537,7.3028408]]}},
    {"type":"Feature","id":"way/557402729","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1517551,7.3036749],[5.1519696,7.3035312],[5.1521708,7.3033876],[5.1523291,7.3032751],[5.1523881,7.3032539],[5.1524437,7.3032552],[5.1524846,7.3032831],[5.1529043,7.3041644]]}},
    {"type":"Feature","id":"way/557402730","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1526496,7.3052253],[5.1529265,7.3050431],[5.1533939,7.3047351],[5.1534985,7.3046739],[5.1535776,7.3046193],[5.1536232,7.3045741],[5.1536554,7.3045129]]}},
    {"type":"Feature","id":"way/557402732","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1519381,7.3069532],[5.1519441,7.3066546],[5.151979,7.3063553],[5.1520099,7.3061717],[5.1520769,7.3059309],[5.1521936,7.305629],[5.1522462,7.3055402],[5.152313,7.305468],[5.1523787,7.3054241],[5.1524967,7.3053496],[5.1525745,7.3052951],[5.1526496,7.3052253]]}},
    {"type":"Feature","id":"way/557402734","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1506822,7.3020135],[5.1511113,7.3026892],[5.1515914,7.3034248],[5.1517551,7.3036749],[5.1519656,7.3039941],[5.1521064,7.3042256],[5.1522379,7.3044491],[5.1525088,7.3049466],[5.1526496,7.3052253]]}},
    {"type":"Feature","id":"way/557402740","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1536554,7.3045129],[5.1536943,7.3044145],[5.1537506,7.3042562],[5.1538137,7.3040261],[5.1538499,7.3038797],[5.1539022,7.3037467],[5.153988,7.3036004],[5.1541033,7.3034128]]}},
    {"type":"Feature","id":"way/557407630","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.135525,7.3099176],[5.1357463,7.3104723],[5.1360494,7.3111374],[5.1361553,7.3113608]]}},
    {"type":"Feature","id":"way/557407631","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1361929,7.3095797],[5.1360615,7.3096502],[5.1357087,7.3098231],[5.1356015,7.3098843],[5.135525,7.3099176],[5.1353239,7.3099628],[5.1350442,7.3100393],[5.1349208,7.3100745],[5.1347505,7.3101477],[5.134662,7.3101916],[5.1345165,7.3102827]]}},
    {"type":"Feature","id":"way/557407632","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1359468,7.3090809],[5.1361929,7.3095797],[5.1364088,7.3100093],[5.1365778,7.310435],[5.1368165,7.311015]]}},
    {"type":"Feature","id":"way/557407633","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1380194,7.3078051],[5.1381415,7.3081337],[5.1383621,7.3087483],[5.1384392,7.3088747],[5.1384761,7.3088953],[5.1385304,7.308892],[5.1387188,7.3087924],[5.1389154,7.3086885],[5.1392497,7.3085193],[5.1394544,7.3084158],[5.1394983,7.3083951],[5.1397106,7.3082948],[5.1406212,7.3078518]]}},
    {"type":"Feature","id":"way/557407636","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.144729,7.3066219],[5.1447833,7.3066093],[5.145004,7.3065322],[5.145187,7.3064923],[5.1452889,7.3064856],[5.145411,7.3065401],[5.1456155,7.3066559],[5.1461754,7.3069618]]}},
    {"type":"Feature","id":"way/557407637","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.148694,7.3045015],[5.148869,7.3048528],[5.148875,7.30489],[5.148873,7.3049299],[5.1487852,7.3052233],[5.1487456,7.3053895],[5.1487242,7.3055877],[5.1486799,7.3058125],[5.1486424,7.3060094],[5.1485955,7.3062129]]}},
    {"type":"Feature","id":"way/557407638","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1476077,7.3053982],[5.1475621,7.3054414],[5.1474005,7.3055465],[5.1473173,7.3055678],[5.1472681,7.3055894],[5.1472211,7.3056246],[5.1470853,7.3057992],[5.147033,7.3058917],[5.1470076,7.3059316],[5.1469244,7.306034],[5.1469097,7.3060646],[5.1468822,7.3061438],[5.1468627,7.3061784],[5.1467916,7.3062528],[5.1466931,7.306348],[5.1466146,7.3064391],[5.1465543,7.3065375],[5.1465154,7.3066266],[5.1464631,7.3067078],[5.1462534,7.3069549],[5.1462351,7.3069663],[5.1462084,7.3069683],[5.1461754,7.3069618]]}},
    {"type":"Feature","id":"way/557407639","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.148694,7.3045015],[5.1485096,7.3046034],[5.1481864,7.3048029],[5.1480214,7.3049146],[5.1478893,7.3049884]]}},
    {"type":"Feature","id":"way/557407640","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1492257,7.3065482],[5.1493987,7.3058644],[5.1494638,7.3056163],[5.1495516,7.305178],[5.149565,7.3049466],[5.1495704,7.3048122],[5.149557,7.3043879],[5.1495516,7.3038877],[5.1495503,7.3035099],[5.1495308,7.3027691]]}},
    {"type":"Feature","id":"way/569683660","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1507412,7.3061677],[5.1507589,7.3060376],[5.150798,7.3058061],[5.1508474,7.3055699],[5.1509786,7.3050144],[5.1510456,7.304775],[5.1510456,7.3046938],[5.1510228,7.304614],[5.1508959,7.3043649],[5.1508243,7.3042349]]}},
    {"type":"Feature","id":"way/569683672","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1504099,7.3068461],[5.1503228,7.3072345],[5.1502718,7.3074048],[5.1502289,7.307482],[5.1501819,7.307599],[5.1501565,7.3076722],[5.1501384,7.3077547],[5.1501228,7.3078291],[5.1501111,7.3078783],[5.1500804,7.3080032],[5.1500173,7.3084029],[5.1500161,7.3084502],[5.1500274,7.3085171]]}},
    {"type":"Feature","id":"way/569683673","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1494282,7.3066213],[5.1493733,7.3067397],[5.1493004,7.306921],[5.1491584,7.3072401],[5.1490936,7.307341],[5.1489964,7.3074314],[5.1488784,7.3074979],[5.1488315,7.3075325],[5.148802,7.3075831],[5.1487702,7.3076582],[5.1487434,7.3077071],[5.1485058,7.3079514],[5.1483353,7.3080326],[5.1482763,7.3080385],[5.148226,7.3080232],[5.1481692,7.3079846],[5.1480563,7.3078677],[5.1479936,7.3077888],[5.1479774,7.3077489],[5.1479852,7.3076989]]}},
    {"type":"Feature","id":"way/581991077","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1536487,7.2921178],[5.1541623,7.2928268],[5.1544507,7.2931886],[5.1545714,7.2933442],[5.154731,7.2935425],[5.1548879,7.2937839]]}},
    {"type":"Feature","id":"way/581991079","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1527619,7.2948315],[5.1530703,7.2946329],[5.1534247,7.2943965],[5.1534985,7.2943606],[5.1535582,7.2943353]]}},
    {"type":"Feature","id":"way/582039238","properties":{"highway":"tertiary","surface":"unpaved"},"geometry":{"type":"LineString","coordinates":[[5.1124594,7.3079842],[5.1124795,7.307911],[5.1126619,7.3074926],[5.1127967,7.3072093],[5.1130307,7.3067783],[5.1131507,7.3065728],[5.1132714,7.3064132],[5.1133747,7.3062835],[5.1134364,7.3061638],[5.1134921,7.3059503],[5.113547,7.3057607],[5.1135705,7.3055259],[5.1135725,7.3053557],[5.1135705,7.3052326],[5.1135497,7.3051029],[5.1134874,7.3049253],[5.1134129,7.3046806],[5.1133935,7.3045841],[5.1133787,7.3044624],[5.113372,7.3041152],[5.1133305,7.303635],[5.1133043,7.3034687],[5.113254,7.3033078],[5.1129945,7.302636],[5.1126016,7.3017188],[5.1124111,7.301379],[5.1122408,7.3010457],[5.1121898,7.3008868],[5.112167,7.3007917],[5.112153,7.3005728],[5.1121429,7.3004046],[5.1121744,7.2999875],[5.1122053,7.2998312],[5.1122421,7.2996796],[5.1124024,7.2993131],[5.1124648,7.2991814],[5.1125043,7.2990856],[5.112554,7.2988695],[5.1125784,7.298788],[5.1126187,7.2987351],[5.112675,7.2987039],[5.1128439,7.2986404],[5.1132835,7.2984753],[5.1135605,7.2983713],[5.1138662,7.2982542],[5.1141116,7.298205],[5.1143799,7.2981837],[5.1146092,7.2981851],[5.1148814,7.2982516],[5.1151175,7.298358],[5.1154139,7.2985136],[5.1156378,7.2985961],[5.1157934,7.2986254],[5.1159651,7.2986227],[5.116189,7.2986121],[5.1164519,7.2985801],[5.1168462,7.298499],[5.116925,7.298474],[5.1174604,7.2983048],[5.117793,7.2982223],[5.1180438,7.2981811],[5.1181993,7.2981611],[5.1184649,7.2981372],[5.1186473,7.2981624],[5.1188277,7.2982031],[5.1189423,7.298229],[5.1191274,7.2982755],[5.1192642,7.2982941],[5.1195217,7.2982995],[5.1198177,7.2983256],[5.1203652,7.298374],[5.1207421,7.2983859],[5.1208735,7.2983846],[5.1210036,7.2983527],[5.1213778,7.2982542],[5.1223769,7.2979736],[5.122688,7.2978764],[5.1228208,7.2978086],[5.1229254,7.2977354],[5.1230702,7.2975931],[5.1235128,7.2970317],[5.1237837,7.2967656],[5.1238984,7.2964577],[5.1241424,7.2959822],[5.1242067,7.2958561],[5.1245414,7.2951295],[5.1246474,7.2950457],[5.1247989,7.2948581],[5.1249303,7.2947317],[5.1253622,7.2942635],[5.1260287,7.2935425],[5.1265929,7.2929769],[5.1266724,7.2928973],[5.1270916,7.2924791],[5.1275911,7.2919808],[5.1279733,7.2915405],[5.1282388,7.2912265],[5.1285795,7.2908514],[5.1287424,7.2906765],[5.1292339,7.2901876],[5.1296027,7.2898085],[5.1297959,7.2895956],[5.1303242,7.2889797],[5.1306233,7.288693],[5.1310358,7.2882243],[5.1311765,7.2880645],[5.1313086,7.2879288],[5.1314461,7.2878124],[5.1316426,7.2876727],[5.1318384,7.2875836],[5.1320228,7.2875304],[5.1321817,7.2874772],[5.1323507,7.287394],[5.1325566,7.2872994],[5.132643,7.2872597],[5.1327745,7.2872244],[5.1330091,7.2871841],[5.1330957,7.2871692],[5.1332519,7.2871406],[5.1333671,7.2870932],[5.1334135,7.2870741],[5.1338272,7.2868287],[5.1339278,7.2867335],[5.1341516,7.2864924],[5.1342121,7.2864429],[5.1343392,7.2862257],[5.1345313,7.2859188],[5.1347297,7.2855528],[5.1347619,7.2854757],[5.1348599,7.2852443],[5.134888,7.2851352],[5.1349497,7.2850381],[5.1350154,7.2849197],[5.1350623,7.2848492],[5.1351531,7.2847673],[5.135218,7.2846908],[5.1355974,7.2843365],[5.1358669,7.2841229],[5.1359435,7.284063],[5.1361595,7.2839167],[5.1363791,7.2838153],[5.1366154,7.2837185],[5.136893,7.2835974],[5.1370768,7.2835242],[5.1374254,7.2833327],[5.137523,7.2832695],[5.1377054,7.2831512],[5.1378586,7.2830361],[5.1379417,7.2829615],[5.1381452,7.2827457],[5.1382445,7.2826371],[5.1383374,7.2825585],[5.1385949,7.282339],[5.1386874,7.2822485],[5.1389999,7.2819505],[5.1392185,7.2817257],[5.1394384,7.2815341],[5.1398166,7.2812428]]}},
    {"type":"Feature","id":"way/584749915","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1240297,7.3039948],[5.1238722,7.3043586],[5.1236629,7.3049373],[5.1235503,7.3051222],[5.1233505,7.3053456],[5.1230472,7.3056431],[5.1226276,7.3060546],[5.1222467,7.3064318],[5.1220413,7.3066153],[5.1219698,7.3066792],[5.1216741,7.3069552],[5.1216446,7.3070071],[5.1216325,7.3070776],[5.1216298,7.3071454],[5.1216727,7.3076961],[5.1216821,7.3078345],[5.1216593,7.3080353],[5.1216097,7.3082349],[5.1215883,7.3083705],[5.1215976,7.3085036],[5.1216339,7.3086552],[5.1216982,7.3088015],[5.1218015,7.309013],[5.1219021,7.3092072],[5.1219799,7.309311],[5.1220818,7.3094148],[5.1221958,7.3095518],[5.1222602,7.3096702],[5.1223446,7.3098803],[5.1224533,7.3101344],[5.1224707,7.3102076],[5.122472,7.3102887],[5.1224395,7.3104699],[5.1224359,7.3105681],[5.1224372,7.3106651],[5.1224935,7.311015],[5.1225056,7.3110788],[5.1225317,7.3111653],[5.1225713,7.3112338],[5.1226236,7.311283],[5.1226477,7.3113007],[5.1226893,7.3113253]]}},
    {"type":"Feature","id":"way/592569669","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1423117,7.3093363],[5.1425387,7.3093157],[5.1426543,7.3093027],[5.1427998,7.3092964],[5.1428575,7.3092944],[5.1428971,7.3092884],[5.1429635,7.3092738],[5.1431123,7.3092086],[5.1432666,7.3091793],[5.143347,7.3091706],[5.143406,7.3091418],[5.1434805,7.3091414],[5.1435367,7.3091005],[5.1437768,7.3089631],[5.1438841,7.308878],[5.1439934,7.3088055],[5.1440249,7.3087929],[5.1440863,7.3087776],[5.1441238,7.3087676],[5.1441327,7.3087635],[5.1441409,7.3087546],[5.1441637,7.3087207]]}},
    {"type":"Feature","id":"way/592575041","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1529479,7.3071534],[5.1529956,7.3070463],[5.153009,7.3070024],[5.153066,7.3067317],[5.1531284,7.3065249],[5.1532249,7.3062941],[5.1532289,7.3062502],[5.1532182,7.3061917]]}},
    {"type":"Feature","id":"way/592575042","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1529306,7.3106949],[5.1529543,7.3106053],[5.1529694,7.3104872],[5.1529835,7.3099362],[5.1529902,7.3097007],[5.1529902,7.309589],[5.152958,7.309464],[5.1528548,7.309182],[5.1527461,7.3089319],[5.1527327,7.3088734],[5.1527327,7.3088015],[5.1527327,7.3083293],[5.1527689,7.307873],[5.1527797,7.3077906],[5.1528145,7.3076775],[5.1528695,7.3074913],[5.1529165,7.3072971],[5.1529479,7.3071534]]}},
    {"type":"Feature","id":"way/593962787","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1397106,7.3082948],[5.1396972,7.3083799],[5.1396972,7.3084251],[5.1397522,7.3085475],[5.1399922,7.3090037]]}},
    {"type":"Feature","id":"way/593962788","properties":{"access":"permissive","bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"1","motor_vehicle":"yes","oneway":"no","surface":"unpaved"},"geometry":{"type":"LineString","coordinates":[[5.1401989,7.3068904],[5.1404589,7.3074687],[5.1406212,7.3078518],[5.1407915,7.3081903],[5.1409303,7.308477],[5.1409303,7.3085069],[5.1409163,7.3085308],[5.1408331,7.3085727],[5.1404878,7.308745],[5.1402752,7.3088614],[5.1399922,7.3090037],[5.1398025,7.3090908],[5.1396959,7.3091447],[5.13959,7.3092042],[5.1393244,7.3093536],[5.1386122,7.3096622],[5.1381791,7.3098564]]}},
    {"type":"Feature","id":"way/593977685","properties":{"highway":"unclassified","surface":"sand"},"geometry":{"type":"LineString","coordinates":[[5.1333095,7.3098232],[5.1333685,7.3101158],[5.1334047,7.3102807],[5.1334235,7.3104137],[5.1334168,7.3105108],[5.1333846,7.3106133],[5.1333587,7.3106553],[5.1333189,7.3107197],[5.1330346,7.3111227],[5.1327583,7.3115351],[5.1324305,7.3119927],[5.1320234,7.3125541],[5.1319468,7.3126511],[5.1317096,7.3129597],[5.1313891,7.3133441],[5.1310565,7.3137539],[5.1307614,7.3140984],[5.1305053,7.3144336],[5.1301445,7.3148553],[5.1298454,7.3152171],[5.1297496,7.315382],[5.1297301,7.3154538],[5.1297281,7.3155017],[5.129743,7.315534],[5.1297529,7.3155556],[5.1298012,7.3155875],[5.1298548,7.3155942],[5.1302545,7.3154991],[5.1304302,7.3154672],[5.1305535,7.3154632],[5.1309063,7.3154844]]}},
    {"type":"Feature","id":"way/593999311","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1357932,7.3087657],[5.1359468,7.3090809]]}},
    {"type":"Feature","id":"way/593999312","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1359468,7.3090809],[5.1359582,7.3090137],[5.1359783,7.3089472],[5.1360252,7.3088481],[5.1360776,7.3087822],[5.1361124,7.3087191],[5.1361768,7.3086002]]}},
    {"type":"Feature","id":"way/594010657","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1282517,7.305339],[5.1283696,7.3052851]]}},
    {"type":"Feature","id":"way/594010659","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1283696,7.3052851],[5.1282448,7.3052439]]}},
    {"type":"Feature","id":"way/594033269","properties":{"highway":"residential","maxspeed":"40","name":"Middle Belt Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1370268,7.3036237],[5.136956,7.3035006],[5.1368768,7.3033968]]}},
    {"type":"Feature","id":"way/594033270","properties":{"highway":"residential","maxspeed":"40","name":"Middle Belt Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.13685,7.3036257],[5.1368768,7.3033968]]}},
    {"type":"Feature","id":"way/594056219","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"permissive","lanes":"2","motor_vehicle":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1311396,7.2950909],[5.1312911,7.2950563],[5.1316331,7.2950084],[5.1319966,7.2949791],[5.1322661,7.2949592],[5.1325089,7.2949352],[5.1326644,7.2949286],[5.1327543,7.2949179],[5.1329099,7.2949006],[5.1331472,7.2948847],[5.1333645,7.2948594]]}},
    {"type":"Feature","id":"way/594056220","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.139647,7.3067722],[5.139735,7.3067417]]}},
    {"type":"Feature","id":"way/594056221","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"1","maxspeed":"40","motor_vehicle":"yes","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1397274,7.3017361],[5.1396683,7.3017526],[5.1394303,7.3018192],[5.139146,7.3018924]]}},
    {"type":"Feature","id":"way/594056222","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"2","maxspeed":"40","motor_vehicle":"yes","name":"Senate Building Road","oneway":"no","surface":"asphalt","wheelchair":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1390676,7.3015021],[5.1393609,7.3014568],[5.139574,7.301424],[5.1396371,7.3014143]]}},
    {"type":"Feature","id":"way/594056223","properties":{"highway":"residential","name":"Senate building Road"},"geometry":{"type":"LineString","coordinates":[[5.1390421,7.3010816],[5.1390619,7.3014076],[5.1390676,7.3015021],[5.1390749,7.3015842],[5.1390883,7.3017354],[5.139146,7.3018924],[5.1392681,7.3023646]]}},
    {"type":"Feature","id":"way/594090809","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1507412,7.3061677],[5.1503241,7.3060972],[5.1499137,7.3060001],[5.1493987,7.3058644]]}},
    {"type":"Feature","id":"way/594090810","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1485955,7.3062129],[5.1485883,7.3062475],[5.1485967,7.3062784],[5.1486283,7.3063061],[5.1488449,7.3064032],[5.1492257,7.3065482],[5.1494282,7.3066213]]}},
    {"type":"Feature","id":"way/594098001","properties":{"access":"yes","bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"2","motor_vehicle":"yes","name":"Malu Road","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.1393713,7.3049187],[5.1395571,7.3048676],[5.1396496,7.3048421],[5.1397689,7.3048002],[5.1398803,7.304751],[5.1399782,7.3046785],[5.1400848,7.3045708],[5.1401874,7.3044311],[5.1403772,7.3041664],[5.1405136,7.3039393],[5.1405984,7.3037773],[5.1407634,7.3035153]]}},
    {"type":"Feature","id":"way/594098003","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1529479,7.3071534],[5.1528957,7.3071308],[5.1528219,7.3071042],[5.1524283,7.3069971],[5.1522996,7.3069685],[5.1521896,7.3069579],[5.1519381,7.3069532]]}},
    {"type":"Feature","id":"way/594098005","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1511115,7.3016975],[5.1503053,7.3007604],[5.1501357,7.3006054],[5.1500431,7.3004976],[5.1499942,7.3004085],[5.1499245,7.3002342],[5.1498929,7.3001691],[5.1498742,7.3001099],[5.1498681,7.3000247],[5.1498768,7.2998997],[5.1498936,7.2998412],[5.1499191,7.2998052],[5.1499935,7.299756]]}},
    {"type":"Feature","id":"way/594098007","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1414822,7.3074234],[5.1420669,7.3071428],[5.1425859,7.3069246],[5.1430473,7.3067051]]}},
    {"type":"Feature","id":"way/594098008","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1444071,7.3053695],[5.144334,7.3053988]]}},
    {"type":"Feature","id":"way/594098010","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1446096,7.3058511],[5.1445319,7.3058817]]}},
    {"type":"Feature","id":"way/594098011","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1511724,7.3040296],[5.1513192,7.3043347],[5.1513809,7.3043999],[5.1514667,7.3044531],[5.1515619,7.3044863],[5.151627,7.304487],[5.15168,7.3044823],[5.1518235,7.3044511]]}},
    {"type":"Feature","id":"way/594098012","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1477291,7.2975824],[5.1477317,7.2976542],[5.1477411,7.2976749],[5.1477619,7.2976868],[5.147896,7.2976935],[5.1479772,7.2977055],[5.1480818,7.2977434],[5.1482031,7.2978119],[5.1482736,7.2978558],[5.1483124,7.297883],[5.1483547,7.2979516]]}},
    {"type":"Feature","id":"way/594098013","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.148181,7.3035033],[5.1483929,7.3039143],[5.1485686,7.3042575],[5.148694,7.3045015]]}},
    {"type":"Feature","id":"way/594098014","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1478893,7.3049884],[5.147709,7.3051115],[5.1476727,7.3051641],[5.1476593,7.3052086],[5.1476278,7.3053549],[5.1476077,7.3053982]]}},
    {"type":"Feature","id":"way/594111627","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1229079,7.2935371],[5.1229542,7.2935478],[5.1229823,7.2935671],[5.1230172,7.2936243],[5.1230554,7.2937001],[5.1230856,7.2937307],[5.1231252,7.293756],[5.1232841,7.2938305],[5.1233706,7.293875],[5.1234048,7.2939023],[5.123449,7.2939661],[5.1234906,7.2940386],[5.1235241,7.2940899],[5.1235718,7.2941311],[5.1236442,7.2941969],[5.1236992,7.2942794],[5.1237595,7.2943353],[5.1238091,7.2943586],[5.12385,7.2943612],[5.1240277,7.2943665],[5.1240847,7.2943805],[5.1242557,7.2944404],[5.1243529,7.2944982],[5.1247989,7.2948581]]}},
    {"type":"Feature","id":"way/594111635","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1270593,7.2962508],[5.1271485,7.2961404],[5.1271525,7.2960872],[5.1271324,7.2960101],[5.1270935,7.2959209],[5.1270734,7.2958664],[5.127064,7.2958012],[5.12706,7.295736],[5.1270801,7.2956589],[5.1271082,7.2955777],[5.1271129,7.2955458],[5.1271076,7.2954893],[5.1270861,7.2953995],[5.1270197,7.2952252]]}},
    {"type":"Feature","id":"way/594111637","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1245414,7.2951295],[5.1248418,7.2953569],[5.1249209,7.2954274],[5.1250563,7.2955591],[5.1251475,7.2956402],[5.1253554,7.2957959],[5.1257108,7.2961045],[5.1258503,7.2962282],[5.1260099,7.2963519],[5.1262124,7.2965248],[5.1262673,7.2965581],[5.1264122,7.2966186],[5.1264484,7.2966239],[5.1264806,7.2966193],[5.1265409,7.29659],[5.1266885,7.296463],[5.1267575,7.2964104],[5.1269158,7.2963146],[5.1269969,7.2962654],[5.1270311,7.2962521],[5.1270593,7.2962508],[5.1271169,7.2962614],[5.1271806,7.2962967],[5.1272544,7.2963359],[5.1272926,7.2963479],[5.1273443,7.2963546],[5.1274013,7.2963439],[5.1275092,7.2962974],[5.1275803,7.2962521],[5.1276849,7.2961697],[5.127813,7.2960705],[5.1278371,7.296032],[5.1278492,7.2959754],[5.1278458,7.2959076],[5.1278224,7.2957952],[5.1278237,7.2957387],[5.1278425,7.2957007],[5.1278753,7.2956635],[5.127927,7.2956262],[5.1281328,7.2955205],[5.1283025,7.2954134],[5.1284138,7.2953183],[5.128458,7.2952864],[5.128619,7.2951766],[5.1287209,7.2950961],[5.1288718,7.2949305],[5.1289811,7.2948042],[5.1290126,7.2947736],[5.1290575,7.2947416],[5.1291487,7.2946904],[5.1291896,7.2946618],[5.1292412,7.2946099],[5.1294149,7.2944616],[5.1294471,7.2944377],[5.1294914,7.294435],[5.1295463,7.2944603]]}},
    {"type":"Feature","id":"way/594111641","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1287424,7.2906765],[5.128613,7.2905441],[5.1283568,7.2903532],[5.1283132,7.2903126],[5.1278928,7.2898217],[5.1277741,7.2897113],[5.127748,7.2896973],[5.1277184,7.2896967],[5.1276715,7.2897186],[5.1269309,7.2903635],[5.1269118,7.2903688],[5.126886,7.2903601],[5.1267515,7.2902168],[5.1266778,7.2901157],[5.126604,7.2899641],[5.1265651,7.2899361]]}},
    {"type":"Feature","id":"way/594121597","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1285795,7.2908514],[5.1290891,7.2912597],[5.1292138,7.2913582],[5.1292483,7.2913918],[5.1292932,7.2914476],[5.1293807,7.2915404],[5.1296014,7.2917812]]}},
    {"type":"Feature","id":"way/594376887","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1533389,7.3014547],[5.1538257,7.3011169]]}},
    {"type":"Feature","id":"way/594381665","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1541007,7.3046206],[5.1538713,7.3046207],[5.1537278,7.304626],[5.1535776,7.3046193]]}},
    {"type":"Feature","id":"way/594486732","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1355376,7.2911134],[5.1358206,7.2911416],[5.1359181,7.2911562],[5.1359729,7.2911549],[5.1360248,7.291147],[5.1360327,7.2911579],[5.1360399,7.2911727],[5.1360432,7.291191],[5.1360482,7.2912349],[5.1361862,7.2913036],[5.1363659,7.2913954],[5.1364004,7.2914114],[5.1364383,7.2914167],[5.1364946,7.2914184],[5.1365137,7.2914234],[5.1366214,7.2914706],[5.1366365,7.2914726],[5.1366542,7.2914653],[5.1366539,7.2914407],[5.1366519,7.29141],[5.1366597,7.2913567],[5.1366655,7.2912687],[5.1366597,7.2912032],[5.1366584,7.2910889],[5.1366602,7.2909634],[5.1366569,7.2908376],[5.1366675,7.2906933],[5.1366708,7.2905901],[5.1366682,7.290515],[5.1367124,7.2904695],[5.1368524,7.2903371],[5.1368854,7.2903163],[5.1369032,7.2903131],[5.1369301,7.2902959],[5.1371106,7.2901593],[5.1371239,7.290146],[5.137151,7.290118],[5.1372362,7.2900148],[5.1372574,7.2899798],[5.1372781,7.2899654],[5.1373047,7.2899515],[5.1373327,7.2899456],[5.1373867,7.2899123],[5.1374904,7.2898306],[5.1375387,7.2897616],[5.1376165,7.2897014],[5.1377748,7.2896111],[5.137844,7.2895871],[5.1378847,7.2895749],[5.1379314,7.289549],[5.1380404,7.2894478],[5.1381428,7.2893297],[5.1381989,7.2892598],[5.1383122,7.2891212],[5.1383652,7.2890628],[5.1384142,7.2890106],[5.1384446,7.2889933],[5.1385133,7.2889712],[5.1385584,7.2889451],[5.1386061,7.2889025],[5.1386782,7.2888456]]}},
    {"type":"Feature","id":"way/594486733","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1346255,7.2892296],[5.1346333,7.2892821],[5.1346442,7.2893044],[5.1346567,7.2893229],[5.1349077,7.2896115],[5.1351525,7.2898456],[5.1355877,7.290309],[5.1355775,7.2903274],[5.135524,7.2903645],[5.135525,7.2904882],[5.1355264,7.2905421],[5.1355572,7.2907183],[5.1355376,7.2911134]]}},
    {"type":"Feature","id":"way/594486736","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1326644,7.2949286],[5.1325129,7.2944484],[5.1323077,7.2939575],[5.1323064,7.2939056],[5.1323198,7.2938378],[5.132356,7.2937713],[5.1324244,7.2937127],[5.1328415,7.2934999],[5.1331915,7.2933256]]}},
    {"type":"Feature","id":"way/594486740","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1350623,7.2848492],[5.1351763,7.284941],[5.1352293,7.2850021],[5.1353045,7.2850634],[5.1354486,7.2852024],[5.1355707,7.2853141],[5.1356833,7.2853946],[5.1357523,7.2855163],[5.1358627,7.2856392],[5.1359724,7.2857538],[5.1361885,7.2859702],[5.1365013,7.2863381],[5.136599,7.2864397],[5.1368252,7.2867378],[5.1370083,7.2870046],[5.1371004,7.2871165],[5.1372184,7.287228],[5.13742,7.2874299],[5.1375418,7.2875401],[5.1377489,7.2877619],[5.1380904,7.2881544],[5.1381419,7.2882522],[5.1381764,7.2883052],[5.1384996,7.2886392],[5.1385935,7.2887398],[5.1386782,7.2888456],[5.1388052,7.2890248],[5.1388763,7.289064],[5.1389614,7.2891],[5.1390492,7.289165],[5.1391673,7.2893351],[5.1392761,7.2894768],[5.1394739,7.2897002],[5.1395325,7.2898024],[5.139718,7.290071],[5.1397743,7.2901882],[5.1397984,7.2902328],[5.1398097,7.2902929],[5.1398323,7.2904362],[5.139849,7.2906263],[5.1398796,7.2907436],[5.1399143,7.2908018],[5.1399561,7.2908342]]}},
    {"type":"Feature","id":"way/594492964","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.13144,7.2987038],[5.13137,7.2983931],[5.1313612,7.2982977],[5.131355,7.2982011],[5.1312762,7.2974401],[5.1311023,7.2958354],[5.1311065,7.2951926],[5.1311396,7.2950909]]}},
    {"type":"Feature","id":"way/594492967","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1328683,7.2926698],[5.1331137,7.2925075],[5.1335737,7.2922455],[5.1339133,7.2920243],[5.1339274,7.2920053],[5.1339331,7.2919787]]}},
    {"type":"Feature","id":"way/594492968","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1340955,7.2948035],[5.1338802,7.2943885],[5.133848,7.2943107],[5.1338171,7.2942056],[5.133797,7.294163],[5.1337481,7.2941118],[5.1336776,7.2940672],[5.1336133,7.2940193],[5.1335476,7.2939449],[5.1334557,7.2938052],[5.1333149,7.293587],[5.1331915,7.2933256],[5.1328683,7.2926698]]}},
    {"type":"Feature","id":"way/594492970","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1360474,7.286916],[5.1360292,7.28697],[5.1360269,7.2869906],[5.1360325,7.2870105],[5.1360861,7.2871019],[5.1361778,7.2872411],[5.1363437,7.2874154],[5.1363754,7.2874558],[5.1363854,7.2874848],[5.1363879,7.2875322],[5.1363859,7.2875855],[5.1363796,7.2876119],[5.1363546,7.2876536],[5.1362402,7.2877633],[5.1359503,7.2880251],[5.135437,7.2884694],[5.1353637,7.2885241],[5.135295,7.2885597],[5.1352213,7.2886072],[5.1351682,7.2886467],[5.1350493,7.2887584],[5.1349908,7.288811],[5.1348178,7.2889603],[5.1347652,7.2890332],[5.1347153,7.2891503],[5.1346955,7.2891673],[5.1346412,7.2892048],[5.1346255,7.2892296]]}},
    {"type":"Feature","id":"way/594492972","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1399561,7.2908342],[5.1400067,7.290851],[5.1400785,7.2908617],[5.1402223,7.2908638],[5.1404026,7.2908716],[5.1406809,7.290866],[5.1408793,7.2908567],[5.1410362,7.2908358],[5.1412043,7.2908171],[5.1413432,7.2907865],[5.1417724,7.2907621],[5.1418338,7.2907605],[5.1419156,7.2907449],[5.1423964,7.2907128],[5.1424963,7.2906962],[5.1425666,7.29067]]}},
    {"type":"Feature","id":"way/594492974","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1425666,7.29067],[5.1425933,7.291009],[5.1426061,7.2911949],[5.1426329,7.2913262],[5.1426385,7.2914437],[5.1426448,7.2915755],[5.1426302,7.2916864],[5.1426317,7.2917716],[5.1426483,7.2918279],[5.1426687,7.2918608],[5.1427006,7.2919335],[5.1427365,7.2920241],[5.142765,7.2920963]]}},
    {"type":"Feature","id":"way/594492989","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1360482,7.2912349],[5.1360417,7.291305],[5.1360285,7.2913763],[5.1360054,7.2914576],[5.1360013,7.2914779],[5.1359986,7.2914966],[5.1359919,7.2915694],[5.1359919,7.2916877],[5.135999,7.2917962],[5.1360302,7.2919343],[5.1360387,7.2919833],[5.1360449,7.2920502],[5.1360468,7.2921053],[5.1360418,7.2922932],[5.1360282,7.2923849],[5.1360192,7.2925134],[5.1360193,7.2925475],[5.1360382,7.2927121],[5.13605,7.2928127],[5.1360657,7.2928978],[5.1360968,7.2930422],[5.1361181,7.2931313],[5.1361344,7.2932195],[5.1361643,7.2933438],[5.1361769,7.2934702],[5.1361803,7.2935251],[5.1361943,7.2935736],[5.1362335,7.2936549]]}},
    {"type":"Feature","id":"way/594492990","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1328683,7.2926698],[5.132639,7.2927576],[5.1325692,7.2927882],[5.1324244,7.292876],[5.132352,7.2929265],[5.1322205,7.2929891],[5.1319523,7.2931128],[5.1317444,7.2932259],[5.1314668,7.2933775],[5.1312254,7.2934919],[5.1310377,7.2935784],[5.1309022,7.2936396],[5.1308647,7.2936668],[5.1308077,7.293718],[5.1307131,7.2938258],[5.1306179,7.2939176],[5.1305656,7.2939794],[5.1304986,7.2940998]]}},
    {"type":"Feature","id":"way/594492991","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1329193,7.2917599],[5.1326323,7.2919209],[5.1322487,7.2921071],[5.1321213,7.2921663],[5.1320231,7.2922275]]}},
    {"type":"Feature","id":"way/594492992","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1326872,7.2916229],[5.1326323,7.2919209]]}},
    {"type":"Feature","id":"way/594492993","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1326872,7.2916229],[5.1329193,7.2917599]]}},
    {"type":"Feature","id":"way/594492994","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1310358,7.2882243],[5.1314601,7.2886458],[5.131727,7.288981],[5.1319121,7.2892111],[5.1319724,7.2893109],[5.1320086,7.2894213],[5.132073,7.2896927],[5.1321441,7.2900013],[5.1322259,7.2902687],[5.1323023,7.2904975],[5.1323426,7.2906691],[5.1323788,7.2907715],[5.1324485,7.2908833],[5.1324794,7.2909618],[5.1324901,7.2910032],[5.1325142,7.2910961],[5.1325692,7.2912704],[5.1326229,7.2913914],[5.1326604,7.2914859],[5.1326872,7.2916229]]}},
    {"type":"Feature","id":"way/594492995","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1339331,7.2919787],[5.1339288,7.2919442],[5.1338178,7.2917439],[5.1336193,7.2914127],[5.1335911,7.2913914],[5.1335603,7.2913888],[5.1335241,7.2914021],[5.1332197,7.291583],[5.1329193,7.2917599]]}},
    {"type":"Feature","id":"way/594492997","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1381196,7.2935549],[5.1381831,7.2935525],[5.1382757,7.2935537],[5.1383712,7.2935582],[5.1384708,7.29357],[5.1385607,7.2935693],[5.1386526,7.2935686],[5.1387766,7.2935633],[5.1388246,7.2935416],[5.1388465,7.2935081],[5.1388531,7.2934513],[5.1388416,7.2934001],[5.1388407,7.293314],[5.1388318,7.29326],[5.1388195,7.2931758],[5.1388327,7.2929114],[5.1388284,7.2928729],[5.1388416,7.2927988],[5.1387388,7.2922971]]}},
    {"type":"Feature","id":"way/594492998","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1388327,7.2929114],[5.1390937,7.2928815],[5.1392521,7.292857],[5.1393476,7.292841],[5.1393892,7.2928242],[5.1394179,7.2928084],[5.1394511,7.29278],[5.1394972,7.2927115],[5.1395562,7.292625],[5.1395732,7.292587],[5.1395967,7.292543],[5.1396297,7.292472],[5.1396539,7.2924467],[5.1396867,7.2924248],[5.1397383,7.2924043]]}},
    {"type":"Feature","id":"way/594492999","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1381196,7.2935549],[5.1380853,7.2935419],[5.1380891,7.2934169],[5.1381145,7.2933237],[5.1381278,7.2932525],[5.1381488,7.2931745],[5.1381669,7.293089],[5.1381868,7.2930295],[5.1381944,7.292988],[5.1382116,7.2929519],[5.1382152,7.2929048],[5.138213,7.292818],[5.1382187,7.2927881],[5.1381965,7.2925927],[5.138167,7.2924769],[5.1381174,7.2923718],[5.1380651,7.2922854],[5.1380503,7.2922016],[5.138045,7.2921483],[5.1380551,7.2920852],[5.1381087,7.2920537],[5.1383715,7.2921382],[5.1384651,7.2922009],[5.1386658,7.2922806],[5.1387388,7.2922971],[5.1387841,7.2923032],[5.1388455,7.2923065],[5.1388981,7.2923108],[5.1389494,7.2923139],[5.1389975,7.2923285],[5.1390553,7.292336],[5.1391235,7.2923445],[5.1391862,7.2923544],[5.1392785,7.2923765],[5.1393586,7.2923805],[5.1396552,7.2924059],[5.1397383,7.2924043]]}},
    {"type":"Feature","id":"way/594493000","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1397383,7.2924043],[5.139861,7.2923966],[5.1401537,7.2923979],[5.140849,7.2923869],[5.141108,7.2923819],[5.1412684,7.2923963],[5.1413661,7.2923913],[5.1414369,7.2923808],[5.1415384,7.2923598],[5.1415917,7.2923434],[5.1416275,7.2923234],[5.1416739,7.2922908],[5.1417482,7.2922672],[5.1418862,7.2922283],[5.1423472,7.292113],[5.1424285,7.2921038],[5.1424945,7.2921018],[5.1425999,7.2921079],[5.1426995,7.2920968],[5.142765,7.2920963]]}},
    {"type":"Feature","id":"way/594496447","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1388531,7.2934513],[5.1394021,7.293468],[5.1396663,7.2934221],[5.1396895,7.2934118],[5.1396975,7.2933921],[5.1396637,7.2930782],[5.1394972,7.2927115]]}},
    {"type":"Feature","id":"way/594520960","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1483936,7.2920656],[5.1483632,7.2920985],[5.1483457,7.2921344],[5.1483239,7.292267],[5.1483084,7.2927669]]}},
    {"type":"Feature","id":"way/594520962","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1514674,7.2965173],[5.1515898,7.2965774],[5.1516498,7.2966154],[5.1517939,7.2967444],[5.1519133,7.2968468],[5.1519797,7.2969],[5.1519998,7.2969266],[5.1520347,7.2969951],[5.1520541,7.2970464],[5.1521098,7.2971608],[5.1521728,7.2972688],[5.1521731,7.2972772],[5.1521705,7.2972891],[5.152154,7.2973277],[5.1521138,7.297361],[5.1519897,7.2974275],[5.1519026,7.2974847],[5.1518442,7.2975412],[5.1518121,7.2975864],[5.1517832,7.2976283]]}},
    {"type":"Feature","id":"way/594520966","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1506839,7.2951857],[5.1502309,7.2949312],[5.1498071,7.294743]]}},
    {"type":"Feature","id":"way/594520969","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.150923,7.2944473],[5.1514386,7.2945082],[5.1522405,7.2945574]]}},
    {"type":"Feature","id":"way/594520972","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1472181,7.2971182],[5.147241,7.2970717],[5.1472704,7.2970117],[5.1472952,7.2969818],[5.1473864,7.2969007],[5.1474135,7.2968792],[5.1478176,7.2965582],[5.1478538,7.2965294],[5.1478801,7.2964758]]}},
    {"type":"Feature","id":"way/594520979","properties":{"highway":"service"},"geometry":{"type":"LineString","coordinates":[[5.1494417,7.292993],[5.1495114,7.2929931],[5.1495486,7.292997],[5.1495831,7.2930084],[5.1496587,7.2930485]]}},
    {"type":"Feature","id":"way/594520993","properties":{"highway":"tertiary","name":"Oduduwa Road","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1498386,7.2930222],[5.1497192,7.2930976]]}},
    {"type":"Feature","id":"way/594521000","properties":{"highway":"tertiary","oneway":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1496964,7.293049],[5.1497575,7.293034],[5.1498386,7.2930222]]}},
    {"type":"Feature","id":"way/594521005","properties":{"highway":"tertiary","name":"Oduduwa Road","oneway":"yes","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1496953,7.2931265],[5.1496629,7.2932049],[5.149626,7.2932806]]}},
    {"type":"Feature","id":"way/594521010","properties":{"highway":"tertiary","oneway":"yes"},"geometry":{"type":"LineString","coordinates":[[5.149626,7.2932806],[5.1496274,7.2931929],[5.1496346,7.2930965]]}},
    {"type":"Feature","id":"way/594521011","properties":{"highway":"tertiary","junction":"roundabout","oneway":"yes"},"geometry":{"type":"LineString","coordinates":[[5.1496587,7.2930485],[5.1496776,7.2930445],[5.1496964,7.293049],[5.1497113,7.2930611],[5.1497194,7.2930785],[5.1497192,7.2930976],[5.1497106,7.2931148],[5.1496953,7.2931265],[5.1496765,7.2931305],[5.1496577,7.293126],[5.1496428,7.2931139],[5.1496346,7.2930965],[5.1496349,7.2930774],[5.1496435,7.2930602],[5.1496587,7.2930485]]}},
    {"type":"Feature","id":"way/594521013","properties":{"bicycle":"yes","foot":"yes","highway":"residential","horse":"yes","lanes":"2","maxspeed":"40","motor_vehicle":"yes","name":"Oduduwa Road","oneway":"no","sidewalk":"both","sidewalk:left:surface":"concrete","smoothness":"excellent","surface":"asphalt"},"geometry":{"type":"LineString","coordinates":[[5.149626,7.2932806],[5.1496043,7.2933921],[5.1495982,7.293469],[5.1495936,7.2936142],[5.1495976,7.2941852],[5.1495908,7.2943651],[5.1495853,7.2945573],[5.1495811,7.2947888],[5.1495864,7.2952956],[5.1495744,7.2959438],[5.1495124,7.2961019],[5.1494401,7.2962348],[5.1493611,7.2963557],[5.1492459,7.296473],[5.149101,7.2965249],[5.1489508,7.2965435],[5.1486865,7.2965303],[5.1485833,7.2965252],[5.148523,7.2965222],[5.1478801,7.2964758],[5.1474716,7.2964349],[5.1464687,7.2963763],[5.1457939,7.2963533],[5.1452869,7.2963307],[5.1450482,7.2963466],[5.1448792,7.2963945],[5.1446968,7.2964836],[5.144548,7.2966113],[5.1443991,7.2967923],[5.1439941,7.2973072],[5.1432288,7.2982179],[5.1415258,7.3003216],[5.1412126,7.3007032],[5.1411898,7.300729],[5.1410624,7.3008734],[5.1405501,7.3015041],[5.1404087,7.3016782],[5.1400062,7.3021645],[5.1399869,7.3021852],[5.1399721,7.302201],[5.1399305,7.3022462],[5.1398676,7.3023281]]}},
    {"type":"Feature","id":"way/594528716","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1425596,7.2899038],[5.1426634,7.2898727],[5.1426942,7.289835],[5.1427585,7.2897121],[5.1427974,7.2896608],[5.142843,7.2896202],[5.1429382,7.2895547],[5.1429891,7.2895061],[5.1430381,7.289427],[5.143083,7.2893106],[5.1431333,7.2891779],[5.143174,7.2891298]]}},
    {"type":"Feature","id":"way/594528717","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1425078,7.2899802],[5.1425596,7.2899038]]}},
    {"type":"Feature","id":"way/594528718","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1425596,7.2899038],[5.142598,7.2900493],[5.1426227,7.2901483],[5.1426476,7.2902495],[5.142654,7.290278],[5.1426681,7.2903399],[5.1426857,7.2904338],[5.142685,7.2905019],[5.1426709,7.2905559],[5.142632,7.2906127],[5.1425666,7.29067]]}},
    {"type":"Feature","id":"way/594542107","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.142765,7.2920963],[5.1427925,7.2921662],[5.1428093,7.292259],[5.1428026,7.2923718],[5.1428079,7.2924833],[5.1428153,7.2925761],[5.1428388,7.2926593],[5.1428622,7.2927082],[5.1429078,7.2927461],[5.1429648,7.2927794],[5.1430339,7.2928226],[5.1430835,7.2928625],[5.1431257,7.2929147],[5.1431546,7.2929547],[5.1431881,7.2929813],[5.1432229,7.2929786],[5.1432591,7.2929704]]}},
    {"type":"Feature","id":"way/594542108","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1432591,7.2929704],[5.1433503,7.2929637],[5.1434308,7.292977],[5.1435039,7.292997],[5.1436165,7.2930143],[5.1437319,7.2930262],[5.1438016,7.2930249],[5.143866,7.2930076],[5.1439424,7.2929597],[5.1440236,7.2928653]]}},
    {"type":"Feature","id":"way/594542109","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1447586,7.2927297],[5.1446151,7.2926472],[5.1445072,7.2925794],[5.1444575,7.2925075],[5.1444522,7.292441],[5.144483,7.2923745],[5.144542,7.2923186],[5.1445742,7.2922827],[5.1445769,7.2922374],[5.1445648,7.2921842],[5.1445729,7.2921097],[5.1446064,7.2920325],[5.1446815,7.2919394],[5.1448303,7.291829],[5.144947,7.2917505],[5.1450556,7.291696],[5.1451803,7.2916441],[5.1452835,7.2916161],[5.1453713,7.2916061]]}},
    {"type":"Feature","id":"way/594542110","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1453713,7.2916061],[5.1456342,7.2916381],[5.1459078,7.2916554],[5.1460339,7.2916367],[5.1461278,7.291596]]}},
    {"type":"Feature","id":"way/594542111","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1470468,7.2907247],[5.1471219,7.2904187],[5.1471447,7.2902897],[5.1471447,7.2901846],[5.1471206,7.2900954],[5.1469663,7.2898127],[5.1469368,7.2897156],[5.1469382,7.289677],[5.1469637,7.2896438],[5.1470223,7.2896302]]}},
    {"type":"Feature","id":"way/594542113","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1370268,7.3036237],[5.1372696,7.3035652],[5.1373977,7.3035372],[5.1375445,7.3035053],[5.1376733,7.3034813],[5.1378288,7.3034454],[5.1380085,7.3034028],[5.1383142,7.3033244],[5.1386168,7.303252]]}},
    {"type":"Feature","id":"way/594542114","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1424344,7.3019961],[5.1422359,7.3021384],[5.1419354,7.3023952],[5.1416336,7.3026413],[5.141376,7.3028635],[5.1411453,7.3030843],[5.1409588,7.3032745],[5.1407634,7.3035153]]}},
    {"type":"Feature","id":"way/594542115","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1397274,7.3017361],[5.1398749,7.3018106],[5.140119,7.3018758],[5.140343,7.3019184],[5.1405509,7.301945],[5.1407856,7.3019557],[5.1410217,7.301945],[5.141196,7.301945],[5.1413409,7.301929],[5.1415394,7.3018758],[5.1416936,7.3018292],[5.1418452,7.3017986],[5.142029,7.3017986],[5.1421739,7.3018412],[5.1422866,7.3018998],[5.1424344,7.3019961]]}},
    {"type":"Feature","id":"way/594542117","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1398676,7.3023281],[5.1398328,7.302509],[5.139763,7.3027006],[5.1396504,7.3028922],[5.1395431,7.3030704],[5.1394037,7.3032008],[5.1392213,7.30329],[5.1390844,7.3033331]]}},
    {"type":"Feature","id":"way/594542118","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1397274,7.3017361],[5.139836,7.3021091],[5.1398676,7.3023281]]}},
    {"type":"Feature","id":"way/594542120","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1424344,7.3019961],[5.1426464,7.3020892],[5.1428436,7.3021797],[5.1428959,7.3022249],[5.1429214,7.3022621],[5.1429428,7.3023074]]}},
    {"type":"Feature","id":"way/594542121","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1426008,7.3055275],[5.142474,7.3053625],[5.1423071,7.3052228],[5.1421736,7.3050831],[5.1420388,7.3049633],[5.1418625,7.3048263],[5.1417485,7.3046507],[5.1416694,7.3045389],[5.1416023,7.3043527],[5.1415493,7.3040314]]}},
    {"type":"Feature","id":"way/594542122","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1426008,7.3055275],[5.1427591,7.3059412],[5.1429348,7.3063975],[5.1430473,7.3067051],[5.1431734,7.3070909],[5.1432699,7.3073484],[5.1433014,7.3074207],[5.1434517,7.3076455],[5.1436086,7.3078916],[5.1437374,7.3081111],[5.1439224,7.3083984],[5.1440324,7.308558],[5.1441637,7.3087207]]}},
    {"type":"Feature","id":"way/594542123","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1415493,7.3040314],[5.1416975,7.304054],[5.1418819,7.3041032],[5.1420301,7.3041764],[5.1421716,7.3042376],[5.1422943,7.3043188],[5.1424237,7.3044386],[5.1425733,7.3045584],[5.1427188,7.3046648],[5.1428791,7.3047367],[5.1430608,7.3048258],[5.143211,7.3048924],[5.1433814,7.3049536],[5.1435799,7.3050241],[5.1437851,7.3050694],[5.1440761,7.3051013],[5.1442328,7.3051954],[5.144322,7.3051508]]}},
    {"type":"Feature","id":"way/630555680","properties":{"highway":"tertiary","name":"FUTA South Gate Road","surface":"paved"},"geometry":{"type":"LineString","coordinates":[[5.1514196,7.2869989],[5.1518229,7.2870838],[5.152035,7.2871291],[5.1523338,7.2872169],[5.1524148,7.2872419],[5.1536841,7.2876161],[5.1544232,7.2878339],[5.1546326,7.2878958],[5.1553205,7.2880982],[5.1556404,7.2881893],[5.1560995,7.2882476],[5.1562834,7.2882761],[5.1564883,7.2883063],[5.1566634,7.2883293],[5.1568889,7.2883571],[5.1570981,7.2883804],[5.1573108,7.2884003],[5.1574746,7.2884122],[5.1577274,7.2884241],[5.1578637,7.2884304],[5.1583024,7.288446],[5.1585081,7.2884506],[5.158931,7.2884592],[5.1591642,7.2884569],[5.1596071,7.2884523],[5.160475,7.2884323],[5.1609102,7.2884434]]}},
    {"type":"Feature","id":"way/677940768","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1281974,7.3106533],[5.1283254,7.310644],[5.1288188,7.3106201],[5.1292693,7.3105908],[5.129382,7.3105934],[5.1296127,7.3106214],[5.1298957,7.310668],[5.1302297,7.3106906],[5.1305448,7.3107265],[5.1306763,7.3107385],[5.1308332,7.3107199],[5.1310397,7.3106427],[5.1311443,7.3105975],[5.1312798,7.3105669],[5.1317303,7.3104804],[5.1320976,7.3104126],[5.1323121,7.3103633],[5.1325066,7.3102915],[5.1329089,7.3101185],[5.1333095,7.3098232]]}},
    {"type":"Feature","id":"way/677940769","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1333095,7.3098232],[5.1333296,7.3097021],[5.1333162,7.3095931],[5.1332518,7.3094321],[5.1330574,7.3090265],[5.1328937,7.3086102],[5.1327979,7.308189]]}},
    {"type":"Feature","id":"way/677940770","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1324503,7.3103114],[5.1320748,7.3096383],[5.1317972,7.3091647],[5.1312339,7.3082495],[5.13087,7.3072878]]}},
    {"type":"Feature","id":"way/709619046","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1356551,7.299629],[5.135574,7.2993644],[5.1354905,7.2990606]]}},
    {"type":"Feature","id":"way/709619047","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1354905,7.2990606],[5.135293,7.2991003],[5.1351748,7.2991248],[5.134935,7.2991568],[5.1346909,7.2991642],[5.1344332,7.2991592],[5.1339972,7.2991376],[5.1335834,7.2991233],[5.1326938,7.2990687],[5.1322195,7.2990386],[5.1320377,7.299032],[5.1318304,7.2990154],[5.1314695,7.2989991]]}},
    {"type":"Feature","id":"way/709619048","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1315862,7.3001524],[5.132038,7.300153],[5.132477,7.3001638],[5.1327579,7.3001809],[5.1332908,7.3001958],[5.1335801,7.3002016],[5.1336793,7.3001875],[5.1338094,7.3001616],[5.1340595,7.3000909],[5.1344881,7.2999616],[5.1356551,7.299629]]}},
    {"type":"Feature","id":"way/709619052","properties":{"highway":"residential"},"geometry":{"type":"LineString","coordinates":[[5.1283068,7.3031567],[5.1282638,7.3029691],[5.1282383,7.302725],[5.1282463,7.3025062],[5.1282906,7.3021048],[5.1283523,7.3016502],[5.1284489,7.3012192],[5.1286299,7.3004917],[5.1287251,7.3001831],[5.1288163,7.2999597],[5.1289223,7.2997363],[5.129047,7.2994995],[5.129377,7.2989341],[5.1294722,7.2987399],[5.1295326,7.2986641],[5.1296131,7.2985749],[5.1296721,7.2985204],[5.1297432,7.2984778],[5.1298505,7.2984272],[5.1300462,7.2983953],[5.1307898,7.2983594],[5.1310446,7.2983674],[5.13137,7.2983931]]}}
  ]
};

// --- LOCAL GEOGRAPHIC PROJECTION (LON/LAT -> THREE.JS WORLD METERS) ---
// Local equirectangular / tangent-plane conversion centered on FUTA network centroid.
// Three.js convention: +X = East, -Z = North, +Y = Up.
const MAP_SCALE = 0.65; // Slightly scaled campus meters for comfortable 3D driving & turning radius
const EARTH_RADIUS_M = 6378137.0;

function computeCentroid(geojson) {
  let sumLon = 0;
  let sumLat = 0;
  let count = 0;

  for (const feat of geojson.features || []) {
    const coordsList = extractCoordinateLists(feat.geometry);
    for (const ringOrLine of coordsList) {
      for (const pt of ringOrLine) {
        if (Array.isArray(pt) && pt.length >= 2) {
          sumLon += pt[0];
          sumLat += pt[1];
          count++;
        }
      }
    }
  }

  return count > 0
    ? { lon: sumLon / count, lat: sumLat / count }
    : { lon: 5.1415, lat: 7.2985 };
}

function extractCoordinateLists(geometry) {
  if (!geometry || !geometry.type || !geometry.coordinates) return [];
  const t = geometry.type;
  const c = geometry.coordinates;
  if (t === 'LineString') return [c];
  if (t === 'MultiLineString') return c;
  if (t === 'Polygon') return c;
  if (t === 'MultiPolygon') return c.flat();
  if (t === 'GeometryCollection' && Array.isArray(geometry.geometries)) {
    return geometry.geometries.flatMap((g) => extractCoordinateLists(g));
  }
  return [];
}

const originLonLat = computeCentroid(FUTA_GEOJSON);
const originLatRad = THREE.MathUtils.degToRad(originLonLat.lat);
const cosOriginLat = Math.cos(originLatRad);

function lonLatToWorld(lon, lat) {
  const dLonRad = THREE.MathUtils.degToRad(lon - originLonLat.lon);
  const dLatRad = THREE.MathUtils.degToRad(lat - originLonLat.lat);
  const eastMeters = dLonRad * EARTH_RADIUS_M * cosOriginLat;
  const northMeters = dLatRad * EARTH_RADIUS_M;
  return {
    x: eastMeters * MAP_SCALE,
    z: -northMeters * MAP_SCALE
  };
}

// --- BUILD FUTA ROAD NETWORK, JUNCTIONS, ROUNDABOUTS, PROPS & MINIMAP DATA ---
const roadNetworkGroup = new THREE.Group();
scene.add(roadNetworkGroup);

const roadSegments = [];       // For road snapping, nearest-road lookup & minimap rendering
const junctionNodes = new Map(); // Quantized grid key -> { x, z, count, radius }
const roundabouts = [];        // Detected roundabouts for islands & guidance

function getRoadWidth(props = {}) {
  const lanes = parseInt(props.lanes, 10);
  if (!Number.isNaN(lanes) && lanes > 0) {
    return THREE.MathUtils.clamp(lanes * 4.2, 5.5, 14.0);
  }
  if (props.junction === 'roundabout') return 7.2;
  if (props.highway === 'secondary') return 11.5;
  if (props.highway === 'tertiary') return 9.0;
  if (props.highway === 'service') return 5.8;
  return 7.8; // Default residential / campus road width
}

function registerNode(x, z, roadWidth) {
  const snapSize = 1.6;
  const key = `${Math.round(x / snapSize)}_${Math.round(z / snapSize)}`;
  const existing = junctionNodes.get(key);
  if (existing) {
    existing.x = (existing.x * existing.count + x) / (existing.count + 1);
    existing.z = (existing.z * existing.count + z) / (existing.count + 1);
    existing.count += 1;
    existing.radius = Math.max(existing.radius, roadWidth * 0.62);
  } else {
    junctionNodes.set(key, {
      x,
      z,
      count: 1,
      radius: roadWidth * 0.62
    });
  }
}

function addRoadStripSegment(p1, p2, width, props = {}, segIndex = 0) {
  const dx = p2.x - p1.x;
  const dz = p2.z - p1.z;
  const len = Math.hypot(dx, dz);
  if (len < 0.05) return;

  const angle = Math.atan2(dx, dz);
  const midX = (p1.x + p2.x) * 0.5;
  const midZ = (p1.z + p2.z) * 0.5;
  const isUnpaved =
    props.surface === 'unpaved' || props.surface === 'sand' || props.surface === 'ground';
  const roadMat = isUnpaved ? unpavedRoadMat : asphaltMat;

  const strip = new THREE.Mesh(
    new THREE.BoxGeometry(width, 0.08, len + 0.14),
    roadMat
  );
  strip.position.set(midX, -0.08, midZ);
  strip.rotation.y = angle;
  strip.receiveShadow = true;
  roadNetworkGroup.add(strip);

  // Visible outer road edge lines on paved roads
  if (!isUnpaved && len > 1.2) {
    for (const side of [-1, 1]) {
      const edgeOffset = side * (width * 0.5 - 0.18);
      const edge = new THREE.Mesh(
        new THREE.BoxGeometry(0.14, 0.02, len),
        edgeLineMat
      );
      edge.position.set(
        midX + Math.cos(angle) * edgeOffset,
        -0.035,
        midZ - Math.sin(angle) * edgeOffset
      );
      edge.rotation.y = angle;
      edge.receiveShadow = true;
      roadNetworkGroup.add(edge);
    }

    // Center dashed lane divider on multi-lane / wider roads
    const lanes = parseInt(props.lanes, 10) || (width >= 9.0 ? 2 : 1);
    if (lanes >= 2 && len > 2.4 && props.junction !== 'roundabout') {
      const dash = new THREE.Mesh(
        new THREE.BoxGeometry(0.14, 0.02, Math.min(2.4, len * 0.45)),
        centerLineMat
      );
      dash.position.set(midX, -0.033, midZ);
      dash.rotation.y = angle;
      roadNetworkGroup.add(dash);
    }

    // Sidewalks when explicitly tagged or on primary campus corridors
    const hasSidewalk =
      props.sidewalk === 'both' ||
      props.highway === 'secondary' ||
      props.highway === 'tertiary';
    if (hasSidewalk && len > 2.0) {
      for (const side of [-1, 1]) {
        const walkOffset = side * (width * 0.5 + 1.15);
        const walk = new THREE.Mesh(
          new THREE.BoxGeometry(1.7, 0.14, len),
          sidewalkMat
        );
        walk.position.set(
          midX + Math.cos(angle) * walkOffset,
          -0.01,
          midZ - Math.sin(angle) * walkOffset
        );
        walk.rotation.y = angle;
        walk.receiveShadow = true;
        roadNetworkGroup.add(walk);
      }
    }
  }

  // Place roadside trees and streetlights sparsely along longer straight segments
  if (len > 14 && !isUnpaved && props.junction !== 'roundabout') {
    const side = segIndex % 2 === 0 ? 1 : -1;
    const perpX = Math.cos(angle) * side;
    const perpZ = -Math.sin(angle) * side;
    const propDist = width * 0.5 + 2.4;
    const px = midX + perpX * propDist;
    const pz = midZ + perpZ * propDist;

    if (segIndex % 3 === 0) {
      const tree = new THREE.Group();
      tree.position.set(px, -0.08, pz);
      const trunk = new THREE.Mesh(
        new THREE.CylinderGeometry(0.16, 0.24, 2.0, 7),
        trunkMat
      );
      trunk.position.y = 1.0;
      trunk.castShadow = true;
      const crown = new THREE.Mesh(
        new THREE.DodecahedronGeometry(1.4, 0),
        foliageMat
      );
      crown.position.y = 2.6;
      crown.castShadow = true;
      tree.add(trunk, crown);
      roadNetworkGroup.add(tree);
    } else if (segIndex % 5 === 0) {
      const pole = new THREE.Mesh(
        new THREE.CylinderGeometry(0.08, 0.12, 5.6, 6),
        poleMat
      );
      pole.position.set(px, 2.72, pz);
      const bulb = new THREE.Mesh(
        new THREE.BoxGeometry(0.35, 0.1, 0.18),
        lampBulbMat
      );
      bulb.position.set(px - perpX * 0.55, 5.48, pz - perpZ * 0.55);
      roadNetworkGroup.add(pole, bulb);
    }
  }

  const roadName =
    typeof props.name === 'string' && props.name.trim().length > 0
      ? props.name.trim()
      : props.junction === 'roundabout'
        ? 'Campus Roundabout'
        : props.highway
          ? `FUTA ${props.highway.charAt(0).toUpperCase() + props.highway.slice(1)} Road`
          : 'FUTA Campus Road';

  roadSegments.push({
    x1: p1.x,
    z1: p1.z,
    x2: p2.x,
    z2: p2.z,
    width,
    name: roadName,
    highway: props.highway || 'residential'
  });
}

let placedBuildingsCount = 0;
const MAX_PLACEHOLDER_BUILDINGS = 95;

function tryPlacePlaceholderBuilding(p1, p2, width, seed) {
  if (placedBuildingsCount >= MAX_PLACEHOLDER_BUILDINGS) return;
  const dx = p2.x - p1.x;
  const dz = p2.z - p1.z;
  const len = Math.hypot(dx, dz);
  if (len < 12) return;

  const angle = Math.atan2(dx, dz);
  const side = seed % 2 === 0 ? 1 : -1;
  const perpX = Math.cos(angle) * side;
  const perpZ = -Math.sin(angle) * side;
  const setback = width * 0.5 + 11.5;

  const bx = (p1.x + p2.x) * 0.5 + perpX * setback;
  const bz = (p1.z + p2.z) * 0.5 + perpZ * setback;

  // Keep buildings clear of all road segments so no road or spawn point is blocked
  for (const seg of roadSegments) {
    const segLenSq = (seg.x2 - seg.x1) ** 2 + (seg.z2 - seg.z1) ** 2;
    if (segLenSq < 0.01) continue;
    const t = THREE.MathUtils.clamp(
      ((bx - seg.x1) * (seg.x2 - seg.x1) + (bz - seg.z1) * (seg.z2 - seg.z1)) / segLenSq,
      0,
      1
    );
    const cx = seg.x1 + t * (seg.x2 - seg.x1);
    const cz = seg.z1 + t * (seg.z2 - seg.z1);
    if (Math.hypot(bx - cx, bz - cz) < seg.width * 0.5 + 8.5) {
      return;
    }
  }

  const bWidth = 7.5 + ((seed * 3) % 5);
  const bDepth = 8.5 + ((seed * 5) % 5);
  const bHeight = 8.0 + ((seed * 7) % 12);
  const bMat = buildingMaterials[seed % buildingMaterials.length];

  const bGroup = new THREE.Group();
  bGroup.position.set(bx, -0.14, bz);
  bGroup.rotation.y = angle;

  const body = new THREE.Mesh(
    new THREE.BoxGeometry(bWidth, bHeight, bDepth),
    bMat
  );
  body.position.y = bHeight * 0.5;
  body.castShadow = true;
  body.receiveShadow = true;
  bGroup.add(body);

  const roof = new THREE.Mesh(
    new THREE.BoxGeometry(bWidth * 0.72, 0.5, bDepth * 0.72),
    roofMat
  );
  roof.position.y = bHeight + 0.25;
  bGroup.add(roof);

  roadNetworkGroup.add(bGroup);
  placedBuildingsCount++;
}

function buildFutaRoadNetwork(geojson) {
  let featureIndex = 0;

  for (const feature of geojson.features || []) {
    const props = feature.properties || {};
    const roadWidth = getRoadWidth(props);
    const coordLists = extractCoordinateLists(feature.geometry);

    for (const coords of coordLists) {
      if (!Array.isArray(coords) || coords.length < 2) continue;

      const worldPts = [];
      for (const pt of coords) {
        if (!Array.isArray(pt) || pt.length < 2) continue;
        const lon = Number(pt[0]);
        const lat = Number(pt[1]);
        if (!Number.isFinite(lon) || !Number.isFinite(lat)) continue;
        worldPts.push(lonLatToWorld(lon, lat));
      }

      if (worldPts.length < 2) continue;

      // Detect roundabouts from OSM tag or closed ring geometry
      const first = worldPts[0];
      const last = worldPts[worldPts.length - 1];
      const isClosed = Math.hypot(last.x - first.x, last.z - first.z) < 2.5;
      if (props.junction === 'roundabout' || (isClosed && worldPts.length >= 6)) {
        let cx = 0;
        let cz = 0;
        for (const p of worldPts) {
          cx += p.x;
          cz += p.z;
        }
        cx /= worldPts.length;
        cz /= worldPts.length;

        let avgR = 0;
        for (const p of worldPts) {
          avgR += Math.hypot(p.x - cx, p.z - cz);
        }
        avgR /= worldPts.length;

        if (avgR > 1.2) {
          const islandRadius = Math.max(1.2, avgR - roadWidth * 0.52);
          const island = new THREE.Mesh(
            new THREE.CylinderGeometry(islandRadius, islandRadius * 1.06, 0.26, 20),
            roundaboutIslandMat
          );
          island.position.set(cx, 0.04, cz);
          island.receiveShadow = true;
          roadNetworkGroup.add(island);

          const curbRing = new THREE.Mesh(
            new THREE.TorusGeometry(islandRadius, 0.18, 8, 24),
            curbMat
          );
          curbRing.rotation.x = Math.PI / 2;
          curbRing.position.set(cx, 0.16, cz);
          roadNetworkGroup.add(curbRing);

          roundabouts.push({ x: cx, z: cz, radius: avgR });
        }
      }

      for (let i = 0; i < worldPts.length - 1; i++) {
        const a = worldPts[i];
        const b = worldPts[i + 1];
        addRoadStripSegment(a, b, roadWidth, props, featureIndex + i);
        registerNode(a.x, a.z, roadWidth);
        registerNode(b.x, b.z, roadWidth);

        if (i % 4 === 1) {
          tryPlacePlaceholderBuilding(a, b, roadWidth, featureIndex * 7 + i);
        }
      }
    }
    featureIndex++;
  }

  // Render smooth junction discs at multi-road nodes so intersecting strips blend cleanly
  for (const node of junctionNodes.values()) {
    if (node.count >= 2) {
      const disc = new THREE.Mesh(
        new THREE.CylinderGeometry(node.radius, node.radius, 0.085, 16),
        asphaltMat
      );
      disc.position.set(node.x, -0.077, node.z);
      disc.receiveShadow = true;
      roadNetworkGroup.add(disc);
    }
  }
}

buildFutaRoadNetwork(FUTA_GEOJSON);

// --- SPORTS CAR MODEL ---
const car = new THREE.Group();
scene.add(car);

// Car chassis assembly (tilts/rolls independently of ground heading)
const chassisGroup = new THREE.Group();
car.add(chassisGroup);

const paintMat = new THREE.MeshStandardMaterial({
  color: 0xef4444,
  metalness: 0.65,
  roughness: 0.24
});
const darkTrimMat = new THREE.MeshStandardMaterial({
  color: 0x111827,
  metalness: 0.35,
  roughness: 0.55
});
const glassMat = new THREE.MeshStandardMaterial({
  color: 0x0f172a,
  metalness: 0.85,
  roughness: 0.12
});
const headlightMat = new THREE.MeshBasicMaterial({
  color: 0xfef9c3
});
const taillightMat = new THREE.MeshBasicMaterial({
  color: 0xff2a2a
});

// Lower sporty body
const lowerBody = new THREE.Mesh(
  new THREE.BoxGeometry(1.96, 0.52, 4.15),
  paintMat
);
lowerBody.position.set(0, 0.64, 0);
lowerBody.castShadow = true;
lowerBody.receiveShadow = true;
chassisGroup.add(lowerBody);

// Sloped nose / hood
const hood = new THREE.Mesh(
  new THREE.BoxGeometry(1.78, 0.22, 1.35),
  paintMat
);
hood.position.set(0, 0.92, -1.25);
hood.rotation.x = 0.08;
hood.castShadow = true;
chassisGroup.add(hood);

// Cabin / greenhouse
const cabin = new THREE.Mesh(
  new THREE.BoxGeometry(1.56, 0.54, 1.85),
  glassMat
);
cabin.position.set(0, 1.14, 0.12);
cabin.castShadow = true;
chassisGroup.add(cabin);

// Roof panel
const roof = new THREE.Mesh(
  new THREE.BoxGeometry(1.48, 0.08, 1.35),
  paintMat
);
roof.position.set(0, 1.42, 0.18);
roof.castShadow = true;
chassisGroup.add(roof);

// Rear spoiler lip
const spoiler = new THREE.Mesh(
  new THREE.BoxGeometry(1.76, 0.08, 0.32),
  darkTrimMat
);
spoiler.position.set(0, 1.02, 1.88);
spoiler.castShadow = true;
chassisGroup.add(spoiler);

// Front bumper & splitter
const frontLip = new THREE.Mesh(
  new THREE.BoxGeometry(1.98, 0.14, 0.32),
  darkTrimMat
);
frontLip.position.set(0, 0.42, -2.04);
chassisGroup.add(frontLip);

// Headlights (-Z is forward)
for (const x of [-0.72, 0.72]) {
  const hl = new THREE.Mesh(
    new THREE.BoxGeometry(0.36, 0.13, 0.08),
    headlightMat
  );
  hl.position.set(x, 0.74, -2.07);
  chassisGroup.add(hl);
}

// Taillights (+Z is rear)
for (const x of [-0.72, 0.72]) {
  const tl = new THREE.Mesh(
    new THREE.BoxGeometry(0.42, 0.13, 0.08),
    taillightMat
  );
  tl.position.set(x, 0.76, 2.07);
  chassisGroup.add(tl);
}

// Wheels: front wheels steer via pivot; all 4 wheels spin around local X
const tireMat = new THREE.MeshStandardMaterial({
  color: 0x141619,
  roughness: 0.88,
  metalness: 0.08
});
const rimMat = new THREE.MeshStandardMaterial({
  color: 0xd1d5db,
  roughness: 0.3,
  metalness: 0.8
});

const tireGeo = new THREE.CylinderGeometry(0.4, 0.4, 0.28, 22);
tireGeo.rotateZ(Math.PI / 2);

const rimGeo = new THREE.CylinderGeometry(0.24, 0.24, 0.3, 12);
rimGeo.rotateZ(Math.PI / 2);

const frontWheelPivots = [];
const spinningWheels = [];

const wheelCoords = [
  { x: -1.02, z: -1.28, front: true },
  { x:  1.02, z: -1.28, front: true },
  { x: -1.04, z:  1.28, front: false },
  { x:  1.04, z:  1.28, front: false }
];

for (const w of wheelCoords) {
  const spinMesh = new THREE.Group();
  const tire = new THREE.Mesh(tireGeo, tireMat);
  tire.castShadow = true;
  const rim = new THREE.Mesh(rimGeo, rimMat);
  spinMesh.add(tire, rim);

  if (w.front) {
    const pivot = new THREE.Group();
    pivot.position.set(w.x, 0.4, w.z);
    pivot.add(spinMesh);
    car.add(pivot);
    frontWheelPivots.push(pivot);
  } else {
    spinMesh.position.set(w.x, 0.4, w.z);
    car.add(spinMesh);
  }
  spinningWheels.push(spinMesh);
}

// --- ROAD SNAP & SPAWN PLACEMENT ---
function projectOntoSegment(px, pz, seg) {
  const vx = seg.x2 - seg.x1;
  const vz = seg.z2 - seg.z1;
  const lenSq = vx * vx + vz * vz;
  if (lenSq < 1e-6) {
    return {
      x: seg.x1,
      z: seg.z1,
      dist: Math.hypot(px - seg.x1, pz - seg.z1),
      dirX: 0,
      dirZ: -1,
      t: 0,
      seg
    };
  }
  const t = THREE.MathUtils.clamp(
    ((px - seg.x1) * vx + (pz - seg.z1) * vz) / lenSq,
    0,
    1
  );
  const cx = seg.x1 + t * vx;
  const cz = seg.z1 + t * vz;
  const len = Math.sqrt(lenSq);
  return {
    x: cx,
    z: cz,
    dist: Math.hypot(px - cx, pz - cz),
    dirX: vx / len,
    dirZ: vz / len,
    t,
    seg
  };
}

function findNearestRoad(px, pz) {
  let best = null;
  for (const seg of roadSegments) {
    const proj = projectOntoSegment(px, pz, seg);
    if (!best || proj.dist < best.dist) {
      best = proj;
    }
  }
  return best;
}

function getSpawnPointOnFuta() {
  // Northernmost segment of Yobel Road (North Gate corridor)
  const northSeg = roadSegments.find(
    (s) =>
      s.roadName === "Yobel Road" &&
      s.lat1 >= 7.3188 &&
      s.lat2 >= 7.3189
  );

  if (northSeg) {
    const mx = (northSeg.x1 + northSeg.x2) * 0.5;
    const mz = (northSeg.z1 + northSeg.z2) * 0.5;
    const dx = northSeg.x2 - northSeg.x1;
    const dz = northSeg.z2 - northSeg.z1;

    // Face inward (south-southwest into campus); use Math.atan2(dx, dz) to face outward
    const inwardYaw = Math.atan2(-dx, -dz);

    return {
      x: mx,
      z: mz,
      yaw: inwardYaw,
      roadName: northSeg.roadName || "North Gate / Yobel Road",
      lat: (northSeg.lat1 + northSeg.lat2) * 0.5,
      lon: (northSeg.lon1 + northSeg.lon2) * 0.5,
    };
  }

  // Fallback to northernmost road segment in the dataset
  let best = roadSegments[0];
  for (const s of roadSegments) {
    const minZ = Math.min(s.z1, s.z2);
    const bestMinZ = Math.min(best.z1, best.z2);
    if (minZ < bestMinZ) best = s;
  }

  const dx = best.x2 - best.x1;
  const dz = best.z2 - best.z1;
  return {
    x: (best.x1 + best.x2) * 0.5,
    z: (best.z1 + best.z2) * 0.5,
    yaw: Math.atan2(-dx, -dz),
    roadName: best.roadName || "North Gate",
    lat: (best.lat1 + best.lat2) * 0.5,
    lon: (best.lon1 + best.lon2) * 0.5,
  };
}

const initialSpawn = getSpawnPointOnFuta();
car.position.set(initialSpawn.x, -0.04, initialSpawn.z);
car.rotation.y = initialSpawn.yaw;

// --- CONTROLLER & PHYSICS STATE ---
let steering = 512;
let throttle = 512;
let maxSpeedInput = 1023;

let speed = 0;         // signed speed (positive = forward, negative = reverse)
let heading = initialSpawn.yaw;
let lateralVel = 0;

let keyboardSteer = 0;
let keyboardThrottle = 0;

const directionDisplay = document.getElementById('direction');
const speedDisplay = document.getElementById('speed');
const speedLimitDisplay = document.getElementById('speedLimit');
const connectionDisplay = document.getElementById('connection');
const currentRoadDisplay = document.getElementById('currentRoad');
currentRoadDisplay.innerText = initialSpawn.roadName;

// --- MINIMAP RENDERER ---
const minimapCanvas = document.getElementById('minimapCanvas');
const minimapCtx = minimapCanvas.getContext('2d');

function drawMinimap(carX, carZ, carYaw) {
  const w = minimapCanvas.width;
  const h = minimapCanvas.height;
  const radarRange = 220; // World units visible around car
  const scale = Math.min(w, h) / (radarRange * 2);

  minimapCtx.clearRect(0, 0, w, h);
  minimapCtx.fillStyle = '#0f172a';
  minimapCtx.fillRect(0, 0, w, h);

  // Draw nearby FUTA roads
  minimapCtx.lineCap = 'round';
  for (const seg of roadSegments) {
    const sx1 = w * 0.5 + (seg.x1 - carX) * scale;
    const sy1 = h * 0.5 + (seg.z1 - carZ) * scale;
    const sx2 = w * 0.5 + (seg.x2 - carX) * scale;
    const sy2 = h * 0.5 + (seg.z2 - carZ) * scale;

    if (
      Math.max(sx1, sx2) < -20 ||
      Math.min(sx1, sx2) > w + 20 ||
      Math.max(sy1, sy2) < -20 ||
      Math.min(sy1, sy2) > h + 20
    ) {
      continue;
    }

    minimapCtx.strokeStyle =
      seg.highway === 'secondary' || seg.highway === 'tertiary'
        ? '#94a3b8'
        : '#475569';
    minimapCtx.lineWidth = Math.max(1.5, seg.width * scale * 0.65);
    minimapCtx.beginPath();
    minimapCtx.moveTo(sx1, sy1);
    minimapCtx.lineTo(sx2, sy2);
    minimapCtx.stroke();
  }

  // Draw player arrow at center of minimap
  minimapCtx.save();
  minimapCtx.translate(w * 0.5, h * 0.5);
  minimapCtx.rotate(-carYaw);
  minimapCtx.fillStyle = '#ef4444';
  minimapCtx.strokeStyle = '#ffffff';
  minimapCtx.lineWidth = 1.5;
  minimapCtx.beginPath();
  minimapCtx.moveTo(0, -7);
  minimapCtx.lineTo(5, 6);
  minimapCtx.lineTo(0, 3);
  minimapCtx.lineTo(-5, 6);
  minimapCtx.closePath();
  minimapCtx.fill();
  minimapCtx.stroke();
  minimapCtx.restore();
}

// --- WEBSOCKET CLIENT (PORT 81) ---
const socket = new WebSocket('ws://' + location.hostname + ':81/');

socket.onopen = () => {
  connectionDisplay.innerText = '● CONTROLLER ONLINE';
  connectionDisplay.style.color = '#4ade80';
};

socket.onclose = () => {
  connectionDisplay.innerText = '● CONTROLLER OFFLINE';
  connectionDisplay.style.color = '#f87171';
};

socket.onerror = () => {
  console.log('WebSocket connection error');
};

socket.onmessage = (event) => {
  try {
    const data = JSON.parse(event.data);
    if (typeof data.steering === 'number') steering = data.steering;
    if (typeof data.throttle === 'number') throttle = data.throttle;
    if (typeof data.maxSpeed === 'number') maxSpeedInput = data.maxSpeed;
  } catch (err) {
    console.log('Invalid controller JSON');
  }
};

function resetCarToRoad() {
  const nearest = findNearestRoad(car.position.x, car.position.z);
  if (nearest) {
    car.position.x = nearest.x;
    car.position.z = nearest.z;
    heading = Math.atan2(-nearest.dirX, -nearest.dirZ);
    car.rotation.y = heading;
    currentRoadDisplay.innerText = nearest.seg.name;
  } else {
    const sp = getSpawnPointOnFuta();
    car.position.set(sp.x, -0.04, sp.z);
    heading = sp.yaw;
    car.rotation.y = sp.yaw;
    currentRoadDisplay.innerText = sp.roadName;
  }
  speed = 0;
  lateralVel = 0;
}

// --- KEYBOARD FALLBACK CONTROLS ---
window.addEventListener('keydown', (e) => {
  if (e.key === 'ArrowUp') keyboardThrottle = 1;
  if (e.key === 'ArrowDown') keyboardThrottle = -1;
  if (e.key === 'ArrowLeft') keyboardSteer = -1;
  if (e.key === 'ArrowRight') keyboardSteer = 1;
  if (e.key === 'r' || e.key === 'R') resetCarToRoad();
});

window.addEventListener('keyup', (e) => {
  if (e.key === 'ArrowUp' || e.key === 'ArrowDown') keyboardThrottle = 0;
  if (e.key === 'ArrowLeft' || e.key === 'ArrowRight') keyboardSteer = 0;
});

// --- MAIN ANIMATION LOOP ---
const clock = new THREE.Clock();
const desiredCamPos = new THREE.Vector3();
const lookAtTarget = new THREE.Vector3();

function animate() {
  requestAnimationFrame(animate);

  const dt = Math.min(clock.getDelta(), 0.05);

  // 1. Inputs + deadzones
  let steerInput =
    keyboardSteer !== 0 ? keyboardSteer : (steering - 512) / 512;
  let driveInput =
    keyboardThrottle !== 0 ? keyboardThrottle : (throttle - 512) / 512;

  if (Math.abs(steerInput) < 0.10) steerInput = 0;
  if (Math.abs(driveInput) < 0.10) driveInput = 0;

  steerInput = THREE.MathUtils.clamp(steerInput, -1, 1);
  driveInput = THREE.MathUtils.clamp(driveInput, -1, 1);

  // 2. Potentiometer sets max speed limit (20 to 120 km/h scale)
  const clampedPot = THREE.MathUtils.clamp(maxSpeedInput, 0, 1023);
  const speedLimit = Math.round(20 + (clampedPot / 1023) * 100);
  speedLimitDisplay.innerText = speedLimit;

  const targetSpeed = driveInput * speedLimit;
  const accelRate = driveInput !== 0 ? 2.6 : 3.2;
  speed = THREE.MathUtils.damp(speed, targetSpeed, accelRate, dt);

  if (Math.abs(speed) < 0.25 && driveInput === 0) {
    speed = 0;
  }

  // 3. Free 360-degree steering in direction of travel across FUTA network
  const activeSpeed = Math.abs(speed);
  const speedFactor = THREE.MathUtils.clamp(activeSpeed / 14, 0.15, 1);
  const reverseSign = speed >= -0.2 ? 1 : -1;
  const turnRate = 2.15;

  if (activeSpeed > 0.35) {
    const targetYawRate = -steerInput * turnRate * speedFactor * reverseSign;
    heading += targetYawRate * dt;
  }

  // Forward vector in Three.js when yaw = 0 is (0, 0, -1); for yaw = theta it is (-sin(theta), 0, -cos(theta))
  const forwardX = -Math.sin(heading);
  const forwardZ = -Math.cos(heading);
  const rightX = Math.cos(heading);
  const rightZ = -Math.sin(heading);

  const moveStep = speed * dt * 0.28;
  lateralVel = THREE.MathUtils.damp(lateralVel, 0, 8.0, dt);

  car.position.x += forwardX * moveStep + rightX * lateralVel;
  car.position.z += forwardZ * moveStep + rightZ * lateralVel;

  // Soft road corridor guidance & active road label update
  const nearestRoad = findNearestRoad(car.position.x, car.position.z);
  if (nearestRoad) {
    currentRoadDisplay.innerText = nearestRoad.seg.name;
    const maxAllowedOffset = nearestRoad.seg.width * 0.55 + 2.2;
    if (nearestRoad.dist > maxAllowedOffset) {
      const pull = THREE.MathUtils.clamp(
        (nearestRoad.dist - maxAllowedOffset) * 0.22,
        0,
        1.8
      );
      const toRoadX = nearestRoad.x - car.position.x;
      const toRoadZ = nearestRoad.z - car.position.z;
      const dLen = Math.hypot(toRoadX, toRoadZ) || 1;
      car.position.x += (toRoadX / dLen) * pull;
      car.position.z += (toRoadZ / dLen) * pull;
      speed *= 0.94;
    }
  }

  car.rotation.y = heading;

  // Subtle chassis pitch (acceleration/braking) & roll (cornering)
  const targetPitch = THREE.MathUtils.clamp(-driveInput * 0.035, -0.045, 0.045);
  const targetRoll = THREE.MathUtils.clamp(steerInput * speedFactor * 0.055, -0.07, 0.07);
  chassisGroup.rotation.x = THREE.MathUtils.damp(chassisGroup.rotation.x, targetPitch, 8.0, dt);
  chassisGroup.rotation.z = THREE.MathUtils.damp(chassisGroup.rotation.z, targetRoll, 8.0, dt);

  // 4. Front wheel steering & all-wheel rolling rotation
  const frontSteerAngle = -steerInput * 0.42;
  for (const pivot of frontWheelPivots) {
    pivot.rotation.y = THREE.MathUtils.damp(pivot.rotation.y, frontSteerAngle, 14.0, dt);
  }

  const wheelSpinDelta = speed * dt * 0.28;
  for (const wheel of spinningWheels) {
    wheel.rotation.x -= wheelSpinDelta;
  }

  // 5. Scroll asphalt texture with vehicle movement
  asphaltTex.offset.y -= moveStep * 0.16;

  // 6. Smooth third-person chase camera trailing behind car heading
  const camDist = 8.4;
  const camHeight = 4.5;
  desiredCamPos.set(
    car.position.x - forwardX * camDist,
    camHeight,
    car.position.z - forwardZ * camDist
  );
  camera.position.lerp(desiredCamPos, 1 - Math.exp(-7.5 * dt));

  lookAtTarget.set(
    car.position.x + forwardX * 8.5,
    1.1,
    car.position.z + forwardZ * 8.5
  );
  camera.lookAt(lookAtTarget);

  // Keep directional shadow frustum centered near the car
  sunLight.target.position.set(car.position.x, 0, car.position.z);
  sunLight.position.set(car.position.x + 45, 75, car.position.z + 38);
  sunLight.target.updateMatrixWorld();

  // 7. Update HUD telemetry & minimap
  speedDisplay.innerText = Math.round(Math.abs(speed));

  if (speed > 1.0) {
    directionDisplay.innerText = 'FORWARD';
    directionDisplay.className = 'gear-badge gear-forward';
  } else if (speed < -1.0) {
    directionDisplay.innerText = 'REVERSE';
    directionDisplay.className = 'gear-badge gear-reverse';
  } else {
    directionDisplay.innerText = 'NEUTRAL';
    directionDisplay.className = 'gear-badge gear-neutral';
  }

  drawMinimap(car.position.x, car.position.z, heading);

  renderer.render(scene, camera);
}

animate();

// --- RESPONSIVE VIEWPORT ---
window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});
</script>

</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);
  arduinoSerial.begin(9600);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("Game address: http://");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });

  server.begin();
  webSocket.begin();

  Serial.println("Web server started");
}


void loop() {
  server.handleClient();
  webSocket.loop();

  while (arduinoSerial.available()) {
    char c = arduinoSerial.read();

    if (c == '\n') {
      processControllerData();
      inputData = "";
    }
    else if (c != '\r') {
      inputData += c;
    }
  }
}


void processControllerData() {
  int firstComma = inputData.indexOf(',');
  int secondComma = inputData.indexOf(',', firstComma + 1);

  if (firstComma < 0 || secondComma < 0) {
    return;
  }

  steering = inputData.substring(0, firstComma).toInt();
  throttle = inputData.substring(firstComma + 1, secondComma).toInt();
  maxSpeed = inputData.substring(secondComma + 1).toInt();

  String json = "{";
  json += "\"steering\":" + String(steering) + ",";
  json += "\"throttle\":" + String(throttle) + ",";
  json += "\"maxSpeed\":" + String(maxSpeed);
  json += "}";

  webSocket.broadcastTXT(json);

  Serial.println(json);
}
