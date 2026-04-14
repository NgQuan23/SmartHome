import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';
import '../viewmodels/alert_viewmodel.dart';
import '../models/alert_model.dart';

class AlertsScreen extends StatefulWidget {
  const AlertsScreen({super.key});

  @override
  State<AlertsScreen> createState() => _AlertsScreenState();
}

class _AlertsScreenState extends State<AlertsScreen> with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Scaffold(
      appBar: AppBar(
        title: const Text('Alert History'),
        actions: [
          IconButton(
            icon: const Icon(Icons.delete_sweep),
            onPressed: () => context.read<AlertViewModel>().clearHistory(),
          ),
        ],
      ),
      body: Consumer<AlertViewModel>(
        builder: (context, viewModel, child) {
          if (viewModel.isLoading) {
            return const Center(child: CircularProgressIndicator());
          }

          if (viewModel.alerts.isEmpty) {
            return const Center(child: Text("No alerts found."));
          }

          return ListView.separated(
            padding: const EdgeInsets.all(16.0),
            itemCount: viewModel.alerts.length,
            separatorBuilder: (context, index) => const Divider(),
            itemBuilder: (context, index) {
              final alert = viewModel.alerts[index];
              return _AlertItem(alert: alert);
            },
          );
        },
      ),
    );
  }
}

class _AlertItem extends StatelessWidget {
  final AlertModel alert;
  const _AlertItem({required this.alert});

  @override
  Widget build(BuildContext context) {
    final isCritical = alert.severity == "CRITICAL";
    final timeStr = DateFormat('yyyy-MM-dd HH:mm:ss').format(alert.timestamp);

    return ListTile(
      leading: CircleAvatar(
        backgroundColor: isCritical ? Colors.red.withAlpha(50) : Colors.orange.withAlpha(50),
        child: Icon(
          alert.type == "GAS" ? Icons.warning : Icons.directions_run,
          color: isCritical ? Colors.red : Colors.orange,
        ),
      ),
      title: Text(alert.message, style: const TextStyle(fontWeight: FontWeight.bold)),
      subtitle: Text(timeStr),
      trailing: Chip(
        label: Text(alert.severity, style: const TextStyle(color: Colors.white, fontSize: 10)),
        backgroundColor: isCritical ? Colors.red : Colors.orange,
      ),
    );
  }
}
