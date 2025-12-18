const int BTN1 = 18;
const int BTN2 = 19;
const int LED  = 23;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  Serial.println("START press2 test");
}

void loop() {
  int b1 = digitalRead(BTN1);
  int b2 = digitalRead(BTN2);

  Serial.print("RAW b1=");
  Serial.print(b1);
  Serial.print("  b2=");
  Serial.println(b2);

  digitalWrite(LED, (b1 == LOW && b2 == LOW) ? HIGH : LOW);
  delay(200);
}

