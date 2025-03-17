#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

const char *ssid = "Ngu";             // Tên WiFi
const char *password = "khanhkhanh";  // Mật khẩu WiFi

ESP8266WebServer server(80);
Servo myServo;
#define SERVO_PIN 2  // GPIO2 (D4) nối với Servo

void handleServoControl() {
  if (server.hasArg("action")) {
    String action = server.arg("action");

    if (action == "open") {
      myServo.write(180);  // Mở cửa
      server.send(200, "text/plain", "Cửa ĐÃ MỞ!");
      Serial.println("Mở cửa!");
      delay(5000);       // Giữ cửa mở 5s
      myServo.write(0);  // Đóng cửa lại
      Serial.println("Đóng cửa lại!");
    } else if (action == "close") {
      myServo.write(0);  // Đóng cửa ngay lập tức
      server.send(200, "text/plain", "Cửa ĐÃ ĐÓNG!");
      Serial.println("Đóng cửa!");
    } else {
      server.send(400, "text/plain", "Sai lệnh!");
    }
  } else {
    server.send(400, "text/plain", "Thiếu tham số!");
  }
}

void setup() {
  Serial.begin(115200);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Ban đầu cửa đóng

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");
  Serial.print("Truy cập: http://");
  Serial.println(WiFi.localIP());

  server.on("/door", handleServoControl);  // Đổi tên endpoint cho đúng chức năng
  server.begin();
}

void loop() {
  server.handleClient();
}
