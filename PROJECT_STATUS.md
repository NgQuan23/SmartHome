# Project Status

## Goal
Hoàn tất cấu hình Firebase cho project `smart-home-1c235`, flash firmware lên ESP32-S3 và xác minh board có xác thực được, ghi telemetry được, và trả tín hiệu sống lên Firebase Realtime Database.

## Current Phase
Done

## Current Focus
- Bật `Email/Password` trong Firebase Authentication
- Set RTDB rules chỉ cho phép đúng firmware user ghi/đọc `/devices/device1`
- Xác minh end-to-end bằng serial runtime và REST read sau khi board ghi dữ liệu

## Affected Capabilities
- Cloud telemetry
- Offline sync

## Findings
- `src/config.h` đã được chỉnh sang URL RTDB đúng theo web config: `https://smart-home-1c235-default-rtdb.asia-southeast1.firebasedatabase.app`.
- Firmware đã được thêm heartbeat/status path riêng tại `/devices/device1/status` để dễ xác nhận board online khi Firebase write thành công.
- Firebase Console đã bật `Authentication > Email/Password`.
- Firebase Auth đã có user cho firmware: `quanylksnb@gmail.com` với UID `NckGu46OedOFjLRjI8281xsGeAA3`.
- RTDB rules đã được publish theo UID để chỉ user trên được đọc/ghi `/devices/device1`.
- Sau khi flash lại, serial cho thấy Firebase token ở trạng thái `ready`, telemetry push thành công, và status heartbeat được cập nhật.
- REST read bằng ID token hợp lệ xác nhận `/devices/device1/status` và `/devices/device1/telemetry` đều có dữ liệu thật từ board.

## Validation
- `pio run -e esp32-s3-devkitc-1` -> PASS
- `pio run -e esp32-s3-devkitc-1 -t upload --upload-port /dev/cu.usbmodem1101` -> PASS
- Firebase Console check bằng Chrome MCP:
  - `Authentication > Email/Password` -> Enabled
  - User firmware đã được tạo thành công
  - RTDB rules đã save/publish thành công
- Serial runtime check trên `/dev/cu.usbmodem1101`:
  - `WiFi connected successfully. IP: 192.168.2.41`
  - `Blynk: cloud session established`
  - `Firebase token: type=id token status=ready`
  - `Firebase initialized`
  - `Firebase: telemetry pushed`
  - `Firebase: status heartbeat updated`
- REST auth check:
  - `POST https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?...` -> PASS
- RTDB authenticated read check:
  - `/devices/device1/status` -> PASS
  - `/devices/device1/telemetry` -> PASS
- Dữ liệu xác minh sau khi board chạy:
  - `status.connected = true`
  - `status.ip = 192.168.2.41`
  - `status.rssi = -66`
  - `telemetryCount = 212`
- Build hiện tại sử dụng khoảng 16.3% RAM và 98.4% flash partition.

## Risks
- `src/config.h` hiện chứa Wi-Fi credentials thật trong working tree, không nên commit/push nguyên trạng nếu repo dùng chung.
- Firmware đang rất sát giới hạn flash partition hiện tại.
- Chưa có xác minh đầy đủ hành vi sensor/relay ngoài việc board đã ghi được telemetry và heartbeat lên Firebase.

## Next Actions
- Nếu tiếp tục phát triển, nên tách secrets ra khỏi `src/config.h` trước khi commit/push.
- Nếu cần harden thêm, có thể tách mỗi thiết bị sang một UID/path riêng thay vì dùng chung một firmware account.
- Kiểm tra manual trên board thật cho gas/PIR/relay/buzzer để chắc payload Firebase phản ánh đúng trạng thái cảm biến.

## Last Updated
2026-04-05 18:37:00
