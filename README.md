# Smart Home Security & Monitoring System

Dự án này là hệ thống nhà thông minh giám sát các rò rỉ khí gas, khoảng cách với chướng ngại vật/con người, tích hợp các cơ chế cảnh báo và phản hồi thời gian thực qua Web, Telegram và Blynk (IoT Cloud). 

Dự án được xây dựng cho ESP32-S3 (có thể tương thích với các dòng ESP32 khác) và hỗ trợ cập nhật Firmware OTA.

## 🌟 Các tính năng chính (Current Features)

1. **Giám sát khí Gas (MQ2 Sensor)**
   - Liên tục đánh giá mức độ rò rỉ khí qua Analog pin.
   - 3 Cấp độ báo động:
     - **Mức độ 1 (Low Leak)**: Phát hiện rò rỉ nhẹ, hiển thị lên màn hình.
     - **Mức độ 2 (High Leak)**: Rò rỉ cao. Kích hoạt quạt hút gas (FAN Relay) chạy tự động, hiển thị cảnh báo, báo Telegram và Blynk.
     - **Mức độ 3 (FIRE/CRITICAL)**: Nồng độ báo động cháy. Khóa ngay van cung cấp Gas (VALVE Relay), hú Còi (BUZZER), cảnh báo Telegram & Blynk liên tục.

2. **Cảm biến khi bị ngập nước (Siêu âm HC-SR04)**
   - Cảnh báo khoảng cách an toàn. Tuỳ thuộc vào độ dài (Distance Level), hệ thống sẽ hú còi.
   - Nếu ở mức độ nguy hiểm nhất (bị chắn sát), hệ thống sẽ tự động gửi cảnh báo khẩn và ngắt relay mạch.

3. **Cảm biến chuyển động khi có người lạ xâm nhập (PIR Motion Detection)**
   - Nhận diện sự xuất hiện của người. Gửi tín hiệu báo động lên màn hình và hú còi nhẹ 1 lần, ghi nhận hoạt động.

4. **Hiển thị trực quan (LCD I2C 16x2)**
   - Cung cấp cái nhìn trực tiếp từ các cảm biến qua màn hình.

5. **Lưu trữ & Đồng bộ đa nền tảng**
   - Đẩy thông số (Telemetry) liên tục đến **Blynk Cloud**, **Firebase** và **MQTT Broker** (HiveMQ).
   - *Tính năng Offline Sync*: Nếu mất kết nối trong ngắn hạn, dữ liệu sẽ được trữ tạm vào Flash memory (`storageAppend()`) và đồng bộ ngược lên máy chủ (`storageFlush()`) ngay sau khi có mạng.

6. **Telegram Notification Bot**
   - Đẩy thẳng tin nhắn tới tài khoản Telegram hoặc Group Chat khi có cảnh báo nguy hiểm (Cháy, Vượt ngưỡng Gas, Vật cản quá gần).

## ⚙️ Cấu hình "New Blynk" (Blynk IoT)

Hệ thống đã được chuyển đổi tương thích với **New Blynk Cloud (blynk.cloud)**. Bạn không cần kết nối tới Local Server hay blynk-cloud.com cũ nữa. 

### Cách Setup phần mềm
Mở file `src/config.h` và chỉnh sửa các tham số sau theo thông tin trên Console của Blynk IoT (https://blynk.cloud):

```c
// Lấy 3 thông số này trên giao diện Info của Device trên Blynk Cloud
#define BLYNK_TEMPLATE_ID "TMPL_xxxxx"
#define BLYNK_TEMPLATE_NAME "Smart Home Device"
#define BLYNK_AUTH_TOKEN "Your_Blynk_Auth_Token"
```

**Lưu ý quan trọng**:
- Code sử dụng cơ chế kết nối Background không đồng bộ (`Blynk.config` kết hợp `Blynk.run()`) giúp không bị "đóng băng" (block) tiến trình trong thời gian mất mạng hoặc chờ phản hồi từ Firebase/Telegram.
- Hãy đảm bảo Datasream trên Blynk Cloud tương thích với các logic Virtual Pin hiện tại:
  - `V0`: Gas Value (integer)
  - `V1`: Distance (float / int)
  - `V2`: Motion (1/0)
  - `V3`: Thông báo dạng chữ (String)
  - `V4`: Distance Level (1,2,3)

## 📡 Cấu hình các Dịch vụ khác

### Firebase Realtime Database
Trong `config.h`, firmware ESP32 chỉ dùng trực tiếp các trường RTDB/auth sau:
```c
#define FIREBASE_DATABASE_URL "https://smart-home-1c235-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_PROJECT_ID "smart-home-1c235"
#define FIREBASE_API_KEY "AIzaSyxxxxxx"
#define FIREBASE_USER_EMAIL "email@gmail.com"
#define FIREBASE_USER_PASSWORD "password123"
```

Lưu ý:
- `authDomain`, `storageBucket`, `appId`, `messagingSenderId`, `measurementId` là cấu hình cho Web SDK; firmware ESP32 hiện không dùng các trường đó.
- Nếu `Firebase Authentication` chưa bật phương thức `Email/Password` hoặc user đăng nhập không hợp lệ, firmware sẽ không lấy được token và Serial sẽ báo lỗi kiểu `CONFIGURATION_NOT_FOUND`.

### Telegram Bot
Dùng [BotFather](https://t.me/BotFather) để tạo bot và lấy `TG_BOT_TOKEN`. Lấy Chat ID của bạn (qua @userinfobot) và cập nhật:
```c
#define TG_BOT_TOKEN "123456789:ABCDefgh..."
#define TG_CHAT_ID "0987654321"
```

## 🛠 Cách nạp Code
Dự án được quản lý qua **PlatformIO**. 
Bạn có thể cài đặt extension PlatformIO trên VSCode. Sau đó, kết nối mạch ESP32 (ở đây mình dùng ESP32-S3). Mở terminal và gõ:
```bash
pio run -e esp32-s3-devkitc-1 -t upload
```

_Bản cập nhật hiện tại đã vô hiệu hoá Deep Sleep nhằm đảm bảo tính toàn vẹn của Serial Port và duy trì Blynk Server không bị disconnect_._

## 🔌 Pin map cho ESP32-S3 Super Mini

Sơ đồ chân firmware hiện tại được chuẩn hóa cho board `ESP32-S3 Super Mini` như sau:

| Thiết bị | Chân trên cảm biến/module | GPIO ESP32-S3 Super Mini | Ghi chú |
|---|---|---|---|
| MQ2 | `AO` | `GPIO4` | Chỉ đưa tín hiệu analog tối đa 3.3V vào ESP32-S3. Nếu module MQ2 chạy 5V, cần chia áp cho `AO`. |
| PIR | `OUT` | `GPIO5` | Mức logic vào phải là 3.3V-safe. |
| HC-SR04 | `TRIG` | `GPIO6` | Output từ ESP32-S3 sang cảm biến. |
| HC-SR04 | `ECHO` | `GPIO7` | Bắt buộc qua cầu chia áp/level shifter xuống 3.3V trước khi vào ESP32-S3. |
| LCD I2C 16x2 | `SDA` | `GPIO8` | I2C được remap trong firmware. |
| LCD I2C 16x2 | `SCL` | `GPIO18` | Tránh dùng `GPIO9` để không đụng vùng chân nhạy cảm của board. |
| Buzzer | `SIG` | `GPIO15` | Dùng output số. |
| Relay quạt / relay chính | `IN` | `GPIO16` | Hiện firmware dùng chân này cho `PIN_RELAY` và `PIN_FAN_RELAY`. |
| Relay van gas | `IN` | `GPIO17` | Output số riêng cho van gas. |

Khuyến nghị nguồn:
- Cấp `3.3V` cho các tín hiệu logic đi vào ESP32-S3.
- Nếu dùng module relay, MQ2 hoặc LCD chạy `5V`, chỉ cho phép phần `VCC` dùng `5V`; các chân tín hiệu vào ESP32-S3 vẫn phải bảo đảm mức `3.3V`.
