import signal
import sys
import requests
import cv2
import time
import numpy as np
import os
from urllib.request import urlopen

# Thư mục chứa ảnh của chủ nhân
OWNER_IMAGE_DIR = "owner_images/"  # Lưu ảnh nhận diện chủ nhân

# URL của ESP32-CAM và ESP8266
capture_url = "http://172.20.10.7/capture"  # Địa chỉ lấy ảnh từ ESP32-CAM
esp8266_door_url = "http://172.20.10.6/door"  # Địa chỉ điều khiển cửa từ ESP8266

# Biến trạng thái
door_open = False  # Trạng thái cửa (mở/đóng)
last_detection_time = 0  # Lần cuối nhận diện chủ nhân

# Load mô hình nhận diện khuôn mặt của OpenCV
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

# Tải tất cả ảnh chủ nhân từ thư mục để làm dữ liệu nhận diện
owner_faces = []
for file in os.listdir(OWNER_IMAGE_DIR):
    if file.lower().endswith(".jpg"):  # Chỉ lấy file .jpg
        img_path = os.path.join(OWNER_IMAGE_DIR, file)
        img = cv2.imread(img_path)
        gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)  # Chuyển sang ảnh xám
        faces = face_cascade.detectMultiScale(gray_img, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

        if len(faces) > 0:
            x, y, w, h = faces[0]
            owner_faces.append(gray_img[y:y+h, x:x+w])  # Lưu khuôn mặt vào danh sách

# Kiểm tra xem có khuôn mặt chủ nhân nào được tải lên không
if len(owner_faces) == 0:
    print("[ERROR] Không tìm thấy khuôn mặt trong ảnh chủ nhân!")
    sys.exit(1)

print(f"[INFO] Đã tải {len(owner_faces)} ảnh khuôn mặt chủ nhân.")

# Hàm đóng cửa khi cần thiết
def close_door():
    global door_open
    if door_open:
        try:
            requests.get(f"{esp8266_door_url}?action=close", timeout=1)
            print("CỬA ĐÃ ĐÓNG - Gửi tín hiệu đến ESP8266 khi thoát")
            door_open = False
        except requests.RequestException:
            print("Không thể kết nối với ESP8266 để đóng cửa khi thoát")

# Xử lý sự kiện khi thoát chương trình
def handle_exit(signum, frame):
    print("\n[!] Chương trình bị đóng! Đóng cửa trước khi thoát...")
    close_door()
    cv2.destroyAllWindows()
    sys.exit(0)

# Đăng ký xử lý sự kiện thoát
signal.signal(signal.SIGINT, handle_exit)  # Bắt Ctrl + C
signal.signal(signal.SIGTERM, handle_exit)  # Bắt sự kiện kill process

# Chạy vòng lặp chính
try:
    while True:
        try:
            # Lấy ảnh từ ESP32-CAM
            response = urlopen(capture_url)
            img_array = np.asarray(bytearray(response.read()), dtype="uint8")
            frame = cv2.imdecode(img_array, -1)

            # Chuyển ảnh sang grayscale để nhận diện
            gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

            is_owner_detected = False
            for (x, y, w, h) in faces:
                detected_face = gray_frame[y:y+h, x:x+w]

                # So sánh với tất cả ảnh chủ nhân đã lưu
                for owner_face in owner_faces:
                    result = cv2.matchTemplate(detected_face, owner_face, cv2.TM_CCOEFF_NORMED)
                    similarity = result[0][0]  # Độ tương đồng (giá trị từ -1 đến 1)

                    if similarity > 0.8:  # Ngưỡng nhận diện
                        is_owner_detected = True
                        last_detection_time = time.time()
                        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
                        cv2.putText(frame, "Owner Detected", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                        break  # Nếu đã tìm thấy chủ nhân, dừng kiểm tra

            # Nếu phát hiện chủ nhân và cửa đang đóng, gửi tín hiệu mở cửa
            if is_owner_detected and not door_open:
                try:
                    requests.get(f"{esp8266_door_url}?action=open", timeout=1)
                    door_open = True
                    print("CỬA ĐÃ MỞ - Chủ nhân được nhận diện")
                except requests.RequestException:
                    print("Không thể kết nối với ESP8266 để mở cửa")

            # Nếu sau 5 giây không thấy chủ nhân, đóng cửa
            if time.time() - last_detection_time > 5 and door_open:
                close_door()

            # Hiển thị video nhận diện
            cv2.imshow("Face Recognition", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        except Exception as e:
            print("Lỗi:", e)
            time.sleep(1)  # Nếu lỗi, chờ 1 giây rồi thử lại

finally:
    print("Đóng cửa và thoát...")
    close_door()
    cv2.destroyAllWindows()