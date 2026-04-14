import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:syncfusion_flutter_gauges/gauges.dart';
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
      backgroundColor: Colors.grey[50], // Light background for contrast
      appBar: AppBar(
        title: const Text('Smart Dashboard', style: TextStyle(fontWeight: FontWeight.bold)),
        centerTitle: true,
        backgroundColor: Colors.white,
        elevation: 0,
        scrolledUnderElevation: 0,
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
                    style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold, color: Colors.black87),
                  ),
                ),
                const SizedBox(height: 8),
                _buildGasWidget(telemetry, viewModel),
                const SizedBox(height: 16),
                _buildDistanceWidget(telemetry, viewModel),
                const SizedBox(height: 16),
                _buildMotionWidget(telemetry),
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
                      child: Text("Gas Level", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold), overflow: TextOverflow.ellipsis),
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
                  axisLineStyle: const AxisLineStyle(thickness: 15, cornerStyle: CornerStyle.bothCurve),
                  ranges: <GaugeRange>[
                    GaugeRange(startValue: 0, endValue: 800, color: Colors.green, startWidth: 15, endWidth: 15),
                    GaugeRange(startValue: 800, endValue: 3000, color: Colors.orange, startWidth: 15, endWidth: 15),
                    GaugeRange(startValue: 3000, endValue: 4096, color: Colors.red, startWidth: 15, endWidth: 15),
                  ],
                  pointers: <GaugePointer>[
                    NeedlePointer(
                      value: telemetry.gas.toDouble(),
                      needleLength: 0.6,
                      needleColor: Colors.blueGrey[800],
                      knobStyle: const KnobStyle(color: Colors.white, borderColor: Colors.blueGrey, borderWidth: 0.05),
                    )
                  ],
                  annotations: <GaugeAnnotation>[
                    GaugeAnnotation(
                      widget: Text(
                        "${telemetry.gas}",
                        style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold),
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
                      child: Text("Proximity", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold), overflow: TextOverflow.ellipsis),
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
                    child: Text("${telemetry.distance.toStringAsFixed(1)} cm", style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold), overflow: TextOverflow.ellipsis),
                  ),
                  const Icon(Icons.social_distance, color: Colors.blueGrey),
                ],
              ),
              const SizedBox(height: 12),
              ClipRRect(
                borderRadius: BorderRadius.circular(10),
                child: LinearProgressIndicator(
                  value: (telemetry.distance / 100).clamp(0.0, 1.0),
                  minHeight: 12,
                  backgroundColor: Colors.grey[200],
                  valueColor: AlwaysStoppedAnimation<Color>(color),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildMotionWidget(Telemetry telemetry) {
    bool isMotion = telemetry.motion;
    Color color = isMotion ? Colors.red : Colors.green;
    String status = isMotion ? "DETECTED" : "SECURE";

    return Container(
      decoration: _cardDecoration(),
      padding: const EdgeInsets.all(20),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Expanded(
            child: Row(
              children: [
                 Container(
                    padding: const EdgeInsets.all(10),
                    decoration: BoxDecoration(color: color.withOpacity(0.1), shape: BoxShape.circle),
                    child: Icon(isMotion ? Icons.directions_run : Icons.verified_user, color: color, size: 28),
                 ),
                 const SizedBox(width: 12),
                 Expanded(
                   child: Column(
                     crossAxisAlignment: CrossAxisAlignment.start,
                     children: [
                       const Text("Motion", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold), overflow: TextOverflow.ellipsis),
                       Text(isMotion ? "Activity in area!" : "No movement", style: TextStyle(color: Colors.grey[600], fontSize: 13), overflow: TextOverflow.ellipsis),
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
    );
  }

  BoxDecoration _cardDecoration() {
    return BoxDecoration(
      color: Colors.white,
      borderRadius: BorderRadius.circular(20),
      boxShadow: [
        BoxShadow(color: Colors.black.withOpacity(0.04), blurRadius: 10, offset: const Offset(0, 4)),
      ],
    );
  }

  Widget _buildBadge(String text, Color color) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      decoration: BoxDecoration(color: color, borderRadius: BorderRadius.circular(20)),
      child: Text(text, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold, fontSize: 13, letterSpacing: 0.5)),
    );
  }

  Color _getStatusColor(String status) {
    if (status == "SAFE" || status == "CLEAR" || status == "SECURE") return Colors.green;
    if (status == "WARNING" || status == "CLOSE") return Colors.orange;
    return Colors.red;
  }
}
