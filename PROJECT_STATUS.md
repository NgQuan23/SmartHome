# Project Status

## Goal
Thêm tính năng giám sát mực nước: khi nước ngập >= 5cm thì ngắt rơ le (tắt đèn), khi an toàn (> 5cm) thì bật rơ le (bật đèn) và gửi thông báo về ứng dụng mobile qua Firebase.

## Current Phase
Troubleshooting

## Current Focus
- Xử lý lỗi rơ le bật tắt liên tục (Relay toggling).
- Thiết kế bộ lọc làm mượt dữ liệu cảm biến (Sensor Smoothing) và trễ (Hysteresis).
- Đảm bảo tính ổn định của hệ thống trước khi Re-flash.

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
