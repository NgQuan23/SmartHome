import 'package:flutter/foundation.dart';
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
      debugPrint("Error sending command: $e");
    }
  }

  Stream<bool> get awayModeStream {
    final ref = FirebaseDatabase.instance.ref('devices/device1/switches/away_mode');
    return ref.onValue.map((event) {
      final value = event.snapshot.value;
      if (value is bool) return value;
      return false; // Mặc định là tắt nếu chưa có dữ liệu
    });
  }

  Future<void> setAwayMode(bool isEnabled) async {
    try {
      final ref = FirebaseDatabase.instance.ref('devices/device1/switches/away_mode');
      await ref.set(isEnabled);
    } catch (e) {
      debugPrint("Error setting away mode: $e");
    }
  }

  Stream<int> get distanceCriticalStream {
    final ref = FirebaseDatabase.instance.ref('devices/device1/settings/distance_critical');
    return ref.onValue.map((event) {
      final value = event.snapshot.value;
      if (value is int) return value;
      if (value is double) return value.toInt();
      return 5; // Default DIST_LEVEL_2
    });
  }

  Future<void> setDistanceCritical(int limit) async {
    try {
      final ref = FirebaseDatabase.instance.ref('devices/device1/settings/distance_critical');
      await ref.set(limit);
    } catch (e) {
      debugPrint("Error setting distance critical: $e");
    }
  }

  Stream<int> get gasWarningStream {
    final ref = FirebaseDatabase.instance.ref('devices/device1/settings/gas_warning');
    return ref.onValue.map((event) {
      final value = event.snapshot.value;
      if (value is int) return value;
      if (value is double) return value.toInt();
      return 1400; // Default GAS_LEVEL_2
    });
  }

  Future<void> setGasWarning(int limit) async {
    try {
      final ref = FirebaseDatabase.instance.ref('devices/device1/settings/gas_warning');
      await ref.set(limit);
    } catch (e) {
      debugPrint("Error setting gas warning: $e");
    }
  }
}
