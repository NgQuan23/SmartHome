package com.example.smarthome.ui.dashboard

import androidx.lifecycle.*
import com.example.smarthome.data.local.dao.AlertDao
import com.example.smarthome.data.local.entity.AlertEntity
import com.example.smarthome.data.repository.FirebaseRepository
import com.example.smarthome.model.Telemetry
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class DashboardViewModel @Inject constructor(
    private val firebaseRepository: FirebaseRepository,
    private val alertDao: AlertDao
) : ViewModel() {

    private val _telemetry = MutableLiveData<Telemetry>()
    val telemetry: LiveData<Telemetry> = _telemetry

    private val _commandStatus = MutableLiveData<Result<Unit>?>()
    val commandStatus: LiveData<Result<Unit>?> = _commandStatus

    // Thresholds
    private val gasCritical = 3000
    private val distCritical = 2.0f

    init {
        observeTelemetry()
    }

    private fun observeTelemetry() {
        viewModelScope.launch {
            firebaseRepository.getTelemetryUpdates().collectLatest { data ->
                _telemetry.value = data
                checkThresholds(data)
            }
        }
    }

    private suspend fun checkThresholds(data: Telemetry) {
        if (data.gas > gasCritical) {
            saveAlert("GAS", "CRITICAL GAS LEAK!", "CRITICAL")
        }
        if (data.distance > 0 && data.distance < distCritical) {
            saveAlert("PROXIMITY", "Object too close!", "CRITICAL")
        }
        if (data.motion == 1) {
            saveAlert("MOTION", "Unusual motion detected", "WARNING")
        }
    }

    private suspend fun saveAlert(type: String, message: String, severity: String) {
        alertDao.insertAlert(
            AlertEntity(
                type = type,
                message = message,
                severity = severity,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    fun sendCommand(action: String, value: Any? = null) {
        viewModelScope.launch {
            _commandStatus.value = firebaseRepository.sendCommand(action, value)
        }
    }
}
