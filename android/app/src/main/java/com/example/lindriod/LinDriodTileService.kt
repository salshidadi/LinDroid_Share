package com.example.lindriod

import android.content.Intent
import android.service.quicksettings.Tile
import android.service.quicksettings.TileService

class LinDriodTileService : TileService() {

    override fun onStartListening() {
        val tile = qsTile ?: return
        tile.state = if (ReceiverService.isRunning) Tile.STATE_ACTIVE else Tile.STATE_INACTIVE
        tile.updateTile()
    }

    override fun onClick() {
        val tile = qsTile ?: return
        val serviceIntent = Intent(this, ReceiverService::class.java)

        if (tile.state == Tile.STATE_INACTIVE || tile.state == Tile.STATE_UNAVAILABLE) {
            tile.state = Tile.STATE_ACTIVE
            startForegroundService(serviceIntent)
        } else {
            tile.state = Tile.STATE_INACTIVE
            stopService(serviceIntent)
        }
        tile.updateTile()
    }
}