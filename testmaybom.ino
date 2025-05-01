#define BLYNK_TEMPLATE_ID "TMPL6I7LiT0P-"
#define BLYNK_TEMPLATE_NAME "TuoiCay"
#define BLYNK_AUTH_TOKEN "mecpk--FE_rr0y3MTXyVEgj6A92shSrK"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Wi-Fi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tung Lam";      // 🔁 Thay bằng tên Wi-Fi của bạn
char pass[] = "88888888"; // 🔁 Thay bằng mật khẩu Wi-Fi

// Pin định nghĩa
#define RELAY_PIN 13     // Máy bơm
#define RELAY_FAN 14     // Quạt
#define LED_PIN 33       // Đèn LED

#define SOIL_PIN 34      // Độ ẩm đất (Analog)
#define LIGHT_PIN 35      // Cảm biến ánh sáng (Analog)
#define DHT_PIN 15

#define DHT_TYPE DHT11

#define SWITCH_LED 19    
#define SWITCH_FAN 18
#define SWITCH_SOIL 21


DHT dht(DHT_PIN, DHT_TYPE);

// Ngưỡng
const int temp_on = 34;
const int temp_off = 33;
const int light_on = 30;
const int light_off = 50;
const int soil_on = 30;
const int soil_off = 50;

// Biến chế độ
bool auto_mode = false;

// Biến millis
unsigned long lastSendTime = 0;
const long interval = 3000;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_LED, INPUT);
  pinMode(SWITCH_FAN, INPUT);
  pinMode(SWITCH_SOIL, INPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  delay(2000); // Chờ DHT11 khởi động

  Blynk.begin(auth, ssid, pass);
  Serial.println("Khoi dong he thong...");
}

// Blynk V0 - Auto Mode ON/OFF
BLYNK_WRITE(V0) {
  auto_mode = param.asInt();  // 0 hoặc 1
}

void loop() {
  Blynk.run();

  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= interval) {
    lastSendTime = currentTime;

    int soilValue = analogRead(SOIL_PIN);
    int moisturePercent = map(soilValue, 4095, 0, 0, 100);

    int lightValue = analogRead(LIGHT_PIN);
    int lightPercent = map(lightValue, 4095, 0, 0, 100);

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Gửi dữ liệu lên Blynk
    if (!isnan(temperature)) {
      Blynk.virtualWrite(V1, temperature); // Nhiệt độ → V1
      Serial.print("Nhiet do: "); Serial.print(temperature); Serial.println("°C");
    }

    Blynk.virtualWrite(V3, moisturePercent); // Độ ẩm đất → V3
    Serial.print("Do am dat: "); Serial.print(moisturePercent); Serial.println("%");

    Blynk.virtualWrite(V4, lightPercent); // Ánh sáng → V4
    Serial.print("Cuong do anh sang: "); Serial.print(lightPercent); Serial.println("%");

    if (auto_mode) {
      if (temperature > temp_on) {
        Serial.println("Nhiet do cao, bat quat...");
        digitalWrite(RELAY_FAN, HIGH);
      } else if (temperature < temp_off) {
        Serial.println("Nhiet do on, tat quat...");
        digitalWrite(RELAY_FAN, LOW);
      }

      if (moisturePercent < soil_on) {
        Serial.println("Dat kho. Bat may bom...");
        digitalWrite(RELAY_PIN, HIGH);
      } else if (moisturePercent > soil_off) {
        Serial.println("Dat am. Tat may bom...");
        digitalWrite(RELAY_PIN, LOW);
      }

      if (lightPercent < light_on) {
        Serial.println("Troi toi, bat den...");
        digitalWrite(LED_PIN, HIGH);
      } else if (lightPercent > light_off) {
        Serial.println("Troi sang, tat den...");
        digitalWrite(LED_PIN, LOW);
      }
    } else {
      if (digitalRead(SWITCH_LED) == HIGH) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Bat LED (thu cong)");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Tat LED (thu cong)");
      }

      if (digitalRead(SWITCH_FAN) == HIGH) {
        digitalWrite(RELAY_FAN, HIGH);
        Serial.println("Bat quat (thu cong)");
      } else {
        digitalWrite(RELAY_FAN, LOW);
        Serial.println("Tat quat (thu cong)");
      }

      if (digitalRead(SWITCH_SOIL) == HIGH) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("Bat may bom (thu cong)");
      } else {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Tat may bom (thu cong)");
      }
    }
  }
}
