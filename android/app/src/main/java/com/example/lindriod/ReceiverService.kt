package com.example.lindriod

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Build
import android.os.Environment
import android.os.IBinder
import androidx.core.app.NotificationCompat
import kotlinx.coroutines.*
import java.io.File
import java.io.FileOutputStream
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.ServerSocket
import java.nio.ByteBuffer
import java.nio.ByteOrder

class ReceiverService : Service() {
    companion object {
        var isRunning = false
    }

    private val serviceJob = SupervisorJob()
    private val scope = CoroutineScope(Dispatchers.IO + serviceJob)

    private var udpSocket: DatagramSocket? = null
    private var tcpSocket: ServerSocket? = null

    override fun onCreate() {
        super.onCreate()
        isRunning = true

        createNotificationChannel()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            startForeground(1, buildNotification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC)
        } else {
            startForeground(1, buildNotification())
        }

        scope.launch { runUdpDiscovery() }
        scope.launch { runTcpServer() }
    }

    override fun onDestroy() {
        super.onDestroy()
        isRunning = false
        serviceJob.cancel()
        udpSocket?.close()
        tcpSocket?.close()
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun runUdpDiscovery() {
        try {
            udpSocket = DatagramSocket(8081).apply { broadcast = true }
            val buffer = ByteArray(64)
            val packet = DatagramPacket(buffer, buffer.size)

            while (isRunning) {
                udpSocket?.receive(packet)
                val receivedText = String(packet.data, 0, packet.length).trimEnd('\u0000')

                if (receivedText == "LINDRIOD_SHARE_DISCOVERY_REQUEST") {
                    val pongBytes = "LINDRIOD_SHARE_DISCOVERY_ACCEPT".toByteArray()
                    val replyPacket = DatagramPacket(pongBytes, pongBytes.size, packet.address, packet.port)
                    udpSocket?.send(replyPacket)
                }
            }
        } catch (e: Exception) {
        }
    }

    private fun runTcpServer() {
        try {
            tcpSocket = ServerSocket(8080)
            val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
            val saveDir = File(downloadsDir, "LinDriod")
            if (!saveDir.exists()) saveDir.mkdirs()

            while (isRunning) {
                val client = tcpSocket?.accept() ?: continue

                scope.launch {
                    try {
                        val input = client.getInputStream()
                        val headerBytes = ByteArray(272)
                        var bytesReadTotal = 0
                        while (bytesReadTotal < 272) {
                            val read = input.read(headerBytes, bytesReadTotal, 272 - bytesReadTotal)
                            if (read == -1) break
                            bytesReadTotal += read
                        }

                        val buffer = ByteBuffer.wrap(headerBytes).order(ByteOrder.LITTLE_ENDIAN)
                        val magic = buffer.int
                        val padding = buffer.int
                        val fileSize = buffer.long

                        val nameBytes = ByteArray(256)
                        buffer.get(nameBytes)
                        val fileName = String(nameBytes).trimEnd('\u0000')


                        if (magic == 0x45566773) {
                            val outFile = File(saveDir, fileName)
                            val output = FileOutputStream(outFile)

                            val chunk = ByteArray(4096)
                            var totalReceived = 0L

                            while (totalReceived < fileSize) {
                                val bytesRead = input.read(chunk)
                                if (bytesRead == -1) break
                                output.write(chunk, 0, bytesRead)
                                totalReceived += bytesRead
                            }
                            output.close()
                            android.media.MediaScannerConnection.scanFile(this@ReceiverService, arrayOf(outFile.absolutePath), null, null)
                        }
                    } catch (e: Exception) {
                        e.printStackTrace()
                    } finally {
                        client.close()
                    }
                }
            }
        } catch (e: Exception) {
        }
    }

    private fun createNotificationChannel() {
        val channel = NotificationChannel("lindriod_channel", "LinDriod Daemon", NotificationManager.IMPORTANCE_LOW)
        getSystemService(NotificationManager::class.java)?.createNotificationChannel(channel)
    }

    private fun buildNotification() = NotificationCompat.Builder(this, "lindriod_channel")
        .setContentTitle("LinDriod is Active")
        .setContentText("Listening for incoming files...")
        .setSmallIcon(android.R.drawable.stat_sys_download)
        .build()
}