package com.example.smarthome.data.local.entity

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "alerts")
data class AlertEntity(
    @PrimaryKey(autoGenerate = true) val id: Int = 0,
    val type: String,
    val message: String,
    val severity: String,
    val timestamp: Long
)
