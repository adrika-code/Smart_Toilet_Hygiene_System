#include <WiFi.h>
#include <WebServer.h>

// ---- WiFi credentials ----
const char* ssid     = "Galaxy A14 5G 2680";
const char* password = "AADRIKA@9898";

// ---- Pins ----
#define MQ135_PIN   34
#define WATER_PIN   32
#define TRIG_PIN    5
#define ECHO_PIN    18
#define BUZZER_PIN  23

// ---- Thresholds ----
#define GAS_THRESHOLD    1800
#define WATER_LOW        500
#define BIN_FULL_CM      10

WebServer server(80);

int gasValue = 0;
int waterValue = 0;
float distanceCM = 0;
bool alertActive = false;

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

void readSensors() {
  gasValue = analogRead(MQ135_PIN);
  waterValue = analogRead(WATER_PIN);
  long d = readDistance();
  if (d > 0) distanceCM = d;

  alertActive = (gasValue > GAS_THRESHOLD) ||
                (waterValue < WATER_LOW) ||
                (distanceCM > 0 && distanceCM < BIN_FULL_CM);

  digitalWrite(BUZZER_PIN, alertActive ? HIGH : LOW);

  Serial.printf("Gas:%d Water:%d Dist:%.1fcm Alert:%s\n", gasValue, waterValue, distanceCM, alertActive ? "YES" : "NO");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Hygiene Monitor</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "*{box-sizing:border-box;margin:0;padding:0;}";
  html += ":root{--bg:#0f1620;--panel:#16202c;--track:#22303f;--ok:#2dd4bf;--bad:#ff5c5c;--text:#e8edf2;--muted:#7c8a99;}";
  html += "body{background:var(--bg);color:var(--text);font-family:-apple-system,Segoe UI,Roboto,sans-serif;min-height:100vh;padding:24px 16px 60px;}";
  html += ".eyebrow{text-align:center;font-size:11px;letter-spacing:3px;color:var(--muted);text-transform:uppercase;margin-bottom:6px;}";
  html += "h1{text-align:center;font-size:22px;font-weight:600;letter-spacing:0.5px;margin-bottom:4px;}";
  html += ".sub{text-align:center;font-size:12px;color:var(--muted);font-family:Consolas,Menlo,monospace;margin-bottom:22px;}";
  html += ".scanline{height:2px;max-width:200px;margin:0 auto 26px;background:linear-gradient(90deg,transparent,var(--ok),transparent);background-size:200% 100%;animation:scan 2.5s linear infinite;}";
  html += "@keyframes scan{0%{background-position:200% 0;}100%{background-position:-200% 0;}}";
  html += ".statusbar{max-width:560px;margin:0 auto 28px;padding:14px 20px;border-radius:10px;text-align:center;font-weight:600;font-size:14px;letter-spacing:0.5px;border:1px solid var(--track);transition:all .4s ease;}";
  html += ".statusbar.ok{color:var(--ok);box-shadow:0 0 0 1px rgba(45,212,191,0.15) inset;}";
  html += ".statusbar.bad{color:var(--bad);box-shadow:0 0 0 1px rgba(255,92,92,0.15) inset;animation:pulse 1.4s ease-in-out infinite;}";
  html += "@keyframes pulse{0%,100%{opacity:1;}50%{opacity:0.55;}}";
  html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:18px;max-width:560px;margin:0 auto;}";
  html += ".card{background:var(--panel);border:1px solid var(--track);border-radius:14px;padding:22px 12px;display:flex;flex-direction:column;align-items:center;}";
  html += ".ring{width:110px;height:110px;border-radius:50%;display:flex;align-items:center;justify-content:center;position:relative;transition:background 0.6s ease;}";
  html += ".ring-inner{width:82px;height:82px;border-radius:50%;background:var(--panel);display:flex;flex-direction:column;align-items:center;justify-content:center;}";
  html += ".val{font-family:Consolas,Menlo,monospace;font-size:20px;font-weight:700;}";
  html += ".unit{font-size:9px;color:var(--muted);letter-spacing:1px;margin-top:2px;}";
  html += ".label{margin-top:14px;font-size:11px;letter-spacing:1.5px;text-transform:uppercase;color:var(--muted);}";
  html += ".footer{text-align:center;margin-top:32px;font-size:10px;color:var(--track);font-family:Consolas,Menlo,monospace;}";
  html += "</style></head><body>";
  html += "<div class='eyebrow'>ESP32 &middot; LIVE FEED</div>";
  html += "<h1>Toilet Hygiene Monitor</h1>";
  html += "<div class='sub' id='ipline'>tracking air &middot; water &middot; bin level</div>";
  html += "<div class='scanline'></div>";
  html += "<div id='statusbar' class='statusbar ok'>STATUS: OK</div>";
  html += "<div class='grid'>";
  html += "<div class='card'><div class='ring' id='ringGas'><div class='ring-inner'><div class='val' id='gas'>--</div><div class='unit'>PPM-ish</div></div></div><div class='label'>Air Quality</div></div>";
  html += "<div class='card'><div class='ring' id='ringWater'><div class='ring-inner'><div class='val' id='water'>--</div><div class='unit'>RAW</div></div></div><div class='label'>Water Level</div></div>";
  html += "<div class='card'><div class='ring' id='ringBin'><div class='ring-inner'><div class='val' id='dist'>--</div><div class='unit'>CM</div></div></div><div class='label'>Bin Distance</div></div>";
  html += "</div>";
  html += "<div class='footer'>10.184.30.61 &middot; refreshing every 2s</div>";
  html += "<script>";
  html += "function setRing(el,pct,ok){";
  html += "pct=Math.max(0,Math.min(100,pct));";
  html += "const color=ok?'#2dd4bf':'#ff5c5c';";
  html += "el.style.background='conic-gradient('+color+' '+pct+'%, #22303f '+pct+'%)';";
  html += "}";
  html += "async function refresh(){";
  html += "try{";
  html += "const res=await fetch('/data');";
  html += "const d=await res.json();";
  html += "document.getElementById('gas').innerText=d.gas;";
  html += "document.getElementById('water').innerText=d.water;";
  html += "document.getElementById('dist').innerText=d.dist;";
  html += "const gasPct=(d.gas/4095)*100;";
  html += "const gasOk=d.gas<=1800;";
  html += "setRing(document.getElementById('ringGas'),gasPct,gasOk);";
  html += "const waterPct=(d.water/4095)*100;";
  html += "const waterOk=d.water>=500;";
  html += "setRing(document.getElementById('ringWater'),waterPct,waterOk);";
  html += "const binPct=Math.max(0,Math.min(100,(1-(d.dist/30))*100));";
  html += "const binOk=d.dist>=10;";
  html += "setRing(document.getElementById('ringBin'),binPct,binOk);";
  html += "const bar=document.getElementById('statusbar');";
  html += "if(d.alert){bar.className='statusbar bad';bar.innerText='STATUS: ATTENTION NEEDED';}";
  html += "else{bar.className='statusbar ok';bar.innerText='STATUS: OK';}";
  html += "}catch(e){console.log(e);}";
  html += "}";
  html += "refresh();";
  html += "setInterval(refresh,2000);";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"gas\":" + String(gasValue) + ",";
  json += "\"water\":" + String(waterValue) + ",";
  json += "\"dist\":" + String(distanceCM) + ",";
  json += "\"alert\":" + String(alertActive ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 2000) {
    lastRead = millis();
    readSensors();
  }
}