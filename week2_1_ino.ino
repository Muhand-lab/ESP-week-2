#include <WiFi.h>
#include <time.h>
#include "esp_sntp.h"

// 1) Vul dit in:
const char* WIFI_SSID = "Ziggo3799622";
const char* WIFI_PASS = "Fatima2003";

// 2) NTP server (Nederland)
const char* ntpServer = "nl.pool.ntp.org";

// 3) buffer voor tijdtekst
const int glob_buf_size = 64;
char* glob_time_buf;

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
      Serial.println("\nWiFi FAILED (timeout).");
      Serial.println("Tip: check SSID/wachtwoord + gebruik 2.4GHz wifi.");
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

  // Tijdzone NL (CET/CEST)
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
      Serial.println("\nSNTP FAILED (timeout).");
      return;
    }
  }
}

void formatTimeToBuffer(const struct tm* timeinfo) {
  strftime(glob_time_buf, glob_buf_size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("BOOT: setup started");

  glob_time_buf = (char*)malloc(glob_buf_size);
  if (!glob_time_buf) {
    Serial.println("ERROR: malloc failed!");
    while (true) { delay(1000); }
  }

  if (!connectToWifi()) {
    while (true) { delay(1000); }
  }

  SNTP_connect();
  Serial.println("Setup done.");
}

void loop() {
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {
    formatTimeToBuffer(&timeinfo);
    Serial.println(glob_time_buf);
  } else {
    Serial.println("Failed to obtain time");
  }

  delay(2000);
}
