package com.example.lindriod

import android.content.Context
import android.net.Uri
import android.provider.OpenableColumns
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.InputStream
import java.net.Socket
import java.nio.ByteBuffer
import java.nio.ByteOrder

object Transfer {
    private const val TCP_PORT = 8080
    private const val PROTOCOL_MAGIC = 0x45566773
    private const val MAX_FILENAME_LEN = 256
    private const val CHUNK_SIZE = 4096

    suspend fun sendFile(context: Context, fileUri: Uri, targetIp: String) = withContext(Dispatchers.IO) {
        var socket: Socket? = null
        var inputStream: InputStream? = null

        try {
            val cursor = context.contentResolver.query(fileUri, null, null, null, null)
            var fileName = "unknown_file"
            var fileSize = 0L

            cursor?.use {
                if (it.moveToFirst()) {
                    val nameIndex = it.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                    val sizeIndex = it.getColumnIndex(OpenableColumns.SIZE)
                    if (nameIndex != -1) fileName = it.getString(nameIndex)
                    if (sizeIndex != -1) fileSize = it.getLong(sizeIndex)
                }
            }

            socket = Socket(targetIp, TCP_PORT)
            val outputStream = socket.getOutputStream()


            val headerSize = 4 + 4 + 8 + MAX_FILENAME_LEN
            val headerBuffer = ByteBuffer.allocate(headerSize).apply {
                order(ByteOrder.LITTLE_ENDIAN)
                putInt(PROTOCOL_MAGIC)
                putInt(0)
                putLong(fileSize)

                val nameBytes = fileName.toByteArray(Charsets.UTF_8).take(MAX_FILENAME_LEN - 1).toByteArray()
                put(nameBytes)
            }


            outputStream.write(headerBuffer.array())

            inputStream = context.contentResolver.openInputStream(fileUri)
            val buffer = ByteArray(CHUNK_SIZE)
            var bytesRead: Int

            while (inputStream?.read(buffer).also { bytesRead = it ?: -1 } != -1) {
                outputStream.write(buffer, 0, bytesRead)
            }

            outputStream.flush()

        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            inputStream?.close()
            socket?.close()
        }
    }
}