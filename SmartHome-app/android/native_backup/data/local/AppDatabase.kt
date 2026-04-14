package com.example.smarthome.data.local

import androidx.room.Database
import androidx.room.RoomDatabase
import com.example.smarthome.data.local.dao.AlertDao
import com.example.smarthome.data.local.entity.AlertEntity

@Database(entities = [AlertEntity::class], version = 1, exportSchema = false)
abstract class AppDatabase : RoomDatabase() {
    abstract fun alertDao(): AlertDao
}
