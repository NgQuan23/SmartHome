# PRD - Smart Home Security & Monitoring System

## Document Control

- Status: Draft v1
- Date: 2026-04-05
- Product Scope: Embedded IoT safety monitoring for home environments
- Primary Repo Scope: ESP32-S3 firmware with cloud alerting and telemetry integrations

## 1. Executive Summary

Smart Home Security & Monitoring System là hệ thống giám sát an toàn nhà ở được vận hành bởi ESP32-S3, kết hợp cảm biến gas MQ2, cảm biến chuyển động PIR và cảm biến siêu âm HC-SR04 để phát hiện sớm các tình huống nguy hiểm và phản ứng theo thời gian thực.

Sản phẩm nhằm giải quyết 3 nhóm rủi ro chính trong gia đình:
- Rò rỉ khí gas và nguy cơ cháy nổ.
- Vật cản hoặc con người ở khoảng cách nguy hiểm trong khu vực cần theo dõi.
- Chuyển động bất thường cần ghi nhận và cảnh báo.

Phiên bản hiện tại của repo tập trung vào firmware và các tích hợp cảnh báo hoặc telemetry: LCD, buzzer, relay, Wi-Fi, MQTT, Firebase Realtime Database, Blynk, Telegram, OTA và cơ chế lưu tạm dữ liệu offline bằng LittleFS. PRD này mô tả phạm vi sản phẩm thực tế theo hiện trạng codebase, đồng thời tách rõ những hạng mục chưa được triển khai đầy đủ nhưng có thể thuộc roadmap tiếp theo.

## 2. Product Summary

### 2.1 Product Goal

Xây dựng thiết bị giám sát an toàn nhà thông minh có khả năng:
- Phát hiện sự cố tại chỗ.
- Kích hoạt cơ chế cảnh báo và phản ứng tự động ngay trên thiết bị.
- Gửi cảnh báo đến người dùng qua các kênh từ xa.
- Đồng bộ telemetry để giúp theo dõi tình trạng hệ thống.
- Hoạt động ổn định trong điều kiện mất mạng ngắn hạn và tự động đồng bộ lại khi có kết nối.

### 2.2 Current Product Reality In Repo

Những thành phần đã được thể hiện rõ trong repo:
- Firmware PlatformIO cho ESP32-S3, framework Arduino.
- Gas monitoring với 3 ngưỡng cảnh báo.
- Proximity monitoring bằng HC-SR04.
- Motion detection bằng PIR.
- Hiển thị trạng thái qua LCD I2C.
- Actuator control qua buzzer và relay.
- Gửi telemetry lên MQTT, Firebase RTDB và Blynk.
- Gửi cảnh báo qua Telegram và Blynk.
- OTA update khi có Wi-Fi.
- Cơ chế queue offline trên LittleFS và flush khi mạng hồi phục.

Những thành phần chưa đủ bằng chứng triển khai đầy đủ trong repo:
- Web dashboard riêng.
- Mobile app riêng.
- Remote control qua MQTT command topic.
- Quản lý nhiều thiết bị, nhiều người dùng.
- Backend Data Connect phù hợp domain Smart Home.

## 3. Problem Statement

Hộ gia đình và người quản lý thiết bị cần một hệ thống nhỏ gọn, chi phí hợp lý nhưng vẫn có khả năng phát hiện sự cố sớm và thông báo kịp thời. Nếu chỉ dựa vào cảnh báo tại chỗ, người dùng có thể bỏ lỡ tình huống nguy hiểm khi vắng nhà. Nếu chỉ dựa vào cloud, hệ thống sẽ dễ bị mất khả năng xử lý khi mạng không ổn định.

Sản phẩm cần đảm bảo:
- Có phản ứng tại chỗ ngay lập tức.
- Có thông báo từ xa khi internet sẵn sàng.
- Không mất hoàn toàn dữ liệu sự kiện khi kết nối gián đoạn ngắn hạn.

## 4. Target Users

### 4.1 Primary Users

- Homeowner: người sở hữu nhà, muốn nhận cảnh báo từ xa khi có sự cố.
- Family operator: thành viên gia đình cần xem tình trạng cơ bản của hệ thống và được thông báo khi cần.
- Installer/maintainer: người lắp đặt, cấu hình ngưỡng, kênh thông báo và cập nhật firmware.

### 4.2 User Needs

- Biết sớm khi có rò rỉ gas, vật cản nguy hiểm hoặc chuyển động đáng nghi.
- Nhận thông báo từ xa mà không cần lúc nào cũng đứng cạnh thiết bị.
- Tin cậy rằng dữ liệu và cảnh báo không bị mất ngay khi Wi-Fi chập chờn.
- Có cách bảo trì, cập nhật và cấu hình hệ thống dễ dàng.

## 5. Goals And Success Metrics

### 5.1 Business Goals

- Tạo một MVP an toàn nhà thông minh có thể demo, lắp đặt và mở rộng tiếp.
- Chứng minh khả năng kết hợp local safety response và remote telemetry trong cùng một thiết bị.
- Giảm rủi ro bỏ sót sự cố so với giải pháp không có cảnh báo từ xa.

### 5.2 Product Success Metrics

- Local sensor polling được thực hiện theo chu kỳ ngắn, mục tiêu <= 1 giây cho phát hiện và xử lý vòng lặp.
- Sự cố gas mức cao hoặc critical tạo cảnh báo local ngay trong chu kỳ đọc cảm biến hiện tại.
- Khi Wi-Fi hoạt động, cảnh báo Telegram hoặc Blynk được đẩy đi trong thời gian hợp lý sau khi sự cố xảy ra.
- Telemetry thất bại do mất mạng được xếp hàng vào bộ nhớ local và đồng bộ lại sau khi kết nối phục hồi.
- OTA update có thể khởi tạo khi thiết bị đang online và không làm hỏng vòng lặp giám sát cơ bản.

## 6. Scope

### 6.1 In Scope For Current Product

- Firmware cho ESP32-S3.
- Detection và alerting cho gas, motion và proximity.
- Local UI hoặc UX trên LCD và cảnh báo âm thanh.
- Relay-based automated response.
- Telemetry transport qua MQTT, Firebase RTDB và Blynk.
- Telegram hoặc Blynk notification.
- Offline event buffering.
- OTA firmware update.
- Manual configuration thông qua file cấu hình firmware.

### 6.2 Out Of Scope For Current Release

- Ứng dụng web hoặc mobile đầy đủ có login và dashboard lịch sử.
- Multi-device fleet management.
- Machine learning hoặc predictive analytics.
- Audio hoặc video surveillance.
- Auto provisioning tại hiện trường.
- Quản lý user hoặc account trên backend.
- Data Connect schema hiện tại, vì nội dung schema không phù hợp domain Smart Home.

## 7. Hardware Target And Pin Mapping

### 7.1 Board Target For Current Firmware

- Target board: ESP32-S3 Super Mini.
- PlatformIO environment: `esp32-s3-devkitc-1` với board definition `esp32-s3-mini`.
- Firmware phải dùng pin map tập trung trong `src/config.h` để tránh hard-code pin trong `src/main.cpp`.

### 7.2 Pin Mapping For Sensor And Actuator Wiring

| Module | Chân module | GPIO ESP32-S3 Super Mini | Firmware macro | Purpose |
|---|---|---|---|---|
| MQ2 gas sensor | `AO` | `GPIO4` | `PIN_MQ2` | Đọc analog nồng độ gas |
| PIR motion sensor | `OUT` | `GPIO5` | `PIN_PIR` | Phát hiện chuyển động |
| HC-SR04 | `TRIG` | `GPIO6` | `PIN_TRIG` | Phát xung đo khoảng cách |
| HC-SR04 | `ECHO` | `GPIO7` | `PIN_ECHO` | Đọc xung phản hồi khoảng cách |
| LCD I2C 16x2 | `SDA` | `GPIO8` | `PIN_I2C_SDA` | Dữ liệu I2C cho LCD |
| Buzzer | `SIG` | `GPIO15` | `PIN_BUZZER` | Còi cảnh báo tại chỗ |
| Relay quạt / relay chính | `IN` | `GPIO16` | `PIN_RELAY` / `PIN_FAN_RELAY` | Điều khiển relay tải hoặc quạt |
| Relay van gas | `IN` | `GPIO17` | `PIN_VALVE_RELAY` | Khóa hoặc mở van gas |
| LCD I2C 16x2 | `SCL` | `GPIO18` | `PIN_I2C_SCL` | Clock I2C cho LCD |

### 7.3 Wiring Constraints

- Không dùng `GPIO9` đến `GPIO14` cho cảm biến ngoài vì đây là nhóm chân nhạy cảm liên quan flash của board.
- Tránh dùng `GPIO0`, `GPIO3`, `GPIO19`, `GPIO20`, `GPIO45`, `GPIO46` cho wiring cảm biến thường trực để không gây rủi ro boot mode hoặc xung đột USB/JTAG.
- `HC-SR04 ECHO` không được nối trực tiếp 5V vào ESP32-S3; bắt buộc hạ mức xuống 3.3V bằng cầu chia áp hoặc level shifter.
- `MQ2 AO` chỉ được đưa vào `GPIO4` khi tín hiệu analog tối đa 3.3V. Nếu module MQ2 chạy 5V, phải dùng mạch chia áp cho chân `AO`.
- Nếu LCD I2C hoặc relay module dùng nguồn `5V`, phải bảo đảm các đường tín hiệu giao tiếp với ESP32-S3 vẫn là mức `3.3V` an toàn.

## 8. User Stories

1. Là chủ nhà, tôi muốn nhận cảnh báo ngay khi gas vượt ngưỡng nguy hiểm để có thể xử lý sớm trước khi xảy ra sự cố lớn.
2. Là chủ nhà, tôi muốn hệ thống tự động bật quạt hoặc khóa van gas ở các mức cảnh báo xác định để giảm rủi ro ngay cả khi tôi không ở nhà.
3. Là người vận hành, tôi muốn nhìn thấy trạng thái hiện tại trên LCD để kiểm tra nhanh tại chỗ.
4. Là chủ nhà, tôi muốn nhận cảnh báo Telegram hoặc Blynk khi vật cản ở quá gần vùng nguy hiểm để có thể can thiệp.
5. Là người dùng, tôi muốn hệ thống vẫn thu thập và giữ lại dữ liệu khi mất mạng ngắn hạn để tránh mất log sự kiện.
6. Là người bảo trì, tôi muốn cập nhật firmware OTA khi thiết bị đang kết nối để tránh phải tháo lắp phần cứng không cần thiết.

## 9. Feature Table

| ID | Feature | Priority | Description | Dependencies | Current Status |
|---|---|---|---|---|---|
| F1 | Gas Leak Monitoring | Must | Đọc giá trị MQ2, phân loại 3 mức nguy hiểm và kích hoạt cảnh báo hoặc hành động tương ứng | MQ2, ADC, rule thresholds | Implemented |
| F2 | Proximity Hazard Monitoring | Must | Đọc khoảng cách từ HC-SR04, xác định mức cảnh báo gần hoặc nguy hiểm | HC-SR04, timing logic | Implemented |
| F3 | Motion Detection | Should | Ghi nhận xâm nhập hoặc chuyển động bằng PIR và tạo cảnh báo cơ bản | PIR, cooldown logic | Implemented |
| F4 | Local Safety Response | Must | Hiển thị LCD, bật hoặc tắt buzzer, relay, quạt và van gas theo mức độ sự cố | LCD I2C, relay wiring | Implemented |
| F5 | Cloud Telemetry Sync | Must | Đẩy dữ liệu lên MQTT, Firebase RTDB và Blynk khi có mạng | Wi-Fi, cloud credentials | Implemented |
| F6 | Remote Alerting | Must | Gửi thông báo từ xa qua Telegram và Blynk khi có sự cố cảnh báo | Wi-Fi, Telegram bot, Blynk | Implemented |
| F7 | Offline Queue And Replay | Must | Lưu dữ liệu vào LittleFS khi cloud publish thất bại và flush lại khi mạng phục hồi | LittleFS, Wi-Fi reconnect | Implemented |
| F8 | OTA Firmware Update | Should | Cho phép cập nhật firmware qua mạng khi thiết bị online | Wi-Fi, ArduinoOTA | Implemented |
| F9 | Device Configuration And Calibration | Must | Quản lý SSID, cloud credentials, thresholds, device topics và relay mapping | Firmware config process | Partially implemented |
| F10 | Remote Command Execution | Could | Nhận lệnh qua MQTT hoặc Blynk để điều khiển thiết bị từ xa | Command model, auth, actuator rules | Not implemented |
| F11 | Historical Dashboard | Could | Cung cấp giao diện đọc lịch sử, biểu đồ và trạng thái cảnh báo | Backend, frontend | Not implemented |

## 10. Functional Requirements

### F1. Gas Leak Monitoring

- Hệ thống phải đọc cảm biến MQ2 theo chu kỳ định kỳ.
- Hệ thống phải phân loại tối thiểu 3 mức cảnh báo gas.
- Ở mức leak thấp, hệ thống phải hiển thị cảnh báo trên LCD.
- Ở mức leak cao, hệ thống phải kích quạt hút gas và gửi cảnh báo từ xa.
- Ở mức critical hoặc fire, hệ thống phải khóa van gas, kích hoạt buzzer và gửi cảnh báo khẩn.

### F2. Proximity Hazard Monitoring

- Hệ thống phải đo khoảng cách bằng cảm biến siêu âm.
- Hệ thống phải xác định mức cảnh báo dựa trên ngưỡng khoảng cách.
- Ở mức nguy hiểm nhất, hệ thống phải kích hoạt hành động báo động mạnh hơn và có thể ngắt relay.

### F3. Motion Detection

- Hệ thống phải ghi nhận chuyển động bằng cảm biến PIR.
- Hệ thống phải sử dụng cooldown để giảm lặp lại cảnh báo do nhiễu.
- Motion event phải được đưa vào telemetry.

### F4. Local Safety Response

- LCD phải hiển thị tối thiểu giá trị gas, khoảng cách và trạng thái tổng quan.
- Buzzer và relay phải thay đổi theo mức độ cảnh báo.
- Logic phản ứng không được gây trễ quá lớn đến chu kỳ giám sát tổng thể.

### F5. Cloud Telemetry Sync

- Mỗi chu kỳ telemetry phải gồm tối thiểu: gas, distance, distance_level, motion.
- Hệ thống phải có khả năng push dữ liệu đến MQTT.
- Hệ thống phải có khả năng push dữ liệu đến Firebase RTDB.
- Hệ thống phải cập nhật Blynk virtual pins khi đang connected.

### F6. Remote Alerting

- Ở các mức cảnh báo nghiêm trọng, hệ thống phải gửi thông điệp cảnh báo qua Telegram.
- Hệ thống phải gửi thông báo text đến Blynk khi có sự cố.
- Hệ thống phải áp dụng cooldown cho alert để tránh spam.

### F7. Offline Queue And Replay

- Nếu gửi MQTT hoặc Firebase thất bại, payload phải được lưu local.
- Khi Wi-Fi hồi phục, hệ thống phải thử flush lại queue.
- Queue replay không được làm mất bản ghi chưa gửi thành công.

### F8. OTA Firmware Update

- Khi có Wi-Fi, thiết bị phải cho phép nhận OTA update.
- Quá trình OTA phải có logging serial để hỗ trợ bảo trì.

### F9. Device Configuration And Calibration

- Hệ thống phải cho phép cấu hình credentials và thresholds trước khi build.
- Hệ thống phải cho phép đặt device topics và định danh thiết bị.
- Cần có cơ chế tách secrets khỏi source control trước khi release production.

### F10. Remote Command Execution

- Nếu triển khai trong giai đoạn sau, hệ thống phải có command schema rõ ràng.
- Remote commands phải có xác thực và giới hạn tác động với relay hoặc van gas.

## 11. Acceptance Criteria

### AC1. Gas Alert Levels

- Khi giá trị gas < `GAS_LEVEL_1`, LCD hiển thị trạng thái an toàn và buzzer không kêu.
- Khi giá trị gas >= `GAS_LEVEL_1` và < `GAS_LEVEL_2`, hệ thống hiển thị cảnh báo mức thấp.
- Khi giá trị gas >= `GAS_LEVEL_2` và < `GAS_LEVEL_3`, quạt được kích hoạt và thông báo từ xa được gửi theo cooldown.
- Khi giá trị gas >= `GAS_LEVEL_3`, van gas được khóa, buzzer được kích hoạt và thông báo khẩn được gửi.

### AC2. Proximity Alert

- Khi khoảng cách ở trên ngưỡng cảnh báo, hệ thống không kích hoạt critical proximity action.
- Khi khoảng cách vào mức cảnh báo trung bình, buzzer cảnh báo ngắn và có thông báo từ xa theo cooldown.
- Khi khoảng cách vào mức nguy hiểm nhất, relay bị cắt theo logic hiện có và cảnh báo khẩn được gửi.

### AC3. Motion Event

- Khi PIR phát hiện chuyển động và đã qua cooldown, sự kiện được đưa vào telemetry.
- Motion event không được spam liên tục trong thời gian cooldown.

### AC4. Telemetry Delivery

- Khi MQTT và Firebase sẵn sàng, payload được gửi lên cloud.
- Khi một trong hai kênh gửi thất bại, payload được queue local.
- Sau khi mạng phục hồi, queue được flush lại và số bản ghi còn lại giảm xuống nếu gửi thành công.

### AC5. Local Experience

- LCD hiển thị thông tin cốt lõi để người dùng quan sát tại chỗ.
- Hệ thống phản ứng local ngay cả khi cloud service không khả dụng.

### AC6. OTA

- Khi Wi-Fi connected, OTA service có thể được khởi tạo.
- Quá trình OTA có callback logging start, progress, end và error.

## 11. Non-Functional Requirements

### Reliability

- Vòng lặp giám sát phải ưu tiên tính liên tục, tránh block lâu khi mạng chập chờn.
- Hệ thống phải tiếp tục xử lý local alert ngay cả khi cloud service tạm thời không khả dụng.
- Queue offline phải được bảo toàn qua các lần retry ngắn hạn.

### Performance

- Chu kỳ polling mục tiêu là nhanh và ổn định cho use case cảnh báo trong thời gian thực gần.
- Thời gian xử lý thông thường không được làm trễ đến mức bỏ sót sự cố hiện tại.

### Security

- Secrets như Wi-Fi, Firebase, Telegram và Blynk không được commit vào repo production.
- Remote command, nếu mở rộng, phải có xác thực và audit trail.
- OTA production cần có cơ chế bảo vệ phù hợp môi trường vận hành.

### Maintainability

- Cấu hình chân pin, ngưỡng cảnh báo và cloud integration phải dễ thay đổi.
- Tài liệu README, PRD và deployment guide phải được cập nhật đồng bộ khi phạm vi sản phẩm thay đổi.

### Observability

- Serial log phải đủ để chẩn đoán Wi-Fi, MQTT, Firebase, Telegram và OTA.
- Cần có logging rõ ràng cho queue flush, publish thất bại và alert quan trọng.

### Compatibility

- Board mục tiêu chính là ESP32-S3; các dòng ESP32 khác cần được xem là compatibility target thứ cấp.

## 12. Assumptions And Constraints

### Assumptions

- Sản phẩm hiện tại được vận hành như một single-device prototype hoặc MVP.
- Người lắp đặt có thể build firmware và chỉnh sửa `src/config.h` trước khi nạp code.
- Wi-Fi là kết nối mạng chính cho cloud sync và alerting.

### Constraints

- Repo hiện không có test suite tự động cho firmware; validation hiện tại chủ yếu sẽ là hardware hoặc manual test.
- Logic dashboard hoặc web app không tồn tại đầy đủ trong repo.
- `functions/` đang là stub cloud function, chưa thể hiện luồng nghiệp vụ chính.
- `dataconnect/` đang chứa schema và seed data không phù hợp domain Smart Home, cần xem là artifact ngoài phạm vi hoặc cần được làm sạch.
- Mô tả trong README về "ngập nước" và code thực tế của HC-SR04 chưa hoàn toàn nhất quán; cần chốt lại use case chính là flood detection hay obstacle hoặc proximity monitoring.

## 13. Risks

- Hardcoded secrets trong firmware là rủi ro bảo mật nghiêm trọng nếu push lên remote production.
- Ngưỡng gas và distance đang có tính thủ công, có thể cần calibration theo môi trường thực tế.
- Logic alarm trong `src/main.cpp` hiện có nhiều nhánh ghi đè LCD hoặc relay, có thể gây hành vi khó dự đoán nếu nhiều sự kiện xảy ra đồng thời.
- Chưa có bằng chứng về branch review, CI hoặc test automation cho firmware release.
- Có khoảng trống giữa mô tả sản phẩm "Web" trong README và artifact thực tế trong repo.

## 14. Release Recommendation

### Phase 1 - Core MVP

- Gas monitoring và response local.
- Proximity monitoring và local alert.
- Motion detection cơ bản.
- Telegram hoặc Blynk alerting.
- MQTT hoặc Firebase telemetry.
- Offline queue và OTA.

### Phase 2 - Operational Hardening

- Tách secrets khỏi source code.
- Chuẩn hóa config theo environment.
- Thêm hardware validation checklist.
- Refactor logic alert để tránh xung đột state.

### Phase 3 - Product Expansion

- Remote commands an toàn.
- Lịch sử và dashboard quan sát.
- Multi-device management.
- Installer onboarding và calibration workflow.

## 15. Open Questions

1. Sản phẩm có cần một web dashboard thực sự hay chỉ cần cloud dashboards có sẵn như Blynk hoặc Firebase?
2. Use case chính của HC-SR04 là obstacle safety, intrusion proximity hay flood hoặc water proxy?
3. Relay đang được dùng để cắt tải nào trong hệ thống thực tế?
4. Có cần lưu event history dài hạn trên backend, hay chỉ cần cảnh báo thời gian thực?
5. Phiên bản production có cần multi-device hay vẫn là single-device demo?
6. Quy trình cập nhật credentials và xoay secret sẽ được xử lý như thế nào trước khi release?

## 16. Recommended Next Documents

Sau khi PRD này được xác nhận, các tài liệu nên được tạo tiếp:
- `design/architecture.md`: kiến trúc firmware, cloud integrations, data flow.
- `design/flows/*.md`: luồng xử lý cho gas alert, proximity alert, offline sync, OTA.
- `design/screens.md`: nếu dự án sẽ có dashboard hoặc giao diện quan sát.
- `PROJECT_STATUS.md`: theo dõi phase, rủi ro và next actions.
