#include <jni.h>
#include "godiceapi.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct {
	JNIEnv *env;
	jclass type;
} class_spec_t;

typedef struct {
	jobject object;
	jmethodID method_id;
} instance_method_t;

static instance_method_t listenerMethod(class_spec_t *class_spec,
										const char *name, const char *signature) {
	instance_method_t result = { NULL, NULL };
	jfieldID listener_field = (*class_spec->env)->
		GetStaticFieldID(class_spec->env, class_spec->type,
						 "listener", "Lorg/sample/godicesdklib/GoDiceSDK$Listener;");
	jobject listener = (*class_spec->env)->
			GetStaticObjectField(class_spec->env, class_spec->type, listener_field);
	if (listener == NULL) {
		return result;
	}
	jclass listener_class = (*class_spec->env)->GetObjectClass(class_spec->env, listener);
	result.object = listener;
	result.method_id = (*class_spec->env)->
		GetMethodID(class_spec->env, listener_class, name, signature);
	return result;
}

static void onDiceColorCallback(void *userdata, int dice_id, godice_color_t color) {
	class_spec_t *class_spec = (class_spec_t*)userdata;
	instance_method_t method = listenerMethod(class_spec, "onDiceColor", "(II)V");
	if (method.object == NULL) {
		return;
	}
	(*class_spec->env)->CallVoidMethod(class_spec->env, method.object, method.method_id,
									   dice_id, (jint)color);
}

static void onDiceStableCallback(void *userdata, int dice_id, uint8_t number) {
	class_spec_t *class_spec = (class_spec_t*)userdata;
	instance_method_t method = listenerMethod(class_spec, "onDiceStable", "(II)V");
	if (method.object == NULL) {
		return;
	}
	(*class_spec->env)->CallVoidMethod(class_spec->env, method.object, method.method_id,
									   dice_id, (jint)number);
}

static void onDiceRollCallback(void *userdata, int dice_id) {
	class_spec_t *class_spec = (class_spec_t*)userdata;
	instance_method_t method = listenerMethod(class_spec, "onDiceRoll", "(II)V");
	if (method.object == NULL) {
		return;
	}
	(*class_spec->env)->CallVoidMethod(class_spec->env, method.object, method.method_id,
									   dice_id, (jint)0);
}

static void onChargingStateChangedCallback(void *userdata, int dice_id, bool charging) {
	class_spec_t *class_spec = (class_spec_t*)userdata;
	instance_method_t method = listenerMethod(class_spec, "onDiceChargingStateChanged", "(IZ)V");
	if (method.object == NULL) {
		return;
	}
	(*class_spec->env)->CallVoidMethod(class_spec->env, method.object, method.method_id,
									   dice_id, (jboolean)charging);
}

static void onChargeLevelCallback(void *userdata, int dice_id, uint8_t level) {
	class_spec_t *class_spec = (class_spec_t*)userdata;
	instance_method_t method = listenerMethod(class_spec, "onDiceChargeLevel", "(II)V");
	if (method.object == NULL) {
		return;
	}
	(*class_spec->env)->CallVoidMethod(class_spec->env, method.object, method.method_id,
									   dice_id, (jint)level);
}

static godice_callbacks_t g_callbacks = {
	onDiceColorCallback,
	onDiceStableCallback,
	onChargingStateChangedCallback,
	onChargeLevelCallback,
	onDiceRollCallback,
};

JNIEXPORT void JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_incomingPacket(JNIEnv *env, jclass type,
													  jint dice_id, jobject dice_type,
													  jbyteArray packet) {
	class_spec_t class_spec = {env, type};
	jclass dice_type_class = (*env)->GetObjectClass(env, dice_type);
	jfieldID dice_type_max_field = (*env)->GetFieldID(env, dice_type_class, "max", "I");
	jint dice_max = (*env)->GetIntField(env, dice_type, dice_type_max_field);
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	jsize packet_size = (*env)->GetArrayLength(env, packet);
	godice_incoming_packet(&g_callbacks, &class_spec, (int)dice_id, (int)dice_max,
		(uint8_t*)packet_data, (size_t)packet_size);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_initializationPacket(JNIEnv *env, jclass type,
															jint dice_sensitivity,
															jint blinks_number,
															jfloat on_duration,
															jfloat off_duration,
															jint color,
															jobject blink,
															jobject leds) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_INIT_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}

	jclass blink_class = (*env)->GetObjectClass(env, blink);
	jfieldID blink_raw_field = (*env)->GetFieldID(env, blink_class, "raw", "I");
	jint blink_raw = (*env)->GetIntField(env, blink, blink_raw_field);

	jclass leds_class = (*env)->GetObjectClass(env, leds);
	jfieldID leds_raw_field = (*env)->GetFieldID(env, leds_class, "raw", "I");
	jint leds_raw = (*env)->GetIntField(env, leds, leds_raw_field);

	godice_toggle_leds_t toggle_leds;
	toggle_leds.number_of_blinks = (uint8_t)MIN(MAX(blinks_number, 0), 255);
	toggle_leds.light_on_duration_10ms = (uint8_t)MIN(MAX((int)(on_duration * 100), 0), 255);
	toggle_leds.light_off_duration_10ms = (uint8_t)MIN(MAX((int)(off_duration * 100), 0), 255);
	toggle_leds.color_red = (color >> 16) & 0xff;
	toggle_leds.color_green = (color >> 8) & 0xff;
	toggle_leds.color_blue = color & 0xff;
	toggle_leds.blink_mode = blink_raw;
	toggle_leds.leds = leds_raw;

	size_t written_size;
	godice_status_t status = godice_init_packet(
		(uint8_t*)packet_data, GODICE_INIT_PACKET_SIZE, &written_size,
		dice_sensitivity, &toggle_leds);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_openLedsPacket(JNIEnv *env, jclass type,
													  jint color1, jint color2) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_OPEN_LEDS_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}
	size_t written_size;
	godice_status_t status = godice_open_leds_packet(
			(uint8_t*)packet_data, GODICE_OPEN_LEDS_PACKET_SIZE, &written_size,
			(color1 >> 16) & 0xff, (color1 >> 8) & 0xff, color1 & 0xff,
			(color2 >> 16) & 0xff, (color2 >> 8) & 0xff, color2 & 0xff);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}


JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_toggleLedsPacket(JNIEnv *env, jclass type,
														jint blinks_number,
														jfloat on_duration,
														jfloat off_duration,
														jint color,
														jobject blink,
														jobject leds) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_TOGGLE_LEDS_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}

	jclass blink_class = (*env)->GetObjectClass(env, blink);
	jfieldID blink_raw_field = (*env)->GetFieldID(env, blink_class, "raw", "I");
	jint blink_raw = (*env)->GetIntField(env, blink, blink_raw_field);

	jclass leds_class = (*env)->GetObjectClass(env, leds);
	jfieldID leds_raw_field = (*env)->GetFieldID(env, leds_class, "raw", "I");
	jint leds_raw = (*env)->GetIntField(env, leds, leds_raw_field);

	godice_toggle_leds_t toggle_leds;
	toggle_leds.number_of_blinks = (uint8_t)MIN(MAX(blinks_number, 0), 255);
	toggle_leds.light_on_duration_10ms = (uint8_t)MIN(MAX((int)(on_duration * 100), 0), 255);
	toggle_leds.light_off_duration_10ms = (uint8_t)MIN(MAX((int)(off_duration * 100), 0), 255);
	toggle_leds.color_red = (color >> 16) & 0xff;
	toggle_leds.color_green = (color >> 8) & 0xff;
	toggle_leds.color_blue = color & 0xff;
	toggle_leds.blink_mode = blink_raw;
	toggle_leds.leds = leds_raw;

	size_t written_size;
	godice_status_t status = godice_toggle_leds_packet(
		(uint8_t*)packet_data, GODICE_TOGGLE_LEDS_PACKET_SIZE, &written_size, &toggle_leds);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_closeToggleLedsPacket(JNIEnv *env, jclass type) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}
	size_t written_size;
	godice_status_t status = godice_close_toggle_leds_packet(
		(uint8_t*)packet_data, GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE, &written_size);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_getColorPacket(JNIEnv *env, jclass type) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_GET_COLOR_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}
	size_t written_size;
	godice_status_t status = godice_get_color_packet(
		(uint8_t*)packet_data, GODICE_GET_COLOR_PACKET_SIZE, &written_size);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_getChargeLevelPacket(JNIEnv *env, jclass type) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_GET_CHARGE_LEVEL_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}
	size_t written_size;
	godice_status_t status = godice_get_charge_level_packet(
		(uint8_t*)packet_data, GODICE_GET_CHARGE_LEVEL_PACKET_SIZE, &written_size);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL
Java_org_sample_godicesdklib_GoDiceSDK_detectionSettingsUpdatePacket(JNIEnv *env, jclass type,
																	 jint samplesCount,
																	 jint movementCount,
																	 jint faceCount,
																	 jint minFlatDeg,
																	 jint maxFlatDeg,
																	 jint weakStable,
																	 jint movementDeg,
																	 jint rollThreshold) {
	jbyteArray packet = (*env)->NewByteArray(env, GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE);
	if (packet == NULL) {
		return NULL;
	}
	jbyte *packet_data = (*env)->GetByteArrayElements(env, packet, NULL);
	if (packet_data == NULL) {
		return NULL;
	}
	size_t written_size;
	godice_status_t status = godice_detection_settings_update_packet(
			(uint8_t*)packet_data, GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE, &written_size,
			(uint8_t)samplesCount, (uint8_t)movementCount, (uint8_t)faceCount,
			(uint8_t)minFlatDeg, (uint8_t)maxFlatDeg, (uint8_t)weakStable,
			(uint8_t)movementDeg, (uint8_t)rollThreshold);
	(*env)->ReleaseByteArrayElements(env, packet, packet_data, 0);
	if (status == GODICE_OK) {
		return packet;
	} else {
		return NULL;
	}
}
