import 'dart:ui';
import 'package:flutter/material.dart';

class AppTheme {
  // Core Colors
  static const Color background = Color(0xFF0B1326); // Deep Indigo-Charcoal
  static const Color surfaceLow = Color(0xFF131B2E);
  static const Color surfaceHigh = Color(0xFF222A3D);
  static const Color surfaceHighest = Color(0xFF2D3449);
  
  static const Color primary = Color(0xFF8AEBFF); // Cyan action
  static const Color primaryContainer = Color(0xFF22D3EE);
  
  static const Color secondary = Color(0xFFDDB7FF); // Vivid Purple
  static const Color secondaryContainer = Color(0xFF6F00BE);
  
  static const Color tertiary = Color(0xFF68F5B8); // Soft Green (Safe)
  static const Color error = Color(0xFFFFB4AB); // Danger
  
  static const Color textHighEmphasis = Color(0xFFDAE2FD);
  static const Color textMediumEmphasis = Color(0xFFBBC9CD);

  static const Color outlineVariant = Color(0xFF3C494C);

  static ThemeData get darkTheme {
    return ThemeData(
      brightness: Brightness.dark,
      scaffoldBackgroundColor: background,
      colorScheme: const ColorScheme.dark(
        primary: primary,
        secondary: secondary,
        tertiary: tertiary,
        error: error,
        surface: surfaceLow,
        background: background,
      ),
      appBarTheme: const AppBarTheme(
        backgroundColor: Colors.transparent,
        elevation: 0,
        centerTitle: true,
        iconTheme: IconThemeData(color: textHighEmphasis),
        titleTextStyle: TextStyle(
          color: textHighEmphasis,
          fontSize: 20,
          fontWeight: FontWeight.bold,
          letterSpacing: 0.5,
        ),
      ),
      bottomNavigationBarTheme: const BottomNavigationBarThemeData(
        backgroundColor: surfaceLow,
        selectedItemColor: primary,
        unselectedItemColor: textMediumEmphasis,
      ),
      sliderTheme: const SliderThemeData(
        thumbShape: RectangularSliderThumbShape(), // Requires creating a subtle square or just using native with glow
        trackHeight: 2.0,
      ),
      textTheme: const TextTheme(
        bodyLarge: TextStyle(color: textHighEmphasis),
        bodyMedium: TextStyle(color: textMediumEmphasis),
        headlineSmall: TextStyle(color: textHighEmphasis, fontWeight: FontWeight.bold),
      ),
    );
  }

  // Glassmorphism helper
  static BoxDecoration get glassCardDecoration {
    return BoxDecoration(
      color: surfaceHighest.withOpacity(0.4),
      borderRadius: BorderRadius.circular(20),
      border: Border.all(
        color: primary.withOpacity(0.2),
        width: 1,
      ),
      boxShadow: [
        BoxShadow(
          color: primary.withOpacity(0.08),
          blurRadius: 50,
          spreadRadius: -10,
        ),
      ],
    );
  }

  // Futuristic Component Styles
  static BoxDecoration sciFiButtonDecoration(Color color) {
    return BoxDecoration(
      color: surfaceHigh,
      border: Border.all(color: color, width: 1.5),
      boxShadow: [
        BoxShadow(
          color: color.withOpacity(0.5),
          blurRadius: 15,
          spreadRadius: -2,
        ),
      ],
    );
  }
}

// Custom Slider Shape for Sci-Fi look
class RectangularSliderThumbShape extends SliderComponentShape {
  const RectangularSliderThumbShape();

  @override
  Size getPreferredSize(bool isEnabled, bool isDiscrete) {
    return const Size(8, 24);
  }

  @override
  void paint(
    PaintingContext context, 
    Offset center, 
    {required Animation<double> activationAnimation, 
    required Animation<double> enableAnimation, 
    required bool isDiscrete, 
    required TextPainter labelPainter, 
    required RenderBox parentBox, 
    required SliderThemeData sliderTheme, 
    required TextDirection textDirection, 
    required double value, 
    required double textScaleFactor, 
    required Size sizeWithOverflow}) {
      
    final Canvas canvas = context.canvas;
    final Paint fillPaint = Paint()
      ..color = sliderTheme.thumbColor ?? Colors.cyan
      ..style = PaintingStyle.fill;
      
    final Paint glowPaint = Paint()
      ..color = (sliderTheme.thumbColor ?? Colors.cyan).withOpacity(0.5)
      ..maskFilter = const MaskFilter.blur(BlurStyle.normal, 8);

    final rect = Rect.fromCenter(center: center, width: 8, height: 24);
    canvas.drawRect(rect, glowPaint);
    canvas.drawRect(rect, fillPaint);
  }
}
