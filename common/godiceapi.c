#include "godiceapi.h"
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

//#define LOGGING

#if defined __ANDROID__ && defined LOGGING
	#include <android/log.h>
	#define log(FORMAT, ...) __android_log_print(ANDROID_LOG_WARN, "GoDice", FORMAT, __VA_ARGS__)
#elif defined __OBJC__ && defined LOGGING
	#define log(FORMAT, ...) NSLog(@FORMAT, __VA_ARGS__)
#else
	#define log(FORMAT, ...)
#endif

#define countof(array) (sizeof(array) / sizeof(array[0]))

// Event keys
static const char EK_Battery[] = "Bat";
static const char EK_Roll[] = "R";
static const char EK_Stable[] = "S";
static const char EK_FakeStable[] = "FS";
static const char EK_MoveStable[] = "MS";
static const char EK_TiltStable[] = "TS";
static const char EK_Tap[] = "Tap";
static const char EK_DoubleTap[] = "DTap";
static const char EK_Charging[] = "Char";
static const char EK_Color[] = "Col";

//static const int Sensitivity = 30;
typedef struct __attribute__((__packed__)) {
	int8_t x, y, z;
} axis_t;

static const axis_t D6Values[] = {
	/* 1 */ {  -64,    0,    0 },
	/* 2 */ {    0,    0,   64 },
	/* 3 */ {    0,   64,    0 },
	/* 4 */ {    0,  -64,    0 },
	/* 5 */ {    0,    0,  -64 },
	/* 6 */ {   64,    0,    0 },
};

static const axis_t D20Values[] = {
	/*  1 */ {  -64,    0,  -22 },
	/*  2 */ {   42,  -42,   40 },
	/*  3 */ {    0,   22,  -64 },
	/*  4 */ {    0,   22,   64 },
	/*  5 */ {  -42,  -42,   42 },
	/*  6 */ {   22,   64,    0 },
	/*  7 */ {  -42,  -42,  -42 },
	/*  8 */ {   64,    0,  -22 },
	/*  9 */ {  -22,   64,    0 },
	/* 10 */ {   42,  -42,  -42 },
	/* 11 */ {  -42,   42,   42 },
	/* 12 */ {   22,  -64,    0 },
	/* 13 */ {  -64,    0,   22 },
	/* 14 */ {   42,   42,   42 },
	/* 15 */ {  -22,  -64,    0 },
	/* 16 */ {   42,   42,  -42 },
	/* 17 */ {    0,  -22,  -64 },
	/* 18 */ {    0,  -22,   64 },
	/* 19 */ {  -42,   42,  -42 },
	/* 20 */ {   64,    0,   22 },
};

static const axis_t D24Values[] = {
	/*  1 */ {   20,  -60,  -20 },
	/*  2 */ {   20,    0,   60 },
	/*  3 */ {  -40,  -40,   40 },
	/*  4 */ {  -60,    0,   20 },
	/*  5 */ {   40,   20,   40 },
	/*  6 */ {  -20,  -60,  -20 },
	/*  7 */ {   20,   60,   20 },
	/*  8 */ {  -40,   20,  -40 },
	/*  9 */ {  -40,   40,   40 },
	/* 10 */ {  -20,    0,   60 },
	/* 11 */ {  -20,  -60,   20 },
	/* 12 */ {   60,    0,   20 },
	/* 13 */ {  -60,    0,  -20 },
	/* 14 */ {   20,   60,  -20 },
	/* 15 */ {   20,    0,  -60 },
	/* 16 */ {   40,  -20,  -40 },
	/* 17 */ {  -20,   60,  -20 },
	/* 18 */ {  -40,  -40,  -40 },
	/* 19 */ {   40,  -20,   40 },
	/* 20 */ {   20,  -60,   20 },
	/* 21 */ {   60,    0,  -20 },
	/* 22 */ {   40,   20,  -40 },
	/* 23 */ {  -20,    0,  -60 },
	/* 24 */ {  -20,   60,   20 },
};

static int identity_transform(int roll) {
	return roll;
}

static int d4_transform(int roll) {
  static int lookup[] = {
    /*  1 */   3,
    /*  2 */   1,
    /*  3 */   4,
    /*  4 */   1,
    /*  5 */   4,
    /*  6 */   4,
    /*  7 */   1,
    /*  8 */   4,
    /*  9 */   2,
    /* 10 */   3,
    /* 11 */   1,
    /* 12 */   1,
    /* 13 */   1,
    /* 14 */   4,
    /* 15 */   2,
    /* 16 */   3,
    /* 17 */   3,
    /* 18 */   2,
    /* 19 */   2,
    /* 20 */   2,
    /* 21 */   4,
    /* 22 */   1,
    /* 23 */   3,
    /* 24 */   2,
  };
	return lookup[roll - 1];
}

static int d8_transform(int roll) {
  static int lookup[] = {
    /*  1 */   3,
    /*  2 */   3,
    /*  3 */   6,
    /*  4 */   1,
    /*  5 */   2,
    /*  6 */   8,
    /*  7 */   1,
    /*  8 */   1,
    /*  9 */   4,
    /* 10 */   7,
    /* 11 */   5,
    /* 12 */   5,
    /* 13 */   4,
    /* 14 */   4,
    /* 15 */   2,
    /* 16 */   5,
    /* 17 */   7,
    /* 18 */   7,
    /* 19 */   8,
    /* 20 */   2,
    /* 21 */   8,
    /* 22 */   3,
    /* 23 */   6,
    /* 24 */   6,
  };
  return lookup[roll - 1];
}

static int d10_transform(int roll) {
  static int lookup[] = {
    /*  1 */   8,
    /*  2 */   2,
    /*  3 */   6,
    /*  4 */   1,
    /*  5 */   4,
    /*  6 */   3,
    /*  7 */   9,
    /*  8 */   0,
    /*  9 */   7,
    /* 10 */   5,
    /* 11 */   5,
    /* 12 */   7,
    /* 13 */   0,
    /* 14 */   9,
    /* 15 */   3,
    /* 16 */   4,
    /* 17 */   1,
    /* 18 */   6,
    /* 19 */   2,
    /* 20 */   8,
  };
  return lookup[roll - 1];
}

static int d12_transform(int roll) {
  static int lookup[] = {
    /*  1 */   1,
    /*  2 */   2,
    /*  3 */   3,
    /*  4 */   4,
    /*  5 */   5,
    /*  6 */   6,
    /*  7 */   7,
    /*  8 */   8,
    /*  9 */   9,
    /* 10 */  10,
    /* 11 */  11,
    /* 12 */  12,
    /* 13 */   1,
    /* 14 */   2,
    /* 15 */   3,
    /* 16 */   4,
    /* 17 */   5,
    /* 18 */   6,
    /* 19 */   7,
    /* 20 */   8,
    /* 21 */   9,
    /* 22 */  10,
    /* 23 */  11,
    /* 24 */  12,
  };
  return lookup[roll - 1];
}

static int d10x_transform(int roll) {
	return d10_transform(roll) * 10;
}

typedef struct {
	const int max;
	const axis_t *values;
	const int values_num;
	int (*transform)(int roll);
} diceType_t;

static diceType_t DiceTypes[] = {
	{4, D24Values, countof(D24Values), d4_transform},
	{6, D6Values, countof(D6Values), identity_transform},
	{8, D24Values, countof(D24Values), d8_transform},
	{10, D20Values, countof(D20Values), d10_transform},
	{12, D24Values, countof(D24Values), d12_transform},
	{20, D20Values, countof(D20Values), identity_transform},
	{100, D20Values, countof(D20Values), d10x_transform},
};

static float axis_distance(const axis_t *from, const axis_t *to) {
	float x = (float)to->x - (float)from->x;
	float y = (float)to->y - (float)from->y;
	float z = (float)to->z - (float)from->z;
	return sqrtf(x * x + y * y + z * z);
}

static int axis_to_value(const axis_t values[], size_t values_num, const axis_t *axis) {
	int value = 0;
	float min_dist = FLT_MAX;
	for (int i = 0; i < values_num; i++) {
		float dist = axis_distance(axis, &values[i]);
		log("V[%2d] (%-3d, %-3d, %-3d) dist: %.6f",
			i + 1, (int)values[i].x, (int)values[i].y, (int)values[i].z, dist);
		if (dist < min_dist) {
			value = i + 1;
			min_dist = dist;
		}
	}
	return value;
}

static bool is_event_prefix(const uint8_t *packet, size_t size, const char *key) {
	size_t key_len = strlen(key);
	if (size < key_len) {
		return false;
	}
	return memcmp(packet, key, key_len) == 0;
}

static godice_status_t incoming_roll_packet(const godice_callbacks_t *cb, void *cb_userdata,
											int dice_id, const uint8_t *packet, size_t size) {
	if (cb->on_dice_roll == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (size == 0) {
		return GODICE_INVALID_PACKET;
	}
	cb->on_dice_roll(cb_userdata, dice_id);
	return GODICE_OK;
}

typedef struct __attribute__((__packed__)) {
	uint8_t key;
	axis_t axis;
} stablePacket_t;

static godice_status_t incoming_stable_packet(const godice_callbacks_t *cb, void *cb_userdata,
								   int dice_id, int dice_max,
								   const uint8_t *raw_packet, size_t size,
								   const char *stable_type) {
	if (cb->on_dice_stable == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (size != sizeof(stablePacket_t)) {
		return GODICE_INVALID_PACKET;
	}
	stablePacket_t *packet = (stablePacket_t*)raw_packet;
	log("%cS (%-3d, %-3d, %-3d)",
		stable_type == NULL ? ' ' : *stable_type,
		(int)packet->axis.x,
		(int)packet->axis.y,
		(int)packet->axis.z);
	for (int i = 0; i < countof(DiceTypes); i++) {
		diceType_t *dice_type = &DiceTypes[i];
		if (dice_max == dice_type->max) {
			int raw_roll = axis_to_value(dice_type->values, dice_type->values_num, &packet->axis);
			int transformed_roll = dice_type->transform(raw_roll);
			cb->on_dice_stable(cb_userdata, dice_id, transformed_roll);
			break;
		}
	}
	return GODICE_OK;
}

static godice_status_t incoming_battery_packet(const godice_callbacks_t *cb, void *cb_userdata,
											   int dice_id, const uint8_t *packet, size_t size) {
	if (cb->on_charge_level == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (size != 1 || packet[0] > 100) {
		return GODICE_INVALID_PACKET;
	}
	cb->on_charge_level(cb_userdata, dice_id, packet[0]);
	return GODICE_OK;
}

static godice_status_t incoming_charging_packet(const godice_callbacks_t *cb, void *cb_userdata,
												int dice_id, const uint8_t *packet, size_t size) {
	if (cb->on_charging_state_chaged == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (size != 1 || (packet[0] != 0 && packet[0] != 1)) {
		return GODICE_INVALID_PACKET;
	}
	cb->on_charging_state_chaged(cb_userdata, dice_id, packet[0]);
	return GODICE_OK;
}

static godice_status_t incoming_color_packet(const godice_callbacks_t *cb, void *cb_userdata,
											 int dice_id, const uint8_t *packet, size_t size) {
	if (cb->on_dice_color == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (size != 1) {
		return GODICE_INVALID_PACKET;
	}
	switch (packet[0]) {
		case GODICE_BLACK:
		case GODICE_RED:
		case GODICE_GREEN:
		case GODICE_BLUE:
		case GODICE_YELLOW:
		case GODICE_ORANGE:
			cb->on_dice_color(cb_userdata, dice_id, packet[0]);
			return GODICE_OK;
		default:
			return GODICE_INVALID_PACKET;
	}
}

godice_status_t godice_incoming_packet(const godice_callbacks_t *cb, void *cb_userdata,
									   int dice_id, int dice_max, const uint8_t *packet, size_t size) {
	if (cb == NULL) {
		return GODICE_INVALID_CALLBACK;
	}
	if (is_event_prefix(packet, size, EK_Roll)) {
		return incoming_roll_packet(cb, cb_userdata, dice_id, packet, size);
	}
	if (is_event_prefix(packet, size, EK_Tap)) {
		return GODICE_OK;
	}
	if (is_event_prefix(packet, size, EK_DoubleTap)) {
		return GODICE_OK;
	}
	if (is_event_prefix(packet, size, EK_Battery)) {
		return incoming_battery_packet(cb, cb_userdata, dice_id, packet + sizeof(EK_Battery) - 1, size - sizeof(EK_Battery) + 1);
	}
	if (is_event_prefix(packet, size, EK_Charging)) {
		return incoming_charging_packet(cb, cb_userdata, dice_id, packet + sizeof(EK_Charging) - 1, size - sizeof(EK_Charging) + 1);
	}
	if (is_event_prefix(packet, size, EK_Stable)) {
		return incoming_stable_packet(cb, cb_userdata,
									  dice_id, dice_max,
									  packet, size, NULL);
	}
	if (is_event_prefix(packet, size, EK_FakeStable) ||
		is_event_prefix(packet, size, EK_TiltStable) ||
		is_event_prefix(packet, size, EK_MoveStable)) {

		return incoming_stable_packet(cb, cb_userdata,
									  dice_id, dice_max,
									  packet + 1, size - 1, (char*)packet);
	}
	if (is_event_prefix(packet, size, EK_Color)) {
		return incoming_color_packet(cb, cb_userdata, dice_id, packet + sizeof(EK_Color) - 1, size - sizeof(EK_Color) + 1);
	}
	return GODICE_INVALID_PACKET;
}

godice_status_t godice_init_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
								   int dice_sensitivity, const godice_toggle_leds_t *toggle_leds) {
	if (buffer_size < GODICE_INIT_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x19;
	buffer[1] = dice_sensitivity;
	buffer[2] = toggle_leds->number_of_blinks;
	buffer[3] = toggle_leds->light_on_duration_10ms;
	buffer[4] = toggle_leds->light_off_duration_10ms;
	buffer[5] = toggle_leds->color_red;
	buffer[6] = toggle_leds->color_green;
	buffer[7] = toggle_leds->color_blue;
	buffer[8] = (uint8_t)toggle_leds->blink_mode;
	buffer[9] = (uint8_t)toggle_leds->leds;
	*written_size = GODICE_INIT_PACKET_SIZE;
	return GODICE_OK;
}

godice_status_t godice_open_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
										uint8_t red1, uint8_t green1, uint8_t blue1,
										uint8_t red2, uint8_t green2, uint8_t blue2) {
	if (buffer_size < GODICE_OPEN_LEDS_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x08;
	buffer[1] = red1;
	buffer[2] = green1;
	buffer[3] = blue1;
	buffer[4] = red2;
	buffer[5] = green2;
	buffer[6] = blue2;
	*written_size = GODICE_OPEN_LEDS_PACKET_SIZE;
	return GODICE_OK;
}


godice_status_t godice_toggle_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
										  const godice_toggle_leds_t *toggle_leds) {
	if (buffer_size < GODICE_TOGGLE_LEDS_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x10;
	buffer[1] = toggle_leds->number_of_blinks;
	buffer[2] = toggle_leds->light_on_duration_10ms;
	buffer[3] = toggle_leds->light_off_duration_10ms;
	buffer[4] = toggle_leds->color_red;
	buffer[5] = toggle_leds->color_green;
	buffer[6] = toggle_leds->color_blue;
	buffer[7] = (uint8_t)toggle_leds->blink_mode;
	buffer[8] = (uint8_t)toggle_leds->leds;
	*written_size = GODICE_TOGGLE_LEDS_PACKET_SIZE;
	return GODICE_OK;
}

godice_status_t godice_close_toggle_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size) {
	if (buffer_size < GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x14;
	*written_size = GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE;
	return GODICE_OK;
}

godice_status_t godice_get_color_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size) {
	if (buffer_size < GODICE_GET_COLOR_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x17;
	*written_size = GODICE_GET_COLOR_PACKET_SIZE;
	return GODICE_OK;
}

godice_status_t godice_get_charge_level_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size) {
	if (buffer_size < GODICE_GET_CHARGE_LEVEL_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x03;
	*written_size = GODICE_GET_CHARGE_LEVEL_PACKET_SIZE;
	return GODICE_OK;
}

godice_status_t godice_detection_settings_update_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
														uint8_t samples_count, uint8_t movement_count,
														uint8_t face_count, uint8_t min_flat_deg,
														uint8_t max_flat_deg, uint8_t weak_stable,
														uint8_t movement_deg, uint8_t roll_threshold) {
	if (buffer_size < GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE) {
		return GODICE_BUFFER_TOO_SMALL;
	}
	buffer[0] = 0x65;
	buffer[1] = samples_count;
	buffer[2] = movement_count;
	buffer[3] = face_count;
	buffer[4] = min_flat_deg;
	buffer[5] = max_flat_deg;
	buffer[6] = weak_stable;
	buffer[7] = movement_deg;
	buffer[8] = roll_threshold;
	*written_size = GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE;
	return GODICE_OK;
}
