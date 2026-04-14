import 'package:flutter/foundation.dart';
import '../models/telemetry.dart';
import '../models/alert_model.dart';
import '../services/firebase_service.dart';
import 'alert_viewmodel.dart';

class DashboardViewModel extends ChangeNotifier {
  final FirebaseService _firebaseService = FirebaseService();
  AlertViewModel? _alertViewModel;
  Telemetry? _currentTelemetry;
  bool _isLoading = true;

  Telemetry? get telemetry => _currentTelemetry;
  bool get isLoading => _isLoading;

  static const int gasWarning = 800;
  static const int gasCritical = 3000;
  static const double distWarning = 5.0;
  static const double distCritical = 2.0;

  DashboardViewModel() {
    _listenToTelemetry();
  }

  void setAlertViewModel(AlertViewModel vm) {
    _alertViewModel = vm;
  }

  DateTime _lastUINotify = DateTime.now();
  final Map<String, DateTime> _lastAlerts = {};

  void _listenToTelemetry() {
    _firebaseService.telemetryStream.listen((data) {
      _currentTelemetry = data;
      _isLoading = false;
      
      final now = DateTime.now();
      if (now.difference(_lastUINotify).inMilliseconds > 200) {
        _lastUINotify = now;
        notifyListeners();
      }

      _checkAlerts(data);
    });
  }

  void _checkAlerts(Telemetry data) async {
    if (_alertViewModel == null) return;
    final now = DateTime.now();

    void triggerAlert(String type, String message, String severity) {
      if (_lastAlerts.containsKey(type) && now.difference(_lastAlerts[type]!).inSeconds < 10) return;
      _lastAlerts[type] = now;
      _alertViewModel!.addAlert(AlertModel(
        type: type,
        message: message,
        severity: severity,
        timestamp: now,
      ));
    }

    if (data.gas > gasCritical) {
      triggerAlert("GAS", "CRITICAL GAS LEAK DETECTED!", "CRITICAL");
    } else if (data.gas > gasWarning) {
      triggerAlert("GAS", "Gas level warning", "WARNING");
    }

    if (data.distance < distCritical) {
      triggerAlert("PROXIMITY", "Intruder detected at close range!", "CRITICAL");
    }

    if (data.motion) {
      triggerAlert("MOTION", "Unusual motion detected", "WARNING");
    }
  }

  String getGasStatus() {
    if (_currentTelemetry == null) return "Unknown";
    if (_currentTelemetry!.gas > gasCritical) return "CRITICAL";
    if (_currentTelemetry!.gas > gasWarning) return "WARNING";
    return "SAFE";
  }

  String getDistanceStatus() {
    if (_currentTelemetry == null) return "Unknown";
    if (_currentTelemetry!.distance < distCritical) return "INTRUDER!";
    if (_currentTelemetry!.distance < distWarning) return "CLOSE";
    return "CLEAR";
  }
}
