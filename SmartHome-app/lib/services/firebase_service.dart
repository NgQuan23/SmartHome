import 'package:firebase_database/firebase_database.dart';
import '../models/telemetry.dart';

class FirebaseService {
  // Đảm bảo đường dẫn này khớp chính xác với code trên ESP32
  final DatabaseReference _dbRef = FirebaseDatabase.instance.ref('devices/device1/telemetry');

  Stream<Telemetry> get telemetryStream {
    return _dbRef.onValue.map((event) {
      final Object? value = event.snapshot.value;
      
      // Kiểm tra kiểu dữ liệu an toàn để tránh Crash app
      if (value is Map) {
        return Telemetry.fromJson(Map<dynamic, dynamic>.from(value));
      }
      
      // Trả về dữ liệu mặc định nếu không có dữ liệu để tránh treo màn hình Loading
      return Telemetry(gas: 0, distance: 0, distanceLevel: 0, motion: false);
    });
  }

  Future<void> sendCommand(String action, dynamic value) async {
    try {
      final commandRef = FirebaseDatabase.instance.ref('devices/device1/commands');
      
      // Sử dụng set() thay vì push().set() nếu bạn muốn ghi đè lệnh mới nhất
      // giúp ESP32 dễ dàng đọc dữ liệu hơn
      await commandRef.set({
        "action": action,
        "status": "pending",
        "timestamp": ServerValue.timestamp,
        "value": value
      });
    } catch (e) {
      print("Error sending command: $e");
    }
  }
}
