package com.example.lindriod

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        overridePendingTransition(0, 0)

        if (intent?.action == Intent.ACTION_SEND) {
            val sharedUri = intent.getParcelableExtra(Intent.EXTRA_STREAM) as? Uri

            if (sharedUri != null) {
                lifecycleScope.launch {
                    val ip = Discovery.findLinuxDaemon()

                    if (ip != null) {
                        Transfer.sendFile(applicationContext, sharedUri, ip)
                    }

                    finish()
                    overridePendingTransition(0, 0)
                }
                return
            }
        }

        finish()
        overridePendingTransition(0, 0)
    }
}