package org.sample.godicesdk

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.content.pm.PackageManager
import android.os.Build
import java.lang.ref.WeakReference

object PermissionsHelper {
    var activity: WeakReference<Activity> = WeakReference(null)

    fun requestPermissions(activity: Activity, adapter: BluetoothAdapter?, completion: () -> Unit) {
        this.activity = WeakReference(activity)
        var bluetooth = false
        if (isBLESupported() && (adapter == null || !adapter.isEnabled)) {
            bluetooth = true
        }

        var permissions = emptyArray<String>()
        if (activity.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            permissions += Manifest.permission.ACCESS_FINE_LOCATION
        }

        if (Build.VERSION.SDK_INT >= 31) {
            val pm = arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
            for (permission in pm) {
                if (activity.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                    permissions += permission
                }
            }
        }

        if (bluetooth || permissions.isNotEmpty()) {
            permissionsGrantedCallback = { _, _ ->
                requestPermissions(activity, adapter, completion)
            }
            PermissionsActivity.requestPermissions(activity, permissions, bluetooth)
        } else {
            permissionsGrantedCallback = null
            completion()
        }
    }

    internal fun checkMissingManifestPermissions(): List<String>? {
        var requiredPermissions = arrayOf(
            Manifest.permission.ACCESS_FINE_LOCATION
        )
        if (Build.VERSION.SDK_INT >= 31) {
            requiredPermissions += arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        }
        val missing = mutableListOf<String>()
        for (permission in requiredPermissions) {
            if (activity.get()?.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                missing.add(permission)
            }
        }
        return if (missing.isEmpty()) {
            null
        } else {
            missing
        }
    }

    var permissionsGrantedCallback: ((permissions: Array<String>, grantResults: IntArray)-> Unit)? = null

    private fun isBLESupported(): Boolean {
        if (activity.get()?.packageManager?.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE) == false) {
            return false
        }
        return true
    }
}
