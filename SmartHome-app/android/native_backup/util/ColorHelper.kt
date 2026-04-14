package com.example.smarthome.util

import com.example.smarthome.R

object ColorHelper {
    fun getGasColor(value: Int): Int {
        return when {
            value > 3000 -> R.color.critical
            value > 1400 -> R.color.warning
            else -> R.color.safe
        }
    }

    fun getDistanceColor(level: Int): Int {
        return when (level) {
            3 -> R.color.critical
            2 -> R.color.warning
            else -> R.color.safe
        }
    }
}
