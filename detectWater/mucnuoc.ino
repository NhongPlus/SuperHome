#include <Servo.h>

#define WATER_SENSOR_PIN A0  // Chân analog của cảm biến nước
#define THRESHOLD 1000       // Ngưỡng xác định có nước hay không
#define SERVO_PIN 9          // Chân điều khiển servo

Servo myServo;
bool servoState = false;

void setup() {
    Serial.begin(9600);
    pinMode(WATER_SENSOR_PIN, INPUT);
    myServo.attach(SERVO_PIN);
    myServo.write(0); // Vị trí ban đầu
}

void loop() {
    int waterLevel = analogRead(WATER_SENSOR_PIN);
    Serial.print("Water Level: ");
    Serial.println(waterLevel);
    
    if (waterLevel > THRESHOLD && !servoState) {
        Serial.println("Water detected! Rotating servo.");
        myServo.write(90);  // Quay servo 90 độ khi có nước
        servoState = true;
    } else if (waterLevel <= THRESHOLD && servoState) {
        Serial.println("No water detected. Resetting servo.");
        myServo.write(0);  // Trả servo về vị trí ban đầu
        servoState = false;
    }
    
    delay(1000); // Chờ 1 giây trước lần đọc tiếp theo
}
