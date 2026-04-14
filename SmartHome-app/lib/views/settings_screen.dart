import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../services/mqtt_service.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> with AutomaticKeepAliveClientMixin {
  late final MqttService mqttService;

  @override
  bool get wantKeepAlive => true;

  @override
  void initState() {
    super.initState();
    mqttService = MqttService(); // Init once
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Scaffold(
      appBar: AppBar(title: const Text('Settings & Controls')),
      body: ListView(
        padding: const EdgeInsets.all(16.0),
        children: [
          const _SectionHeader(title: "Manual Controls"),
          ListTile(
            leading: const Icon(Icons.volume_up, color: Colors.blue),
            title: const Text("Test Buzzer"),
            subtitle: const Text("Test device alarm system"),
            trailing: ElevatedButton(
              onPressed: () => mqttService.publishCommand('{"buzzer": 1}'),
              child: const Text("TEST"),
            ),
          ),
          ListTile(
            leading: const Icon(Icons.lightbulb, color: Colors.blue),
            title: const Text("Relay Control"),
            subtitle: const Text("Manual relay switch"),
            trailing: Switch(value: false, onChanged: (v) {}),
          ),
          const Divider(),
          const _SectionHeader(title: "Alert Thresholds"),
          const _ThresholdSlider(title: "Gas Warning Level", value: 800, max: 4096),
          const _ThresholdSlider(title: "Distance Critical (cm)", value: 5, max: 100),
          const Divider(),
          const _SectionHeader(title: "Device Info"),
          const ListTile(
            title: Text("Firmware Version"),
            trailing: Text("1.0.2-stable"),
          ),
          const ListTile(
            title: Text("MAC Address"),
            trailing: Text("AA:BB:CC:DD:EE:FF"),
          ),
          const SizedBox(height: 20),
          Center(
            child: OutlinedButton.icon(
              icon: const Icon(Icons.logout, color: Colors.red),
              label: const Text("Sign Out", style: TextStyle(color: Colors.red)),
              onPressed: () {},
            ),
          )
        ],
      ),
    );
  }
}

class _SectionHeader extends StatelessWidget {
  final String title;
  const _SectionHeader({required this.title});
  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8.0),
      child: Text(title, style: const TextStyle(fontWeight: FontWeight.bold, fontSize: 18, color: Colors.blueAccent)),
    );
  }
}

class _ThresholdSlider extends StatelessWidget {
  final String title;
  final double value;
  final double max;
  const _ThresholdSlider({required this.title, required this.value, required this.max});
  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text("$title: ${value.toInt()}"),
        Slider(value: value, max: max, onChanged: (v) {}),
      ],
    );
  }
}
