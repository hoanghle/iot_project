#define BLYNK_TEMPLATE_ID "TMPL6I7LiT0P-"
#define BLYNK_TEMPLATE_NAME "TuoiCay"
#define BLYNK_AUTH_TOKEN "mecpk--FE_rr0y3MTXyVEgj6A92shSrK"

#include <WiFi.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>

// WiFi và Blynk cấu hình
char auth[] = BLYNK_AUTH_TOKEN;  // <- THAY bằng token từ Blynk
char ssid[] = "Tung Lam";              // <- THAY bằng WiFi
char pass[] = "88888888";          // <- THAY bằng mật khẩu WiFi

#define RELAY_PIN 13   // Relay cho máy bơm
#define RELAY_FAN 14   // Relay cho quạt
#define SOIL_PIN 34  // Chân GPIO 34 nối với A0 của cảm biến
#define LIGHT_PIN 35   // Chân GPIO 35 nối với LDR (cảm biến ánh sáng)
#define LED_PIN 33      // Chân GPIO 5 nối với LED

#define SWITCH_LED 19  // công tắc đèn
#define SWITCH_FAN 18 //Công tắc quạt
#define SWITCH_SOIL 21 //Công tắc máy bơm

#define DHT_PIN 15     // Chân GPIO 15 nối với DATA của DHT11
#define DHT_TYPE DHT11 // Loại cảm biến (DHT11)

DHT dht(DHT_PIN, DHT_TYPE);  // Khởi tạo cảm biến DHT11
const int temp_on = 34;   // Bật quạt ở 35°C
const int temp_off = 33;
const int light_on = 30;  // Bật đèn khi ánh sáng < 30% (trời tối)
const int light_off = 50; // Tắt đèn khi ánh sáng > 50% (trời sáng)
const int soil_on = 30; // Bật máy bơm khi độ ẩm đất < 30% (đất khô)
const int soil_off = 50; // Tắt máy bơm khi độ ẩm đất > 50% (đất đủ ẩm)


// Trạng thái điều khiển
bool auto_mode = true;
bool led_manual = false;
bool fan_manual = false;
bool pump_manual = false;


// Nhận dữ liệu từ Blynk
BLYNK_WRITE(V0) {
  auto_mode = param.asInt();
  Serial.print("Chuyen che do tu dong: ");
  Serial.println(auto_mode ? "BAT" : "TAT");
}

BLYNK_WRITE(V5) {
  led_manual = param.asInt();
  Serial.print("Dieu khien LED thu cong: ");
  Serial.println(led_manual ? "BAT" : "TAT");
}

BLYNK_WRITE(V6) {
  fan_manual = param.asInt();
  Serial.print("Dieu khien QUAT thu cong: ");
  Serial.println(fan_manual ? "BAT" : "TAT");
}

BLYNK_WRITE(V7) {
  pump_manual = param.asInt();
  Serial.print("Dieu khien MAY BOM thu cong: ");
  Serial.println(pump_manual ? "BAT" : "TAT");
}

void setup() {
  Serial.begin(115200);  
  Blynk.begin(auth, ssid, pass);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_LED, INPUT);  
  pinMode(SWITCH_FAN, INPUT);
  pinMode(SWITCH_SOIL, INPUT);

  digitalWrite(RELAY_PIN, LOW);  // Tắt máy bơm ban đầu 
  digitalWrite(RELAY_FAN, LOW);  
  digitalWrite(LED_PIN, LOW);    

  dht.begin();
  delay(2000); // Chờ 2 giây để DHT11 khởi động
  Serial.println("Khoi dong he thong...");
}

unsigned long lastSensorRead = 0;        
const long sensorInterval = 3000;

void loop() {
  Blynk.run();

   unsigned long currentMillis = millis(); 

  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;

    running();
  }

  delay(3000);

}

void running(){
  int soilValue = analogRead(SOIL_PIN);  // Đọc giá trị analog (0-4095)
  int moisturePercent = map(soilValue, 4095, 0, 0, 100);  // Chuyển thành phần trăm (khô: 0%, ướt: 100%)
  
  // Đọc giá trị ánh sáng
  int lightValue = analogRead(LIGHT_PIN);  // Đọc giá trị analog (0-4095)
  int lightPercent = map(lightValue, 4095, 0, 0, 100);  // Chuyển thành phần trăm (tối: 0%, sáng: 100%)

  // Đọc giá trị từ cảm biến nhiệt độ DHT11
  float humidity = dht.readHumidity();      // Đọc độ ẩm không khí (%)
  float temperature = dht.readTemperature(); // Đọc nhiệt độ (độ C)
  
  // Kiểm tra nếu đọc lỗi
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Loi doc cam bien DHT11!");
  } else {
    // In giá trị DHT11
    //Serial.print("Do am khong khi: "); Serial.print(humidity); Serial.println("%"); 
    Blynk.virtualWrite(V1, temperature);
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println("°C");
  }

  //Serial.print("Gia tri do am (raw): "); Serial.println(soilValue);
  Blynk.virtualWrite(V3, moisturePercent);
  Serial.print("Do am dat: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  //Serial.print("Gia tri anh sang (raw): ");  Serial.println(lightValue);
  Blynk.virtualWrite(V4, lightPercent);
  Serial.print("Cuong do anh sang: ");
  Serial.print(lightPercent);
  Serial.println("%");

  if (auto_mode == true){
    //bật tắt quạt tự động 
    if (temperature > temp_on) { 
      Serial.println("Nhiet do cao, bat quat...");
      digitalWrite(RELAY_FAN, HIGH); 
    } else if (temperature < temp_off) { 
      Serial.println("Nhiet do on, tat quat...");
      digitalWrite(RELAY_FAN, LOW); 
    }

    //bật tắt máy bơm tự động
    if (moisturePercent < soil_on){
      Serial.println("Dat kho. Bat may bom...");
      digitalWrite(RELAY_PIN, HIGH);
    }
    else if (moisturePercent > soil_off){
      Serial.println("Dat am. Tat may bom...");
      digitalWrite(RELAY_PIN, LOW);
    }

    //bật tắt đèn tự động
    if (lightPercent < light_on){
      Serial.println("Troi am u, bat den...");
      digitalWrite(LED_PIN, LOW);  
    }
    else if (lightPercent > light_off){
      Serial.println("Du anh sang, tat den");
      digitalWrite(LED_PIN, HIGH);
    }
        // Gửi trạng thái các công tắc lên Blynk (để đồng bộ giao diện)
    Blynk.virtualWrite(V5, digitalRead(SWITCH_LED));  // LED
    Blynk.virtualWrite(V6, digitalRead(SWITCH_FAN));  // Quạt
    Blynk.virtualWrite(V7, digitalRead(SWITCH_SOIL)); // Máy bơm

  }
  else{
    // công tắc điều khiển led
    if (digitalRead(SWITCH_LED) == HIGH) { // Công tắc KCD1 ở vị trí ON
      digitalWrite(LED_PIN, LOW); // Bật LED
      Blynk.virtualWrite(V5, 1); 
      Serial.println("Bat LED");
    } else { // Công tắc KCD1 ở vị trí OFF
      digitalWrite(LED_PIN, HIGH); // Tắt LED
      Blynk.virtualWrite(V5, 0); 
      Serial.println("Tat LED");
    }

    //công tắc đk quạt
    if (digitalRead(SWITCH_FAN) == HIGH){
      digitalWrite(RELAY_FAN, HIGH);
      Blynk.virtualWrite(V6, 1);  
      Serial.println("Bat quat");
    }
    else{
      digitalWrite(RELAY_FAN, LOW);
       Blynk.virtualWrite(V6, 0);  
      Serial.println("Tat quat");
    }

    //công tắc đk máy bơm
    if (digitalRead(SWITCH_SOIL) == HIGH){
      digitalWrite(RELAY_PIN, HIGH);
      Blynk.virtualWrite(V7, 1);   
      Serial.println("Bat may bom");
    }
    else {
      digitalWrite(RELAY_PIN, LOW);
      Blynk.virtualWrite(V7, 0);   
      Serial.println("Tat may bom");
    }
  }
}