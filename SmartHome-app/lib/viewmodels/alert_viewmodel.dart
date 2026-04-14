import 'package:flutter/foundation.dart';
import '../models/alert_model.dart';
import '../services/database_service.dart';

class AlertViewModel extends ChangeNotifier {
  final DatabaseService _dbService = DatabaseService();
  List<AlertModel> _alerts = [];
  bool _isLoading = true;

  List<AlertModel> get alerts => _alerts;
  bool get isLoading => _isLoading;

  AlertViewModel() {
    loadAlerts();
  }

  Future<void> loadAlerts() async {
    _isLoading = true;
    _alerts = await _dbService.getAlerts();
    _isLoading = false;
    notifyListeners();
  }

  Future<void> addAlert(AlertModel alert) async {
    await _dbService.insertAlert(alert);
    await loadAlerts();
  }

  Future<void> clearHistory() async {
    await _dbService.clearAllAlerts();
    _alerts = [];
    notifyListeners();
  }
}
