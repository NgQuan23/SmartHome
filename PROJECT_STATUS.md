# Project Status

## Goal
Làm cho mức độ báo động (Alerts Threshold) có thể tuỳ chỉnh được từ ứng dụng, đồng bộ trạng thái cấu hình xuống board mạch ESP32 để điều khiển báo động và hiển thị trực quan thông số `<Khoảng cách thực tế>/<Ngưỡng cảnh báo>cm` trên màn hình LCD.

## Current Phase
Done

## Current Focus
- Hoàn thành trích xuất Firebase Realtime Database properties `distance_critical` và `gas_warning`.
- Update `SmartHome-app/lib/views/settings_screen.dart` thành công giúp thanh kéo tương tác mượt mà với trạng thái thật trên Firebase.
- Thay đổi `src/main.cpp` hiển thị thành công LCD string format `D:<Thực tế>/<Cài đặt>cm`.
- Hoàn thành compile-check Firmware thành công bằng `pio run`.

## Affected Capabilities
- Sensor Threshold Customization (ESP32).
- Firmware Display Logic (LCD).
- App Settings sync logic.

## Findings
- Việc lấy cấu hình threshold hoạt động như `getAwayMode` ở nhánh `/devices/device1/settings/*`.
- Cấu hình cũ `DIST_LEVEL_2` và `GAS_LEVEL_2` giờ đây sẽ là fallback khi không tải được Database hoặc bị lỗi ngắt mạng, thiết bị SmartHome vẫn cảnh báo chống nguy hiểm rò rỉ dựa trên con số này.

## Validation
- Project PlatformIO đã Build thành công với firmware logic hoàn toàn mới.
- Flutter Widget Trees đã validate với State update API.

## Risks
- Không có. (Xếp hạng rủi ro thay đổi hành vi cơ bản của Alert Threshold đã được giảm thiểu bởi Fallback static).

## Next Actions
- User cần NẠP LẠI (Re-flash) Code bằng `pio run -t upload`.
- User cần Hot Reload Flutter App (phím `r`) để thử nghiệm kéo Threshold.

## Last Updated
2026-04-15 03:14:00
