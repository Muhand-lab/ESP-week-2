#include <WiFi.h>
#include <time.h>
#include "esp_sntp.h"

/* ====== Pas dit aan ====== */
const char* WIFI_SSID = "Ziggo3799622";
const char* WIFI_PASS = "Fatima2003";

/* ====== Hardware pins ====== */
const int BTN1 = 18;   // knop 1 naar GND
const int BTN2 = 19;   // knop 2 naar GND
const int LED  = 23;   // LED + weerstand naar GND

/* ====== NTP ====== */
const char* ntpServer = "nl.pool.ntp.org";

/* ====== Debounce ====== */
const unsigned long DEBOUNCE_MS = 20;
int stable1 = HIGH, stable2 = HIGH;
int lastRead1 = HIGH, lastRead2 = HIGH;
unsigned long lastChange1 = 0, lastChange2 = 0;

int readDebounced(int pin, int &stable, int &lastRead, unsigned long &lastChange) {
  int r = digitalRead(pin);
  if (r != lastRead) {
    lastRead = r;
    lastChange = millis();
  }
  if (millis() - lastChange > DEBOUNCE_MS) {
    stable = lastRead;
  }
  return stable;
}

/* ====== Queue (audit) van 200 timestamps ====== */
static const int QSIZE = 200;
static const int TS_BUF_SIZE = 32; // "YYYY-MM-DD HH:MM:SS" past

char audit[QSIZE][TS_BUF_SIZE];
int q_head = 0;   // index van oudste
int q_count = 0;  // hoeveel items zitten erin

void queuePush(const char* ts) {
  int idx = (q_head + q_count) % QSIZE;

  // als vol: overschrijf oudste -> head opschuiven
  if (q_count == QSIZE) {
    q_head = (q_head + 1) % QSIZE;
    idx = (q_head + q_count - 1) % QSIZE; // plek van “nieuwste”
  } else {
    q_count++;
  }

  strncpy(audit[idx], ts, TS_BUF_SIZE - 1);
  audit[idx][TS_BUF_SIZE - 1] = '\0';
}

void printQueueInfo() {
  Serial.print("Audit queue count = ");
  Serial.println(q_count);
  if (q_count > 0) {
    Serial.print("Oldest = ");
    Serial.println(audit[q_head]);
    int newest = (q_head + q_count - 1) % QSIZE;
    Serial.print("Newest = ");
    Serial.println(audit[newest]);
  }
}

/* ====== WiFi + SNTP ====== */
bool connectToWifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - start > 20000) {
      Serial.println("\nWiFi FAILED (timeout)");
      return false;
    }
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void SNTP_connect() {
  const time_t old_past = 1577836800; // 2020-01-01
  Serial.println("Starting SNTP...");

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char*)ntpServer);
  sntp_init();

  // Tijdzone NL
  setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
  tzset();

  unsigned long start = millis();
  while (true) {
    delay(500);
    time_t now;
    time(&now);

    if (now >= old_past) {
      Serial.println("SNTP time synced!");
      return;
    }

    Serial.print(".");
    if (millis() - start > 20000) {
      Serial.println("\nSNTP FAILED (timeout)");
      return;
    }
  }
}

bool getTimestamp(char* out, int outSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;
  strftime(out, outSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
  return true;
}

/* ====== Pers logica ====== */
bool pressActivePrev = false;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("week2-3 started");

  pinMode(LED, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  if (!connectToWifi()) {
    Serial.println("Stop: WiFi not connected.");
    while (true) delay(1000);
  }
  SNTP_connect();
  Serial.println("Setup done.");
}

void loop() {
  int b1 = readDebounced(BTN1, stable1, lastRead1, lastChange1);
  int b2 = readDebounced(BTN2, stable2, lastRead2, lastChange2);

  bool pressed1 = (b1 == LOW);
  bool pressed2 = (b2 == LOW);
  bool pressActive = (pressed1 && pressed2);

  // LED aan/uit
  digitalWrite(LED, pressActive ? HIGH : LOW);

  // Alleen loggen bij overgang UIT->AAN (rising edge)
  if (pressActive && !pressActivePrev) {
    char ts[TS_BUF_SIZE];
    if (getTimestamp(ts, sizeof(ts))) {
      queuePush(ts);
      Serial.print("PRESS ACTIVATED at: ");
      Serial.println(ts);
    } else {
      Serial.println("PRESS ACTIVATED but time not available");
    }
    printQueueInfo();
  }

  pressActivePrev = pressActive;
  delay(10);
}
