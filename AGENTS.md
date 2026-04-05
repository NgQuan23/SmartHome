# AGENTS.md

Tài liệu này định nghĩa cách các AI agents phải làm việc trong repo `SmartHome`. Mục tiêu là giữ mọi thay đổi bám sát chức năng hiện có của hệ thống giám sát nhà thông minh dùng ESP32-S3, đồng thời buộc mỗi task sau khi hoàn tất phải được kiểm tra lại một lần và báo cáo rõ ràng cho người dùng.

## 1. Project Snapshot

### Mục tiêu dự án
Repo này triển khai firmware cho hệ thống Smart Home Security & Monitoring System với các khả năng chính:
- Giám sát rò rỉ gas bằng MQ2 với 3 mức cảnh báo.
- Theo dõi khoảng cách/chướng ngại vật bằng HC-SR04 và phản ứng bằng relay, còi, cảnh báo khẩn.
- Phát hiện chuyển động bằng PIR.
- Hiển thị trạng thái trên LCD I2C.
- Đồng bộ telemetry lên Blynk Cloud, Firebase Realtime Database, MQTT.
- Gửi thông báo Telegram khi có sự kiện nguy hiểm.
- Hỗ trợ OTA và lưu tạm dữ liệu để đồng bộ lại khi mạng phục hồi.

### Stack và runtime hiện tại
- Firmware: Arduino framework trên ESP32-S3.
- Build system: PlatformIO (`platformio.ini`).
- Thiết bị/chân chính: MQ2, PIR, HC-SR04, relay, buzzer, LCD I2C.
- Dịch vụ tích hợp: Blynk, Firebase RTDB, MQTT HiveMQ, Telegram Bot, OTA.
- Bộ nhớ tạm/offline sync: lớp lưu trữ trong `src/storage.*`.

### Khu vực code chính
- `src/main.cpp`: vòng lặp cảm biến, mức cảnh báo, relay, buzzer, LCD.
- `src/config.h`: cấu hình WiFi, cloud, pin, threshold, feature flags. Đây là file nhạy cảm.
- `src/wifi_mqtt.*`: WiFi và MQTT.
- `src/firebase_client.*`: đẩy telemetry lên Firebase.
- `src/blynk_client.*`: Blynk Cloud.
- `src/telegram.*`: Telegram alerts.
- `src/storage.*`: queue offline và flush lại khi có mạng.
- `src/ota.*`: OTA update.
- `platformio.ini`: board, framework, thư viện, build flags.
- `boards/`: board definition.
- `functions/`, `dataconnect/`: chỉ chỉnh khi task thực sự liên quan backend hoặc connector.
- `test/`: nơi đặt kiểm thử; nếu chưa có test hữu ích thì phải ghi rõ rủi ro manual-only.

## 2. Process Overview

### Thứ tự pha bắt buộc
Mọi task không tầm thường phải đi theo thứ tự:

`Requirement -> Design -> Implementation -> Test`

Main agent có thể thêm bước preflight trước pha Requirement, nhưng không được bỏ qua thứ tự 4 pha trên nếu task có thay đổi hành vi, thay đổi tích hợp, thay đổi cấu hình, hoặc ảnh hưởng release.

### Preflight bắt buộc trước mọi task không tầm thường
1. Đọc `README.md`.
2. Đọc các file liên quan trực tiếp đến task trong `src/` và `platformio.ini`.
3. Kiểm tra Git tối thiểu:
   - `git status --short --branch`
   - `git log --oneline -5`
   - `git diff --stat` nếu có thay đổi
4. Xác định task tác động vào capability nào:
   - Gas safety
   - Distance / obstacle safety
   - Motion detection
   - LCD / local UX
   - Cloud telemetry
   - Offline sync
   - Alerts / notifications
   - OTA / maintenance
5. Xác định có cần tạo hoặc cập nhật artifact tài liệu hay không.

### Quy tắc hoàn tất task
Mỗi task chỉ được xem là hoàn tất khi đã đi qua đủ 2 bước cuối:
1. `Self-check`: agent tự kiểm tra lại diff, logic, file liên quan và validation phù hợp.
2. `User report`: main agent báo cáo lại cho người dùng phạm vi đã làm, cách kiểm tra, kết quả và rủi ro còn lại.

Không được kết thúc task chỉ bằng việc "đã sửa xong".

## 3. Detailed Workflow By Phase

## Giai đoạn 1: Requirement

### Mục tiêu
Hiểu chính xác yêu cầu và ánh xạ nó vào chức năng hiện có của SmartHome trước khi đề xuất thay đổi.

### Workflow
1. `main` nhận yêu cầu và xác định đây là:
   - bug fix
   - thay đổi hành vi
   - thêm feature
   - thay đổi cấu hình / cloud
   - review Git / release
2. `agent-analyst` đọc `README.md`, các file module liên quan và ghi nhận:
   - tính năng hiện tại bị ảnh hưởng
   - module bị ảnh hưởng
   - tác động đến sensor, relay, network, alert, OTA, docs
3. Với task đủ lớn, `agent-analyst` tạo hoặc cập nhật `PRD.md` gồm:
   - mục tiêu
   - phạm vi
   - acceptance criteria
   - ràng buộc phần cứng / cloud
   - giả định
   - rủi ro
4. Với task nhỏ, thay vì tạo `PRD.md`, `main` phải cập nhật brief ngắn trong `PROJECT_STATUS.md`.
5. Nếu task đụng tới `src/config.h`, credentials, thresholds, pin mapping, relay actions hoặc alert behavior, phải ghi rõ đó là thay đổi nhạy cảm.

### Checkpoint
- User phải xác nhận lại phạm vi khi task thay đổi:
  - ngưỡng gas / distance
  - chân cắm / relay logic
  - payload telemetry
  - hành vi Telegram / Blynk / Firebase / MQTT
  - cấu hình OTA
- Sau khi rõ scope, `main` cập nhật trạng thái: `Requirement -> Design`.

## Giai đoạn 2: Design

### Mục tiêu
Thiết kế rõ luồng tác động, dữ liệu và điểm rủi ro trước khi code.

### Workflow
1. `agent-analyst` tạo hoặc cập nhật `design/system-architecture.md` khi task ảnh hưởng nhiều hơn một module.
2. Tạo hoặc cập nhật flow docs phù hợp trong `design/flows/`:
   - `design/flows/01-gas-safety.md`
   - `design/flows/02-distance-protection.md`
   - `design/flows/03-motion-alert.md`
   - `design/flows/04-cloud-sync.md`
   - `design/flows/05-ota-maintenance.md`
3. Nếu task ảnh hưởng config/pin/threshold, cập nhật `design/pins-and-thresholds.md`.
4. Nếu task ảnh hưởng cloud contract hoặc queue offline, cập nhật `design/integrations.md` với:
   - topic MQTT
   - shape payload JSON
   - mapping Blynk Virtual Pin
   - cấu trúc dữ liệu Firebase
   - điều kiện queue / flush
5. `main` tách implementation thành các task nhỏ, ưu tiên:
   - thay đổi ít rủi ro nhất trước
   - tách sensor/actuator logic khỏi cloud integration khi có thể
   - giữ commit sau này ở mức atomic

### Checkpoint
- User xác nhận design khi task:
  - chạm nhiều module
  - thay đổi hành vi an toàn
  - thay đổi contract với cloud
  - thay đổi setup phần cứng
- Sau khi được xác nhận, `main` cập nhật trạng thái: `Design -> Implementation`.

## Giai đoạn 3: Implementation

### Mục tiêu
Triển khai thay đổi nhỏ nhất hợp lý, không làm trôi phạm vi, không phá vỡ chức năng hiện có trong README.

### Workflow
1. `main` giao việc theo ownership rõ ràng:
   - `agent-firmware`: `src/main.cpp`, sensor polling, relay, buzzer, LCD, deep sleep, pin logic.
   - `agent-integration`: `src/wifi_mqtt.*`, `src/firebase_client.*`, `src/blynk_client.*`, `src/telegram.*`, `src/storage.*`, `src/ota.*`.
   - `agent-qa`: review diff, compile/test, regression checklist, Git readiness.
2. Nếu task đụng cả firmware và integration, ưu tiên làm theo 2 bước:
   - bước 1: logic cục bộ / payload / contract
   - bước 2: nối integration và đồng bộ docs
3. Mọi chỉnh sửa ở `src/config.h` phải tuân thủ:
   - không tự ý commit secrets mới
   - không đổi credential thật nếu user chưa yêu cầu
   - nếu cần minh họa trong docs, dùng placeholder
4. Nếu thay đổi hành vi setup, build, upload, OTA hoặc cấu hình cloud, phải cập nhật `README.md`.
5. Nếu thay đổi logic có thể ảnh hưởng an toàn, phải ghi chú rõ side effect dự kiến trong `PROJECT_STATUS.md`.
6. Sau mỗi task implementation, agent bắt buộc chạy `Task Completion Contract` ở mục 7 trước khi chuyển task tiếp theo.

### Checkpoint
- Dừng và xin xác nhận user nếu gặp:
  - scope nở ra ngoài feature đang làm
  - xung đột giữa README và code
  - yêu cầu đổi kiến trúc
  - yêu cầu xoay vòng secrets thật
  - cần sửa file ngoài phạm vi task hiện tại

## Giai đoạn 4: Test

### Mục tiêu
Xác minh thay đổi không phá vỡ tính năng hiện có, hoặc nêu rõ phần nào chưa thể xác minh.

### Workflow
1. `agent-qa` hoặc agent vừa implement phải tự review lại:
   - file đã sửa
   - diff thực tế
   - phạm vi yêu cầu
   - tài liệu liên quan
2. Chạy validation tối thiểu theo mức tác động:
   - Firmware/code change: `pio run -e esp32-s3-devkitc-1`
   - Build/config change: compile lại và kiểm tra `platformio.ini` / `src/config.h`
   - Docs-only change: rà link, đường dẫn, lệnh setup
   - Git/release task: kiểm tra status, branch, diff, remote, ahead/behind
3. Nếu không có phần cứng hoặc môi trường cloud để test thật, phải tạo checklist manual trong báo cáo:
   - MQ2 alert levels
   - Distance thresholds
   - PIR detection
   - Blynk virtual pins
   - Firebase push
   - MQTT publish
   - Telegram alert
   - storage append/flush
   - OTA behavior
4. Ghi kết quả test hoặc giới hạn test vào `PROJECT_STATUS.md`.
5. `main` gửi báo cáo cho user theo format ở mục 7.

### Checkpoint
- Không được kết luận "ready" nếu chưa có một trong hai:
  - validation đã chạy và có kết quả
  - hoặc lý do cụ thể vì sao chưa chạy được, kèm manual checklist / rủi ro còn lại

## 4. Role And Responsibility Matrix

| Role | Trách nhiệm chính | Output chính |
|---|---|---|
| `main` | Điều phối quy trình, chia task, tổng hợp kết quả, báo cáo user, giữ trạng thái dự án, giữ kỷ luật Git | `PROJECT_STATUS.md`, báo cáo cuối task, quyết định checkpoint |
| `agent-analyst` | Project Analyst + Planner + Prompt Engineer cho task cụ thể, phân tích yêu cầu và rủi ro | `PRD.md`, `design/*`, prompt handoff chuẩn |
| `agent-firmware` | Sensor logic, relay, buzzer, LCD, deep sleep, on-device safety behavior | code trong `src/main.cpp` và tài liệu kỹ thuật liên quan |
| `agent-integration` | WiFi, MQTT, Firebase, Blynk, Telegram, storage, OTA, payload/data contracts | code trong các module integration và `design/integrations.md` |
| `agent-qa` | Self-check, compile/test, regression checklist, commit/push readiness | báo cáo validation, checklist test, nhận xét Git |

### Quy tắc ownership
- Một task phải có một owner chính.
- Khi handoff giữa agents, phải log vào `AGENT_COMMUNICATION.log`.
- Agent sau không được âm thầm thay đổi phạm vi task do agent trước chốt nếu chưa báo `main`.

## 5. User Approval Checkpoints

Phải dừng và xin xác nhận user khi gặp một trong các trường hợp sau:
- Thay đổi feature behavior được mô tả trong `README.md`.
- Thay đổi ngưỡng gas, distance, relay action hoặc còi cảnh báo.
- Thay đổi pin mapping, board config hoặc chiến lược power/deep sleep.
- Thay đổi payload telemetry, mapping Blynk Virtual Pin, schema Firebase hoặc MQTT topic.
- Thay đổi cách hoạt động của Telegram alert hoặc OTA.
- Tạo mới hoặc rewrite `AGENTS.md`, `PRD.md`, tài liệu design quan trọng.
- Cần sửa nhiều module ngoài dự kiến ban đầu.
- Chuẩn bị commit nhiều mục tiêu trong một lần.
- Muốn `git push`, rebase, force push hoặc thao tác Git có rủi ro.

## 6. Project Tracking File

Main agent phải duy trì `PROJECT_STATUS.md` khi task đủ lớn hoặc kéo dài qua nhiều bước.

Format khuyến nghị:

```markdown
# Project Status

## Goal
[Mục tiêu hiện tại]

## Current Phase
[Requirement|Design|Implementation|Test|Done]

## Current Focus
- [Task đang làm]

## Affected Capabilities
- [Gas safety / Distance / Motion / Cloud sync / OTA ...]

## Findings
- [Phát hiện quan trọng]

## Validation
- [Lệnh đã chạy hoặc chưa chạy được vì sao]

## Risks
- [Rủi ro còn lại]

## Next Actions
- [Bước tiếp theo]

## Last Updated
[YYYY-MM-DD HH:MM:SS]
```

## 7. Task Completion Contract

Sau mỗi task, owner của task bắt buộc phải làm đủ các bước sau:

1. Đọc lại file vừa sửa và file lân cận bị ảnh hưởng.
2. Xem lại diff để chắc không có thay đổi ngoài phạm vi.
3. Chạy validation tối thiểu phù hợp với task.
4. Ghi kết quả ngắn gọn vào `PROJECT_STATUS.md` nếu task không tầm thường.
5. Báo cáo lại cho user qua `main`.

### Format báo cáo tối thiểu cho user

```text
Task:
- [đã làm gì]

Files:
- [file 1]
- [file 2]

Checks:
- [đã tự review gì]
- [đã chạy lệnh gì]

Result:
- [pass/fail/partial]

Remaining Risks:
- [nếu có]

Manual Verify:
- [những gì user nên test thêm trên board/cloud]
```

### Quy tắc bắt buộc
- Nếu chưa chạy được test/build, phải nói rõ vì sao.
- Không được báo "xong" nếu chưa có mục `Checks`.
- Nếu task chỉ là tài liệu, vẫn phải kiểm tra lại tính nhất quán với README/code.

## 8. Agent Communication Logging

### File log
- `AGENT_COMMUNICATION.log`

### Format bắt buộc
```text
[YYYY-MM-DD HH:MM:SS] SENDER -> RECEIVER | REQUEST_BRIEF
```

### Quy tắc `REQUEST_BRIEF`
- Một dòng, ngắn gọn, rõ nghĩa.
- Khuyến nghị tối đa 100 ký tự.
- Ưu tiên mở đầu bằng động từ: `Phân tích`, `Thiết kế`, `Implement`, `Kiểm tra`, `Handoff`, `Rà soát`.

### Trường hợp bắt buộc log
- `main` giao task cho agent khác.
- Agent báo kết quả về `main`.
- Handoff giữa `agent-firmware`, `agent-integration`, `agent-qa`.
- Yêu cầu review Git, commit, push readiness.
- Yêu cầu viết prompt hoặc design artifact cho task tiếp theo.

### Trường hợp không cần log
- Agent đọc file nội bộ.
- Agent tự suy luận.
- `main` trả lời user trực tiếp mà không handoff.
- Chỉnh sửa nhỏ trong cùng một task mà không đổi owner.

## 9. Git And Release Rules

### Trước khi commit
Phải kiểm tra:
- branch hiện tại
- working tree có sạch trong phạm vi task hay không
- diff có chứa file ngoài phạm vi không
- có lộ secrets hoặc cấu hình nhạy cảm trong `src/config.h` không
- validation tối thiểu đã chạy chưa

### Chuẩn commit
- Một commit chỉ nên giải quyết một mục tiêu rõ ràng.
- Commit message phải phản ánh đúng thay đổi.
- Nếu task đụng cả logic sensor và cloud integration nhưng có thể tách được, ưu tiên tách commit.
- Không commit file sinh tự động hoặc artifact không cần version control.

### Trước khi push
Phải kiểm tra:
- `git remote -v`
- branch đích và upstream
- local đang ahead hay behind
- còn uncommitted changes hay staged-but-uncommitted changes không
- có validation summary hay chưa

### Cấm tự động push
- Không `git push` nếu user chưa yêu cầu trực tiếp.
- Dù user yêu cầu, `main` vẫn phải báo trước:
  - branch sẽ push
  - remote sẽ push
  - commit nào sẽ đi lên
  - rủi ro còn lại

## 10. Prompt Handoff Standard

Mọi prompt giao việc giữa agents phải có đủ 6 phần sau khi task đủ phức tạp:

1. `Role`
2. `Objective`
3. `Project Context`
4. `Constraints`
5. `Steps`
6. `Expected Output`

Mẫu ngắn:

```text
Role:
Bạn là [agent phù hợp].

Objective:
[mục tiêu cụ thể].

Project Context:
- Repo: SmartHome
- Module liên quan: [file/module]
- Tính năng liên quan: [gas/distance/motion/cloud/OTA]

Constraints:
- Không sửa ngoài phạm vi task
- Không bỏ qua self-check
- Báo rõ rủi ro còn lại

Steps:
1. Phân tích hiện trạng.
2. Đề xuất hoặc triển khai thay đổi nhỏ nhất hợp lý.
3. Tự kiểm tra lại diff và validation.
4. Trả kết quả cho main.

Expected Output:
- Tóm tắt thay đổi
- File bị ảnh hưởng
- Kiểm tra đã chạy
- Rủi ro còn lại
```

## 11. Working Principles

1. Luôn đọc `README.md` và file module liên quan trước khi sửa.
2. Không đoán trạng thái Git nếu chưa kiểm tra thật.
3. Không chỉnh `src/config.h` một cách vô trách nhiệm; xem đây là file nhạy cảm.
4. Ưu tiên thay đổi nhỏ, dễ review, dễ revert.
5. Bất kỳ thay đổi nào làm khác đi hành vi đã nêu trong `README.md` phải được nêu rõ trong báo cáo.
6. Nếu code và tài liệu mâu thuẫn, phải dừng để làm rõ thay vì tự chọn một phía.
7. Nếu không có phần cứng để test thật, phải nói rõ giới hạn kiểm chứng.
8. Nếu repo chưa có test tự động, phải ghi rõ đây là rủi ro release.
9. Không kết thúc task khi chưa tự kiểm tra và báo cáo user.

## 12. Exception Handling

- Không có Git repo: chuyển sang phân tích tĩnh, bỏ qua commit/push assessment nhưng phải nói rõ.
- Không có `PRD.md` hoặc tài liệu design: tạo artifact tối thiểu nếu task đủ lớn.
- Không có phần cứng / cloud credentials hợp lệ: compile và review logic trước, kèm manual checklist.
- Phát hiện secrets trong diff: dừng lại, báo `main`, không tiếp tục commit/push cho tới khi user xác nhận hướng xử lý.
- Phát hiện thay đổi ngoài phạm vi task hiện tại trong working tree: không đụng vào nếu không được user yêu cầu.

## 13. End-to-End Example Workflow

Ví dụ: user yêu cầu "điều chỉnh ngưỡng gas mức 2 và thay đổi nội dung Telegram alert".

1. `main` đọc `README.md`, `src/main.cpp`, `src/config.h`, `src/telegram.cpp`, `git status --short --branch`, `git log --oneline -5`.
2. `main` giao `agent-analyst` phân tích phạm vi:
   - gas alert logic
   - Telegram notification
   - khả năng ảnh hưởng tới Blynk/Firebase/MQTT report
3. `agent-analyst` cập nhật `PRD.md` hoặc `PROJECT_STATUS.md`, nêu acceptance criteria và rủi ro.
4. User xác nhận lại scope.
5. `agent-analyst` cập nhật `design/flows/01-gas-safety.md` và `design/integrations.md` nếu contract alert thay đổi.
6. `main` giao:
   - `agent-firmware` sửa threshold và hành vi tại `src/main.cpp` hoặc `src/config.h`
   - `agent-integration` sửa format Telegram alert nếu cần
7. Sau mỗi phần việc, agent owner phải:
   - đọc lại file vừa sửa
   - xem diff
   - chạy `pio run -e esp32-s3-devkitc-1` nếu code thay đổi
   - ghi kết quả vào `PROJECT_STATUS.md`
   - báo về `main`
8. `agent-qa` rà soát regression:
   - gas level 1/2/3
   - còi, relay, van gas
   - Telegram alert cooldown
   - ảnh hưởng đến Blynk/Firebase/MQTT
9. `main` gửi báo cáo cho user:
   - task đã làm
   - file đã sửa
   - kiểm tra đã chạy
   - kết quả
   - manual verify còn thiếu trên board thật
10. Chỉ khi user yêu cầu tiếp mới chuyển sang commit hoặc push assessment.
