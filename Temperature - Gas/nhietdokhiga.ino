#include <DHT.h>
#include <Servo.h>

#define DHTPIN 2          // Chân DHT11
#define DHTTYPE DHT11
#define MQ_DOUT 3         // Chân DOUT của cảm biến khí gas (Digital)
#define SERVO_PIN 5       // Chân điều khiển Servo
#define BUTTON_PIN 4      // Chân nút nhấn
#define BUZZER_PIN 6      // Chân còi cảnh báo

DHT dht(DHTPIN, DHTTYPE);
Servo myServo;

void setup() {
    Serial.begin(9600);
    dht.begin();
    myServo.attach(SERVO_PIN);
    myServo.write(0);  // Servo bắt đầu ở góc 0 độ
    pinMode(MQ_DOUT, INPUT);  // Đọc tín hiệu digital từ cảm biến khí gas
    pinMode(BUZZER_PIN, OUTPUT);  // Thiết lập chân còi là đầu ra
}

void loop() {
    // Đọc dữ liệu cảm biến
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int gasState = digitalRead(MQ_DOUT); // Đọc trạng thái DOUT (0 hoặc 1)

    Serial.print("Nhiệt độ: "); Serial.print(temp);
    Serial.print("C, Độ ẩm: "); Serial.print(hum);
    Serial.print("%, Khí Gas: "); Serial.println(gasState == LOW ? "Có khí ga" : "Không có");

    // Khi có khí gas (DOUT = LOW, đèn xanh sáng), quay servo
    if (gasState == LOW) {
        myServo.write(90);  
    } else {
        myServo.write(0);  // Khi không có khí gas, quay về 0 độ
    }

    // Cảnh báo khi nhiệt độ vượt quá 40 độ C
    if (temp > 40) {
        digitalWrite(BUZZER_PIN, HIGH);  // Bật còi cảnh báo
    } else {
        digitalWrite(BUZZER_PIN, LOW);   // Tắt còi cảnh báo
    }

    delay(2000); // Chờ một chút trước khi đọc lại
}
