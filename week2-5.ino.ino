#include <Arduino.h>

const int LED_PIN = 23;
const int BTN1 = 18;
const int BTN2 = 19;

SemaphoreHandle_t semBlink;

// 10x knipperen (beschermd door semaphore)
void blink_sem_led() {
  xSemaphoreTake(semBlink, portMAX_DELAY);

  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(150));
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(150));
  }

  xSemaphoreGive(semBlink);
}

void taskBlink(void *pv) {
  blink_sem_led();
  vTaskDelete(NULL);
}

// “1 druk” detectie (edge detect)
bool pressedOnce(int pin, int &lastState) {
  int now = digitalRead(pin);              // INPUT_PULLUP: los=HIGH, gedrukt=LOW
  bool pressed = (lastState == HIGH && now == LOW);
  lastState = now;
  return pressed;
}

int last1 = HIGH;
int last2 = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  // Binary semaphore maken + vrijgeven (belangrijk!)
  semBlink = xSemaphoreCreateBinary();
  xSemaphoreGive(semBlink);

  Serial.println("Opdracht 5 started");
}

void loop() {
  if (pressedOnce(BTN1, last1)) {
    Serial.println("BTN1 pressed -> request blink");
    xTaskCreate(taskBlink, "blink1", 2048, NULL, 1, NULL);
  }

  if (pressedOnce(BTN2, last2)) {
    Serial.println("BTN2 pressed -> request blink");
    xTaskCreate(taskBlink, "blink2", 2048, NULL, 1, NULL);
  }

  delay(10);
}
