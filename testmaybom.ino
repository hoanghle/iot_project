#include <DHT.h>  // Thư viện cho DHT11

#define RELAY_PIN 13   // Relay cho máy bơm
#define RELAY_FAN 14   // Relay cho quạt
#define SOIL_PIN 34  // Chân GPIO 34 nối với A0 của cảm biến
#define LIGHT_PIN 4   // Chân GPIO 35 nối với LDR (cảm biến ánh sáng)
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

bool auto_mode = false;
void setup() {
  Serial.begin(115200);  
  
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

void loop() {
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
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println("°C");
  }

  //Serial.print("Gia tri do am (raw): "); Serial.println(soilValue);
  Serial.print("Do am dat: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  //Serial.print("Gia tri anh sang (raw): ");  Serial.println(lightValue);
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
      digitalWrite(LED_PIN, HIGH);
    }
    else if (lightPercent > light_off){
      Serial.println("Du anh sang, tat den");
      digitalWrite(LED_PIN, LOW);
    }

  }
  else{
    // công tắc điều khiển led
    if (digitalRead(SWITCH_LED) == HIGH) { // Công tắc KCD1 ở vị trí ON
      digitalWrite(LED_PIN, HIGH); // Bật LED
      Serial.println("Bat LED");
    } else { // Công tắc KCD1 ở vị trí OFF
      digitalWrite(LED_PIN, LOW); // Tắt LED
      Serial.println("Tat LED");
    }

    //công tắc đk quạt
    if (digitalRead(SWITCH_FAN) == HIGH){
      digitalWrite(RELAY_FAN, HIGH);
      Serial.println("Bat quat");
    }
    else{
      digitalWrite(RELAY_FAN, LOW);
      Serial.println("Tat quat");
    }

    //công tắc đk máy bơm
    if (digitalRead(SWITCH_SOIL) == HIGH){
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Bat may bom");
    }
    else {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Tat may bom");
    }
  }
  
  delay(3000);

}