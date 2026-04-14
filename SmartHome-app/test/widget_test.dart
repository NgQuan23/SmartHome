import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:smarthome/main.dart';

void main() {
  testWidgets('Smart Home App smoke test', (WidgetTester tester) async {
    // Build our app and trigger a frame.
    await tester.pumpWidget(const SmartHomeApp());

    // Kiểm tra xem Dashboard có hiển thị không
    expect(find.text('Smart Home Dashboard'), findsOneWidget);
  });
}
