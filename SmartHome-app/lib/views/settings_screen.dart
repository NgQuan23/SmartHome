import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../theme.dart';
import '../services/mqtt_service.dart';
import '../services/firebase_service.dart';
import '../viewmodels/dashboard_viewmodel.dart';

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
          Container(
            decoration: AppTheme.glassCardDecoration,
            child: Column(
              children: [
                ListTile(
                  leading: const Icon(Icons.volume_up, color: AppTheme.primary),
                  title: const Text("Test Buzzer", style: TextStyle(color: AppTheme.textHighEmphasis)),
                  subtitle: const Text("Test device alarm system", style: TextStyle(color: AppTheme.textMediumEmphasis)),
                  trailing: Container(
                    decoration: AppTheme.sciFiButtonDecoration(AppTheme.primary),
                    child: TextButton(
                      style: TextButton.styleFrom(foregroundColor: AppTheme.primary),
                      onPressed: () => mqttService.publishCommand('{"buzzer": 1}'),
                      child: const Text("TEST", style: TextStyle(fontWeight: FontWeight.bold, letterSpacing: 1.2)),
                    ),
                  ),
                ),
                ListTile(
                  leading: const Icon(Icons.lightbulb, color: AppTheme.primary),
                  title: const Text("Relay Control", style: TextStyle(color: AppTheme.textHighEmphasis)),
                  subtitle: const Text("Manual relay switch", style: TextStyle(color: AppTheme.textMediumEmphasis)),
                  trailing: Switch(
                    value: false, 
                    activeThumbColor: AppTheme.primary,
                    inactiveThumbColor: AppTheme.textMediumEmphasis,
                    inactiveTrackColor: AppTheme.surfaceHighest,
                    onChanged: (v) {},
                  ),
                ),
              ],
            ),
          ),
          const SizedBox(height: 16),
          const _SectionHeader(title: "Alert Thresholds"),
          Container(
            decoration: AppTheme.glassCardDecoration,
            padding: const EdgeInsets.all(16),
            child: Consumer<DashboardViewModel>(
              builder: (context, vm, child) {
                return Column(
                  children: [
                    _ThresholdSlider(
                      title: "Gas Warning Level", 
                      initialValue: vm.gasWarning.toDouble(), 
                      max: 4096,
                      onChangeEnd: (v) => FirebaseService().setGasWarning(v.toInt()),
                    ),
                    const SizedBox(height: 16),
                    _ThresholdSlider(
                      title: "Distance Critical (cm)", 
                      initialValue: vm.distCritical.toDouble(), 
                      min: 5.0,
                      max: 100,
                      onChangeEnd: (v) => FirebaseService().setDistanceCritical(v.toInt()),
                    ),
                  ],
                );
              },
            ),
          ),
          const SizedBox(height: 16),
          const _SectionHeader(title: "Device Info"),
          Container(
             decoration: AppTheme.glassCardDecoration,
             child: Column(
               children: const [
                 ListTile(
                   title: Text("Firmware Version", style: TextStyle(color: AppTheme.textHighEmphasis)),
                   trailing: Text("1.0.2-stable", style: TextStyle(color: AppTheme.textMediumEmphasis)),
                 ),
                 ListTile(
                   title: Text("MAC Address", style: TextStyle(color: AppTheme.textHighEmphasis)),
                   trailing: Text("AA:BB:CC:DD:EE:FF", style: TextStyle(color: AppTheme.textMediumEmphasis)),
                 ),
               ],
             )
          ),
          const SizedBox(height: 32),
          Center(
            child: Container(
              decoration: AppTheme.sciFiButtonDecoration(AppTheme.error),
              child: TextButton.icon(
                icon: const Icon(Icons.logout, color: AppTheme.error),
                label: const Text("Sign Out", style: TextStyle(color: AppTheme.error, fontWeight: FontWeight.bold, letterSpacing: 1.2)),
                onPressed: () {},
              ),
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
      child: Text(title, style: const TextStyle(fontWeight: FontWeight.bold, fontSize: 18, color: AppTheme.primary)),
    );
  }
}

class _ThresholdSlider extends StatefulWidget {
  final String title;
  final double initialValue;
  final double min;
  final double max;
  final ValueChanged<double> onChangeEnd;

  const _ThresholdSlider({
    required this.title,
    required this.initialValue,
    this.min = 0.0,
    required this.max,
    required this.onChangeEnd,
  });

  @override
  State<_ThresholdSlider> createState() => _ThresholdSliderState();
}

class _ThresholdSliderState extends State<_ThresholdSlider> {
  late double _currentValue;

  @override
  void initState() {
    super.initState();
    _currentValue = widget.initialValue;
  }

  @override
  void didUpdateWidget(_ThresholdSlider oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.initialValue != widget.initialValue) {
      _currentValue = widget.initialValue;
    }
  }

  @override
  Widget build(BuildContext context) {
    double displayValue = _currentValue.clamp(widget.min, widget.max);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text("${widget.title}: ${displayValue.toInt()}", style: const TextStyle(color: AppTheme.textHighEmphasis)),
        Slider(
          value: displayValue, 
          min: widget.min,
          max: widget.max, 
          activeColor: AppTheme.primary,
          inactiveColor: AppTheme.surfaceHighest,
          onChanged: (v) {
            setState(() { _currentValue = v; });
          },
          onChangeEnd: widget.onChangeEnd,
        ),
      ],
    );
  }
}
