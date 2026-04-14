package com.example.smarthome.di

import android.content.Context
import androidx.room.Room
import com.example.smarthome.data.local.AppDatabase
import com.example.smarthome.data.local.dao.AlertDao
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object DatabaseModule {

    @Provides
    @Singleton
    fun provideDatabase(@ApplicationContext context: Context): AppDatabase {
        return Room.databaseBuilder(
            context,
            AppDatabase::class.java,
            "smarthome_db"
        ).build()
    }

    @Provides
    fun provideAlertDao(database: AppDatabase): AlertDao {
        return database.alertDao()
    }
}
