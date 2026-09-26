package com.example.lindriod

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.SocketTimeoutException

object Discovery {
    private const val DISCOVERY_PORT = 8081

    private const val DISCOVERY_PING = "LINDRIOD_SHARE_DISCOVERY_REQUEST"
    private const val DISCOVERY_PONG = "LINDRIOD_SHARE_DISCOVERY_ACCEPT"

    suspend fun findLinuxDaemon(): String? = withContext(Dispatchers.IO) {
        var resultIp: String? = null
        var socket: DatagramSocket? = null

        try {
            socket = DatagramSocket()
            socket.broadcast = true
            socket.soTimeout = 3000

            val pingBytes = DISCOVERY_PING.toByteArray()
            val broadcastAddress = InetAddress.getByName("255.255.255.255")
            val sendPacket = DatagramPacket(pingBytes, pingBytes.size, broadcastAddress, DISCOVERY_PORT)

            socket.send(sendPacket)

            val buffer = ByteArray(64)
            val receivePacket = DatagramPacket(buffer, buffer.size)

            while (true) {
                try {
                    socket.receive(receivePacket)

                    val receivedText = String(receivePacket.data, 0, receivePacket.length).trimEnd('\u0000')

                    if (receivedText == DISCOVERY_PONG) {
                        val senderAddress = receivePacket.address
                        val isLocal = java.net.NetworkInterface.getByInetAddress(senderAddress) != null

                        if (isLocal) {
                            continue
                        }

                        resultIp = senderAddress.hostAddress
                        break
                    }
                } catch (e: SocketTimeoutException) {
                    break
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            socket?.close()
        }

        resultIp
    }
}