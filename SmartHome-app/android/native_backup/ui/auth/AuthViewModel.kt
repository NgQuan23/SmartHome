package com.example.smarthome.ui.auth

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.smarthome.data.repository.AuthRepository
import com.google.firebase.auth.FirebaseUser
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class AuthViewModel @Inject constructor(
    private val authRepository: AuthRepository
) : ViewModel() {

    private val _userState = MutableLiveData<FirebaseUser?>(authRepository.currentUser)
    val userState: LiveData<FirebaseUser?> = _userState

    private val _loginResult = MutableLiveData<Result<FirebaseUser>?>()
    val loginResult: LiveData<Result<FirebaseUser>?> = _loginResult

    fun login(email: String, password: String) {
        viewModelScope.launch {
            val result = authRepository.login(email, password)
            _loginResult.value = result
            if (result.isSuccess) {
                _userState.value = result.getOrNull()
            }
        }
    }

    fun logout() {
        authRepository.logout()
        _userState.value = null
    }

    fun isUserLoggedIn() = authRepository.isUserLoggedIn()
}
