class Telemetry {
  final int gas;
  final double distance;
  final int distanceLevel;
  final bool motion;
  final DateTime? timestamp;

  Telemetry({
    required this.gas,
    required this.distance,
    required this.distanceLevel,
    required this.motion,
    this.timestamp,
  });

  factory Telemetry.fromJson(Map<dynamic, dynamic> json) {
    return Telemetry(
      gas: json['gas'] ?? 0,
      distance: (json['distance'] ?? 0).toDouble(),
      distanceLevel: json['distance_level'] ?? 0,
      motion: json['motion'] == 1,
      timestamp: DateTime.now(),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'gas': gas,
      'distance': distance,
      'distance_level': distanceLevel,
      'motion': motion ? 1 : 0,
    };
  }
}
