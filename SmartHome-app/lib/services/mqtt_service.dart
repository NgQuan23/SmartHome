import 'package:flutter/foundation.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

class MqttService {
  final String _server = 'broker.hivemq.com';
  final String _clientId = 'smarthome_app_${DateTime.now().millisecondsSinceEpoch}';
  final String _publishTopic = 'smarthome/device1/command';
  late MqttServerClient _client;

  MqttService() {
    // Sử dụng MqttServerClient từ gói mqtt_client
    _client = MqttServerClient(_server, _clientId);
    _client.port = 1883;
    _client.keepAlivePeriod = 20;
    _client.onDisconnected = onDisconnected;
    _client.onConnected = onConnected;
    _client.onSubscribed = onSubscribed;
  }

  Future<void> connect() async {
    try {
      await _client.connect();
    } on Exception catch (e) {
      debugPrint('MQTT connection failed: $e');
      _client.disconnect();
    }
  }

  void onConnected() => debugPrint('MQTT Connected');
  void onDisconnected() => debugPrint('MQTT Disconnected');
  void onSubscribed(String topic) => debugPrint('Subscribed to $topic');

  void publishCommand(String command) {
    if (_client.connectionStatus?.state == MqttConnectionState.connected) {
      final builder = MqttClientPayloadBuilder();
      builder.addString(command);
      _client.publishMessage(_publishTopic, MqttQos.atLeastOnce, builder.payload!);
    }
  }
}
