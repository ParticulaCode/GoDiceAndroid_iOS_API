package org.sample.godicesdk

import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.bluetooth.le.ScanSettings.SCAN_MODE_LOW_LATENCY
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.ParcelUuid
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import org.sample.godicesdklib.GoDiceSDK
import java.util.*

@SuppressLint("MissingPermission")
class Dice(private val id: Int, val device: BluetoothDevice) {
    var gatt: BluetoothGatt? = null
    private var service: BluetoothGattService? = null
    private var writeChar: BluetoothGattCharacteristic? = null
    private var readChar: BluetoothGattCharacteristic? = null
    private var writes: Queue<ByteArray> = LinkedList()
    private var writeInProgress = true
    private var dieName = device.name

    fun onConnected() {
        gatt?.discoverServices()
    }

    fun getDieName(): String? {
        return dieName
    }

    fun onServicesDiscovered() {
        service = gatt?.services?.firstOrNull { it.uuid == serviceUUID }
        writeChar = service?.characteristics?.firstOrNull { it.uuid == writeCharUUID }
        readChar = service?.characteristics?.firstOrNull { it.uuid == readCharUUID }
        readChar?.let {
            gatt?.setCharacteristicNotification(it, true)
            val descriptor = it.getDescriptor(CCCDUUID)
            descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            gatt?.writeDescriptor(descriptor)
        }
    }

    fun onEvent() {
        readChar?.value?.let {
            GoDiceSDK.incomingPacket(id, GoDiceSDK.DiceType.D6, it)
        }
    }

    fun nextWrite() {
        synchronized(writes) {
            writeInProgress = false
            writes.poll()?.let { value ->
                writeChar?.let { char ->
                    char.value = value
                    gatt?.writeCharacteristic(char)
                    writeInProgress = true
                }
            }
        }
    }

    fun scheduleWrite(value: ByteArray) {
        synchronized(writes) {
            writes.add(value)
            if (!writeInProgress) {
                nextWrite()
            }
        }
    }

    companion object {
        val serviceUUID: UUID = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
        val writeCharUUID: UUID = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
        val readCharUUID: UUID = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")
        val CCCDUUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
    }
}

@SuppressLint("MissingPermission")
class MainActivity : AppCompatActivity(), GoDiceSDK.Listener {
    var textView: TextView? = null
    var scrollView: ScrollView? = null
    private var deviceList: LinearLayout? = null
    private val dices = HashMap<String, Dice>()
    private var diceIds = LinkedList<String>()


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        textView = findViewById(R.id.textView)
        scrollView = findViewById(R.id.scrollView)
        deviceList = findViewById(R.id.device_list)
        val scanButton: Button = findViewById(R.id.scanButton)

        GoDiceSDK.listener = this
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        val bluetoothAdapter = bluetoothManager.adapter
        scanButton.setOnClickListener {
            PermissionsHelper.requestPermissions(this, bluetoothAdapter) {
                scanButton.isEnabled = false
                startScan()
            }
        }
    }

    private fun startScan() {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        val bluetoothAdapter = bluetoothManager.adapter
        val bluetoothScanner = bluetoothAdapter.bluetoothLeScanner

        val filters = LinkedList<ScanFilter>()
        filters.add(ScanFilter.Builder().setServiceUuid(ParcelUuid(Dice.serviceUUID)).build())
        val scanSettings = ScanSettings.Builder()
            .setScanMode(SCAN_MODE_LOW_LATENCY)
            .build()
        val activity = this
        val scanCallback = object: ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                super.onScanResult(callbackType, result)
                runOnMainThread {
                    activity.onScanResult(result)
                }
            }

            override fun onBatchScanResults(results: List<ScanResult>) {
                super.onBatchScanResults(results)
                runOnMainThread {
                    for (result in results) {
                        activity.onScanResult(result)
                    }
                }
            }
        }
        bluetoothScanner.startScan(filters, scanSettings, scanCallback)
    }

    fun onScanResult(result: ScanResult) {
        if (dices.containsKey(result.device.address) || result.device.name == null || !result.device.name.contains("GoDice")) {
            return
        }
        var diceId = diceIds.indexOf(result.device.address)
        if (diceId < 0) {
            diceId = diceIds.size
            diceIds.add(result.device.address)
        }
        val dice = Dice(diceId, result.device)
        dice.scheduleWrite(GoDiceSDK.openLedsPacket(0xff0000, 0x00ff00))
        Timer().schedule(object: TimerTask() {
            override fun run() {
                dice.scheduleWrite(GoDiceSDK.closeToggleLedsPacket())
            }
        }, 3000)

//        dice.scheduleWrite(GoDiceSDK.toggleLedsPacket(
//            6,
//            1.0f, 1.0f,
//            0xff0000,
//            GoDiceSDK.DiceBlinkMode.PARALLEL,
//            GoDiceSDK.DiceLedsSelector.BOTH))
        dice.scheduleWrite(GoDiceSDK.getColorPacket())
        dice.scheduleWrite(GoDiceSDK.getChargeLevelPacket())
        dices[result.device.address] = dice

        val button = Button(this@MainActivity)
        button.text = result.device.name
        button.setOnClickListener {
            button.isEnabled = false
            dice.gatt = result.device.connectGatt(this, true, object : BluetoothGattCallback() {
                override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
                    super.onConnectionStateChange(gatt, status, newState)
                    if (newState == BluetoothProfile.STATE_CONNECTED) {
                        runOnUiThread(Runnable {
                            textView?.append("Dice ${dice.getDieName()} Connected\n")
                            scrollView?.fullScroll(View.FOCUS_DOWN)
                        })
                        dice.onConnected()
                    }
                }

                override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
                    super.onServicesDiscovered(gatt, status)
                    dice.onServicesDiscovered()
                }

                override fun onCharacteristicChanged(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic?) {
                    super.onCharacteristicChanged(gatt, characteristic)
                    dice.onEvent()
                }

                override fun onDescriptorWrite(gatt: BluetoothGatt?, descriptor: BluetoothGattDescriptor?, status: Int) {
                    super.onDescriptorWrite(gatt, descriptor, status)
                    dice.nextWrite()
                }

                override fun onCharacteristicWrite(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic?, status: Int) {
                    super.onCharacteristicWrite(gatt, characteristic, status)
                    dice.nextWrite()
                }
            })
        }
        deviceList?.addView(button)
    }

    internal fun runOnMainThread(runnable: () -> Unit) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            runnable()
        } else {
            Handler(Looper.getMainLooper()).post {
                runnable()
            }
        }
    }

    override fun onDiceColor(diceId: Int, color: Int) {
        val name: String = when (color) {
            GoDiceSDK.DICE_BLACK -> "black"
            GoDiceSDK.DICE_RED -> "red"
            GoDiceSDK.DICE_GREEN -> "green"
            GoDiceSDK.DICE_BLUE -> "blue"
            GoDiceSDK.DICE_YELLOW -> "yellow"
            GoDiceSDK.DICE_ORANGE -> "orange"
            else -> {
                return
            }
        }
        textView?.append("Dice ${dices[diceIds[diceId]]?.getDieName()} Color: ${name}\n")
        scrollView?.fullScroll(View.FOCUS_DOWN)
    }

    override fun onDiceStable(diceId: Int, number: Int) {
        textView?.append("Dice ${dices[diceIds[diceId]]?.getDieName()} Stable: ${number}\n")
        scrollView?.fullScroll(View.FOCUS_DOWN)
    }

    override fun onDiceRoll(diceId: Int, number: Int) {
        textView?.append("Dice ${dices[diceIds[diceId]]?.getDieName()} Rolling\n")
        scrollView?.fullScroll(View.FOCUS_DOWN)
    }

    override fun onDiceChargingStateChanged(diceId: Int, charging: Boolean) {
        textView?.append("Dice ${dices[diceIds[diceId]]?.getDieName()} Charging: ${charging}\n")
        scrollView?.fullScroll(View.FOCUS_DOWN)
    }

    override fun onDiceChargeLevel(diceId: Int, level: Int) {
        textView?.append("Dice ${dices[diceIds[diceId]]?.getDieName()} Battery Level: ${level}\n")
        scrollView?.fullScroll(View.FOCUS_DOWN)
    }
}
