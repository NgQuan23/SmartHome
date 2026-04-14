class AlertModel {
  final int? id;
  final String type; // GAS, MOTION, PROXIMITY
  final String message;
  final String severity; // WARNING, CRITICAL
  final DateTime timestamp;

  AlertModel({
    this.id,
    required this.type,
    required this.message,
    required this.severity,
    required this.timestamp,
  });

  Map<String, dynamic> toMap() {
    return {
      'id': id,
      'type': type,
      'message': message,
      'severity': severity,
      'timestamp': timestamp.toIso8601String(),
    };
  }

  factory AlertModel.fromMap(Map<String, dynamic> map) {
    return AlertModel(
      id: map['id'],
      type: map['type'],
      message: map['message'],
      severity: map['severity'],
      timestamp: DateTime.parse(map['timestamp']),
    );
  }
}
