package com.example.smarthome.data.repository

import com.example.smarthome.model.Telemetry
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.tasks.await
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class FirebaseRepository @Inject constructor(
    private val database: FirebaseDatabase
) {
    private val deviceId = "device1"
    private val telemetryRef = database.getReference("devices/$deviceId/telemetry")
    private val commandsRef = database.getReference("devices/$deviceId/commands")

    fun getTelemetryUpdates(): Flow<Telemetry> = callbackFlow {
        val listener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val telemetry = snapshot.getValue(Telemetry::class.java)
                if (telemetry != null) {
                    trySend(telemetry)
                }
            }
            override fun onCancelled(error: DatabaseError) {
                close(error.toException())
            }
        }
        telemetryRef.addValueEventListener(listener)
        awaitClose { telemetryRef.removeEventListener(listener) }
    }

    suspend fun sendCommand(action: String, value: Any? = null): Result<Unit> {
        return try {
            val command = mapOf(
                "action" to action,
                "status" to "pending",
                "timestamp" to System.currentTimeMillis(),
                "value" to value
            )
            commandsRef.push().setValue(command).await()
            Result.success(Unit)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
}
