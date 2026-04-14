import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:syncfusion_flutter_gauges/gauges.dart';
import '../theme.dart';
import '../viewmodels/dashboard_viewmodel.dart';
import '../models/telemetry.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Scaffold(
      appBar: AppBar(
        title: const Text('Smart Dashboard'),
      ),
      body: Consumer<DashboardViewModel>(
        builder: (context, viewModel, child) {
          if (viewModel.isLoading) {
            return const Center(child: CircularProgressIndicator());
          }

          final telemetry = viewModel.telemetry;
          if (telemetry == null) {
            return const Center(child: Text("Waiting for connection..."));
          }

          return RefreshIndicator(
            onRefresh: () async => Future.delayed(const Duration(seconds: 1)),
            child: ListView(
              padding: const EdgeInsets.all(16.0),
              physics: const AlwaysScrollableScrollPhysics(),
              children: [
                const Padding(
                  padding: EdgeInsets.only(left: 4, bottom: 8),
                  child: Text(
                    "Overview",
                    style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis),
                  ),
                ),
                const SizedBox(height: 8),
                _buildGasWidget(telemetry, viewModel),
                const SizedBox(height: 16),
                _buildDistanceWidget(telemetry, viewModel),
                const SizedBox(height: 16),
                _buildMotionWidget(telemetry, viewModel),
              ],
            ),
          );
        },
      ),
    );
  }

  Widget _buildGasWidget(Telemetry telemetry, DashboardViewModel vm) {
    String status = vm.getGasStatus();
    Color color = _getStatusColor(status);
    
    return Container(
      decoration: _cardDecoration(),
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Expanded(
                child: Row(
                  children: [
                    Container(
                      padding: const EdgeInsets.all(10),
                      decoration: BoxDecoration(color: color.withOpacity(0.1), shape: BoxShape.circle),
                      child: Icon(Icons.gas_meter, color: color, size: 28),
                    ),
                    const SizedBox(width: 12),
                    const Expanded(
                      child: Text("Gas Level", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis), overflow: TextOverflow.ellipsis),
                    ),
                  ],
                ),
              ),
              const SizedBox(width: 8),
              _buildBadge(status, color),
            ],
          ),
          const SizedBox(height: 20),
          SizedBox(
            height: 160,
            child: SfRadialGauge(
              axes: <RadialAxis>[
                RadialAxis(
                  minimum: 0, maximum: 4096,
                  showLabels: false, showTicks: false,
                  axisLineStyle: const AxisLineStyle(thickness: 15, dashArray: <double>[5, 3]),
                  ranges: <GaugeRange>[
                    GaugeRange(startValue: 0, endValue: 800, color: AppTheme.tertiary, startWidth: 15, endWidth: 15),
                    GaugeRange(startValue: 800, endValue: 3000, color: Colors.orange, startWidth: 15, endWidth: 15),
                    GaugeRange(startValue: 3000, endValue: 4096, color: AppTheme.error, startWidth: 15, endWidth: 15),
                  ],
                  pointers: <GaugePointer>[
                    NeedlePointer(
                      value: telemetry.gas.toDouble(),
                      needleLength: 0.7,
                      needleStartWidth: 1,
                      needleEndWidth: 4,
                      needleColor: AppTheme.primary,
                      knobStyle: const KnobStyle(color: AppTheme.primary, borderColor: AppTheme.primary, borderWidth: 0.05, knobRadius: 0.04),
                      tailStyle: const TailStyle(length: 0.15, width: 4, color: AppTheme.primary),
                    )
                  ],
                  annotations: <GaugeAnnotation>[
                    GaugeAnnotation(
                      widget: Text(
                        "${telemetry.gas}",
                        style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis),
                      ),
                      angle: 90,
                      positionFactor: 0.9,
                    )
                  ],
                )
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildDistanceWidget(Telemetry telemetry, DashboardViewModel vm) {
    String status = vm.getDistanceStatus();
    Color color = _getStatusColor(status);

    return Container(
      decoration: _cardDecoration(),
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Expanded(
                child: Row(
                  children: [
                    Container(
                      padding: const EdgeInsets.all(10),
                      decoration: BoxDecoration(color: color.withOpacity(0.1), shape: BoxShape.circle),
                      child: Icon(Icons.radar, color: color, size: 28),
                    ),
                    const SizedBox(width: 12),
                    const Expanded(
                      child: Text("Proximity", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis), overflow: TextOverflow.ellipsis),
                    ),
                  ],
                ),
              ),
              const SizedBox(width: 8),
              _buildBadge(status, color),
            ],
          ),
          const SizedBox(height: 20),
          Column(
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Expanded(
                    child: Text("${telemetry.distance.toStringAsFixed(1)} cm", style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis), overflow: TextOverflow.ellipsis),
                  ),
                  const Icon(Icons.social_distance, color: AppTheme.textMediumEmphasis),
                ],
              ),
              const SizedBox(height: 12),
              Container(
                decoration: AppTheme.sciFiButtonDecoration(color),
                padding: const EdgeInsets.all(2),
                child: LinearProgressIndicator(
                  value: (telemetry.distance / 100).clamp(0.0, 1.0),
                  minHeight: 12,
                  backgroundColor: Colors.transparent,
                  valueColor: AlwaysStoppedAnimation<Color>(color),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildMotionWidget(Telemetry telemetry, DashboardViewModel vm) {
    bool isMotion = telemetry.motion;
    bool isAwayMode = vm.awayMode;
    
    Color color;
    String status;
    String description;
    IconData icon;

    if (!isAwayMode) {
      color = AppTheme.textMediumEmphasis;
      status = "DISABLED";
      description = "Motion sensing off";
      icon = Icons.blur_off;
    } else {
      color = isMotion ? AppTheme.error : AppTheme.tertiary;
      status = isMotion ? "DETECTED" : "SECURE";
      description = isMotion ? "Activity in area!" : "No movement";
      icon = isMotion ? Icons.directions_run : Icons.verified_user;
    }

    return Container(
      decoration: _cardDecoration(),
      padding: const EdgeInsets.all(20),
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Expanded(
                child: Row(
                  children: [
                     Container(
                        padding: const EdgeInsets.all(10),
                        decoration: BoxDecoration(color: color.withOpacity(0.1), shape: BoxShape.circle),
                        child: Icon(icon, color: color, size: 28),
                     ),
                     const SizedBox(width: 12),
                     Expanded(
                       child: Column(
                         crossAxisAlignment: CrossAxisAlignment.start,
                         children: [
                           const Text("Motion", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis), overflow: TextOverflow.ellipsis),
                           Text(description, style: TextStyle(color: AppTheme.textMediumEmphasis, fontSize: 13), overflow: TextOverflow.ellipsis),
                         ],
                       ),
                     ),
                  ],
                ),
              ),
              const SizedBox(width: 8),
              _buildBadge(status, color),
            ],
          ),
          const Padding(
            padding: EdgeInsets.symmetric(vertical: 8.0),
            child: Divider(),
          ),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Expanded(
                child: Row(
                  children: [
                    Icon(Icons.shield, color: isAwayMode ? AppTheme.primary : AppTheme.textMediumEmphasis, size: 20),
                    const SizedBox(width: 8),
                    Expanded(
                      child: Text(
                        "Motion Alert (Away Mode)",
                        style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w600, color: AppTheme.textHighEmphasis),
                        overflow: TextOverflow.ellipsis,
                      ),
                    ),
                  ],
                ),
              ),
              Switch(
                value: isAwayMode,
                activeColor: AppTheme.primary,
                inactiveThumbColor: AppTheme.textMediumEmphasis,
                inactiveTrackColor: AppTheme.surfaceHighest,
                onChanged: (value) => vm.toggleAwayMode(value),
              ),
            ],
          ),
        ],
      ),
    );
  }

  BoxDecoration _cardDecoration() {
    return AppTheme.glassCardDecoration;
  }

  Widget _buildBadge(String text, Color color) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      decoration: BoxDecoration(color: color.withOpacity(0.2), borderRadius: BorderRadius.circular(20), border: Border.all(color: color, width: 1)),
      child: Text(text, style: TextStyle(color: color, fontWeight: FontWeight.bold, fontSize: 13, letterSpacing: 0.5)),
    );
  }

  Color _getStatusColor(String status) {
    if (status == "SAFE" || status == "CLEAR" || status == "SECURE") return AppTheme.tertiary;
    if (status == "WARNING" || status == "CLOSE") return Colors.orange;
    return AppTheme.error;
  }
}
