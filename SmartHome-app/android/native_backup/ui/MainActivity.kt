package com.example.smarthome.ui

import android.os.Bundle
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.example.smarthome.R
import com.example.smarthome.databinding.ActivityMainBinding
import com.example.smarthome.ui.dashboard.DashboardViewModel
import com.example.smarthome.util.AlertManager
import com.example.smarthome.util.ColorHelper
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val viewModel: DashboardViewModel by viewModels()
    private lateinit var alertManager: AlertManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        alertManager = AlertManager(this)

        setupObservers()
        setupListeners()
    }

    private fun setupObservers() {
        // Lắng nghe dữ liệu Telemetry thời gian thực
        viewModel.telemetry.observe(this) { data ->
            // Cập nhật Gas UI
            binding.tvGasValue.text = data.gas.toString()
            val gasColor = ColorHelper.getGasColor(data.gas)
            binding.cardGas.setCardBackgroundColor(ContextCompat.getColor(this, gasColor))

            // Kiểm tra ngưỡng để báo động cục bộ
            if (data.gas > 3000) {
                alertManager.triggerCriticalAlert("CẢNH BÁO CHÁY/GA", "Phát hiện nồng độ Gas cực cao!")
            }

            // Cập nhật Khoảng cách UI
            binding.tvDistanceValue.text = if (data.distance < 0) "Out of Range" else "${data.distance} cm"
            
            // Cập nhật Chuyển động UI
            binding.tvMotionStatus.text = if (data.motion == 1) "PHÁT HIỆN" else "BÌNH THƯỜNG"
            if (data.motion == 1) {
                binding.tvMotionStatus.setTextColor(ContextCompat.getColor(this, R.color.critical))
            } else {
                binding.tvMotionStatus.setTextColor(ContextCompat.getColor(this, R.color.safe))
            }
        }

        // Lắng nghe trạng thái gửi lệnh
        viewModel.commandStatus.observe(this) { result ->
            result?.let {
                if (it.isSuccess) {
                    Toast.makeText(this, "Lệnh điều khiển đã được gửi", Toast.LENGTH_SHORT).show()
                } else {
                    Toast.makeText(this, "Gửi lệnh thất bại", Toast.LENGTH_LONG).show()
                }
            }
        }
    }

    private fun setupListeners() {
        binding.btnTurnOffRelay.setOnClickListener {
            viewModel.sendCommand("turnOffRelay")
        }
        binding.btnResetAlerts.setOnClickListener {
            viewModel.sendCommand("resetAlerts")
        }
    }
}
