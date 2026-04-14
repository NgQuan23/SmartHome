import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';
import '../theme.dart';
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
            separatorBuilder: (context, index) => const SizedBox(height: 12),
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

    final color = isCritical ? AppTheme.error : Colors.orange;

    return Container(
      decoration: AppTheme.glassCardDecoration,
      margin: const EdgeInsets.only(bottom: 4),
      child: ListTile(
        contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        leading: CircleAvatar(
          backgroundColor: color.withOpacity(0.15),
          child: Icon(
            alert.type == "GAS" ? Icons.warning : Icons.directions_run,
            color: color,
          ),
        ),
        title: Text(alert.message, style: const TextStyle(fontWeight: FontWeight.bold, color: AppTheme.textHighEmphasis)),
        subtitle: Text(timeStr, style: const TextStyle(color: AppTheme.textMediumEmphasis)),
        trailing: Container(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
          decoration: BoxDecoration(
            color: color.withOpacity(0.2), 
            borderRadius: BorderRadius.circular(12),
            border: Border.all(color: color, width: 1),
          ),
          child: Text(alert.severity, style: TextStyle(color: color, fontSize: 10, fontWeight: FontWeight.bold)),
        ),
      ),
    );
  }
}
