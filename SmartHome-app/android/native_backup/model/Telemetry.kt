package com.example.smarthome.model

import com.google.firebase.database.IgnoreExtraProperties

@IgnoreExtraProperties
data class Telemetry(
    val gas: Int = 0,
    val distance: Float = 0f,
    val distance_level: Int = 0,
    val motion: Int = 0,
    val timestamp: Long = 0
)
