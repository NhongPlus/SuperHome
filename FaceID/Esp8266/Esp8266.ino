#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

const char *ssid = "IoT Network";             // Tên WiFi
const char *password = "aiotlab@123";         // Mật khẩu WiFi

// Cấu hình IP tĩnh
IPAddress local_IP(192, 168, 162, 4);         // IP tĩnh cho ESP8266
IPAddress gateway(192, 168, 160, 1);          // Địa chỉ gateway (thường là router)
IPAddress subnet(255, 255, 248, 0);               // Subnet mask
IPAddress dns(8, 8, 8, 8);                    // DNS (Google DNS)

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
    } else if (action == "close") {
      myServo.write(0);  // Đóng cửa ngay lập tức
      server.send(200, "text/plain", "Cửa ĐÃ ĐÓNG!");
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

  // Cấu hình IP tĩnh
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("Cấu hình IP tĩnh thất bại!");
  }

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