import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:provider/provider.dart';
import 'theme.dart';
import 'viewmodels/dashboard_viewmodel.dart';
import 'viewmodels/alert_viewmodel.dart';
import 'views/dashboard_screen.dart';
import 'views/alerts_screen.dart';
import 'views/settings_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  try {
    await Firebase.initializeApp(
      options: const FirebaseOptions(
        apiKey: "AIzaSyAPHaUEbzXHmLfCRPRqJ27c4NPZIVfFyk8",
        appId: "1:1016354783320:android:c39487c6030944",
        messagingSenderId: "1053000000000",
        projectId: "smart-home-1c235",
        databaseURL: "https://smart-home-1c235-default-rtdb.asia-southeast1.firebasedatabase.app",
      ),
    );

    final auth = FirebaseAuth.instance;
    if (auth.currentUser == null) {
      await auth.signInWithEmailAndPassword(
        email: "quanylksnb@gmail.com",
        password: "0912848144",
      );
    }
    debugPrint("Firebase Connected!");
  } catch (e) {
    debugPrint("Firebase Error: $e");
  }

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => AlertViewModel()),
        ChangeNotifierProxyProvider<AlertViewModel, DashboardViewModel>(
          create: (_) => DashboardViewModel(),
          update: (_, alertVM, dashboardVM) => dashboardVM!..setAlertViewModel(alertVM),
        ),
      ],
      child: const SmartHomeApp(),
    ),
  );
}

class SmartHomeApp extends StatelessWidget {
  const SmartHomeApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Smart Home Security',
      theme: AppTheme.darkTheme,
      home: const MainNavigator(),
    );
  }
}

class MainNavigator extends StatefulWidget {
  const MainNavigator({super.key});
  @override
  State<MainNavigator> createState() => _MainNavigatorState();
}

class _MainNavigatorState extends State<MainNavigator> {
  int _selectedIndex = 0;
  late final List<Widget> _screens;
  late final PageController _pageController;

  @override
  void initState() {
    super.initState();
    _pageController = PageController(initialPage: _selectedIndex);
    _screens = [const DashboardScreen(), const AlertsScreen(), const SettingsScreen()];
  }

  @override
  void dispose() {
    _pageController.dispose();
    super.dispose();
  }

  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
    _pageController.animateToPage(
      index,
      duration: const Duration(milliseconds: 300),
      curve: Curves.easeInOut,
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: PageView(
        controller: _pageController,
        onPageChanged: (index) {
          setState(() {
            _selectedIndex = index;
          });
        },
        physics: const ClampingScrollPhysics(), // Provide standard physics for swipe
        children: _screens,
      ),
      bottomNavigationBar: BottomNavigationBar(
        currentIndex: _selectedIndex,
        onTap: _onItemTapped,
        items: const [
          BottomNavigationBarItem(icon: Icon(Icons.dashboard), label: 'Dashboard'),
          BottomNavigationBarItem(icon: Icon(Icons.history), label: 'Alerts'),
          BottomNavigationBarItem(icon: Icon(Icons.settings), label: 'Settings'),
        ],
      ),
    );
  }
}
