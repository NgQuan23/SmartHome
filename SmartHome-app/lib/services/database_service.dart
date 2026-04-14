import 'package:sqflite/sqflite.dart';
import 'package:path/path.dart';
import '../models/alert_model.dart';

class DatabaseService {
  static final DatabaseService _instance = DatabaseService._internal();
  static Database? _database;

  factory DatabaseService() => _instance;
  DatabaseService._internal();

  Future<Database> get database async {
    if (_database != null) return _database!;
    _database = await _initDatabase();
    return _database!;
  }

  Future<Database> _initDatabase() async {
    String path = join(await getDatabasesPath(), 'smarthome_alerts.db');
    return await openDatabase(
      path,
      version: 1,
      onCreate: (db, version) {
        return db.execute(
          'CREATE TABLE alerts(id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT, message TEXT, severity TEXT, timestamp TEXT)',
        );
      },
    );
  }

  Future<void> insertAlert(AlertModel alert) async {
    final db = await database;
    await db.insert('alerts', alert.toMap(), conflictAlgorithm: ConflictAlgorithm.replace);
  }

  Future<List<AlertModel>> getAlerts() async {
    final db = await database;
    final List<Map<String, dynamic>> maps = await db.query('alerts', orderBy: 'timestamp DESC');
    return List.generate(maps.length, (i) => AlertModel.fromMap(maps[i]));
  }

  Future<void> clearAllAlerts() async {
    final db = await database;
    await db.delete('alerts');
  }
}
