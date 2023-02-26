package org.sample.godicesdk;

import android.annotation.SuppressLint
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle

@SuppressLint("MissingPermission")
class PermissionsActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val intent = intent

        if (intent == null) {
            finish()
            return
        }

        val bluetooth = intent.getBooleanExtra(BLUETOOTH_KEY, false)
        if (bluetooth) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            this.startActivityForResult(enableBtIntent, BLUETOOTH_ENABLE_REQUEST_CODE)
        } else {
            bluetoothReplied()
        }
    }

    private fun bluetoothReplied() {
        checkPermissions()
    }

    private fun checkPermissions() {
        val permissions = intent.getCharSequenceArrayExtra(PERMISSIONS_KEY) as Array<String>?

        if (permissions != null) {
            requestPermissions(permissions, PERMISSIONS_REQUEST_CODE)
        } else {
            permissionsReplied(arrayOf(), IntArray(0))
        }
    }

    private fun permissionsReplied(permissions: Array<String>, grantResults: IntArray) {
        val callback = PermissionsHelper.permissionsGrantedCallback
        if (callback != null) {
            callback(permissions, grantResults)
        }
        finish()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == BLUETOOTH_ENABLE_REQUEST_CODE) {
            bluetoothReplied()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            permissionsReplied(permissions, grantResults)
        }
    }

    companion object {
        private const val BLUETOOTH_ENABLE_REQUEST_CODE = 0x1ffff
        private const val PERMISSIONS_REQUEST_CODE = 0x2ffff
        private const val PERMISSIONS_KEY = "PERMISSIONS_KEY"
        private const val BLUETOOTH_KEY = "BLUETOOTH_KEY"

        fun requestPermissions(activity: Activity, permissions: Array<String>, bluetooth: Boolean) {
            var needRequestPermissions = false
            for (permission in permissions) {
                needRequestPermissions = needRequestPermissions || activity.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED
            }

            if (needRequestPermissions || bluetooth) {
                val intent = Intent(activity, PermissionsActivity::class.java)
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                if (needRequestPermissions) {
                    intent.putExtra(PERMISSIONS_KEY, permissions)
                }
                if (bluetooth) {
                    intent.putExtra(BLUETOOTH_KEY, bluetooth)
                }
                activity.startActivity(intent)
            }
        }
    }
}