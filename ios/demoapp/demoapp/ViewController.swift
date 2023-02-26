import UIKit
import GoDiceSDK
import CoreBluetooth

let ServiceUUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
let WriteCharUUID = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
let ReadCharUUID = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E")

class ViewController: UIViewController {
	private var central: CBCentralManager!
	private var diceIds: [UUID] = []
	private var connected: Set<CBPeripheral> = []

	@IBOutlet var textView: UITextView!

	override func viewDidLoad() {
		super.viewDidLoad()
		GoDiceSDK.shared.delegate = self
		central = CBCentralManager(delegate: self, queue: nil)
	}
}

extension ViewController: GoDiceSDKDelegate {
	func onDice(withId diceId: Int, roll: Int) {
		DispatchQueue.main.async {
			self.textView.text.append("Dice \(self.diceIds[diceId]) roll \(roll)\n")
		}
	}

	func onDice(withId diceId: Int, color: godice_color_t) {
		var colorString: String? = nil
		switch color {
		case .GODICE_BLACK:
			colorString = "black"
		case .GODICE_RED:
			colorString = "red"
		case .GODICE_GREEN:
			colorString = "green"
		case .GODICE_BLUE:
			colorString = "blue"
		case .GODICE_YELLOW:
			colorString = "yellow"
		case .GODICE_ORANGE:
			colorString = "orange"
		default:
			break
		}
		guard let colorString = colorString else { return }
		DispatchQueue.main.async {
			self.textView.text.append("Dice \(self.diceIds[diceId]) color \(colorString)\n")
		}
	}

	func onDice(withId diceId: Int, chargeLevel level: UInt) {
		DispatchQueue.main.async {
			self.textView.text.append("Dice \(self.diceIds[diceId]) charge level \(level)\n")
		}
	}

	func onDice(withId diceId: Int, chargingStateChanged charging: Bool) {
		DispatchQueue.main.async {
			self.textView.text.append("Dice \(self.diceIds[diceId]) charging \(charging)\n")
		}
	}
}

extension ViewController: CBCentralManagerDelegate {
	func centralManagerDidUpdateState(_ central: CBCentralManager) {
		switch central.state {
		case .poweredOn:
			central.scanForPeripherals(withServices: [ServiceUUID])
			break
		default:
			break
		}
	}

	func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
		DispatchQueue.main.async {
			if !self.diceIds.contains(peripheral.identifier) {
				self.diceIds.append(peripheral.identifier)
			}
			self.connected.insert(peripheral)
			central.connect(peripheral)
		}
	}

	func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
		DispatchQueue.main.async {
			guard let diceId = self.diceIds.firstIndex(of: peripheral.identifier) else { return }
			self.textView.text.append("Dice \(self.diceIds[diceId]) connected\n")
		}
		peripheral.delegate = self
		peripheral.discoverServices([ServiceUUID])
	}

	func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
		DispatchQueue.main.async {
			self.connected.remove(peripheral)
		}
	}
}

extension ViewController: CBPeripheralDelegate {
	func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
		guard let service = peripheral.services?.first(where: { $0.uuid == ServiceUUID }) else { return }
		peripheral.discoverCharacteristics([ReadCharUUID, WriteCharUUID], for: service)
	}

	func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
		guard let readChar = service.characteristics?.first(where: {$0.uuid == ReadCharUUID}) else { return }
		peripheral.setNotifyValue(true, for: readChar)
		guard let writeChar = service.characteristics?.first(where: {$0.uuid == WriteCharUUID}) else { return }
		peripheral.writeValue(GoDiceSDK.shared.detectionSettingsUpdatePacket(forSamplesCount: GODICE_SAMPLES_COUNT_DEFAULT,
																			 movementCount: GODICE_MOVEMENT_COUNT_DEFAULT,
																			 faceCount: GODICE_FACE_COUNT_DEFAULT,
																			 minFlatDeg: GODICE_MIN_FLAT_DEG_DEFAULT,
																			 maxFlatDeg: GODICE_MAX_FLAT_DEG_DEFAULT,
																			 weakStable: GODICE_WEAK_STABLE_DEFAULT,
																			 movementDeg: GODICE_MOVEMENT_DEG_DEFAULT,
																			 rollThreshold: GODICE_ROLL_THRESHOLD_DEFAULT),
							  for: writeChar, type: .withoutResponse)
		peripheral.writeValue(GoDiceSDK.shared.openLedsPacket(forColor1: .red, color2: .green), for: writeChar, type: .withoutResponse)
//		peripheral.writeValue(GoDiceSDK.shared.initializationPacket(forDiceSensitivity: GODICE_SENSITIVITY_DEFAULT, blinksNumber: 3,
//																	lightOnDuration: 1.0, lightOffDuration: 1.0,
//																	color: .red, blink: .GODICE_BLINK_PARALLEL, leds: .GODICE_LEDS_BOTH),
//							  for: writeChar, type: .withoutResponse)
//		peripheral.writeValue(GoDiceSDK.shared.toggleLedsPacket(forBlinksNumber: 10,
//																lightOnDuration: 1.0,
//																lightOffDuration: 1.0,
//																color: .blue,
//																blink: .GODICE_BLINK_PARALLEL,
//																leds: .GODICE_LEDS_BOTH),
//							  for: writeChar, type: .withoutResponse)
		DispatchQueue.main.asyncAfter(deadline: .now() + 3.0) {
			peripheral.writeValue(GoDiceSDK.shared.closeToggleLedsPacket(), for: writeChar, type: .withoutResponse)
		}
		peripheral.writeValue(GoDiceSDK.shared.getColorPacket(), for: writeChar, type: .withoutResponse)
		peripheral.writeValue(GoDiceSDK.shared.getChargeLevelPacket(), for: writeChar, type: .withoutResponse)
	}

	func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
		guard let packet = characteristic.value else { return }
		DispatchQueue.main.async {
			guard let diceId = self.diceIds.firstIndex(of: peripheral.identifier) else { return }
			GoDiceSDK.shared.incomingPacket(packet, fromDiceWithId: diceId, type: .D6)
		}
	}

	func peripheralIsReady(toSendWriteWithoutResponse peripheral: CBPeripheral) {

	}
}
