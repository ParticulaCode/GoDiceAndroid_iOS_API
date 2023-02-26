#ifndef __GODICESDK_GODICEAPI_H
#define __GODICESDK_GODICEAPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __OBJC__

#import <Foundation/Foundation.h>

#define GODICE_ENUM_BEGIN(NAME) typedef NS_ENUM(NSInteger, NAME) {
#define GODICE_ENUM_END(NAME) };

#else

#define GODICE_ENUM_BEGIN(NAME) typedef enum {
#define GODICE_ENUM_END(NAME) } NAME;

#endif

#define GODICE_SENSITIVITY_DEFAULT (uint8_t)30
#define GODICE_BLINKS_INFINITE (uint8_t)255
#define GODICE_SAMPLES_COUNT_DEFAULT (uint8_t)4
#define GODICE_MOVEMENT_COUNT_DEFAULT (uint8_t)2
#define GODICE_FACE_COUNT_DEFAULT (uint8_t)1
#define GODICE_MIN_FLAT_DEG_DEFAULT (uint8_t)10
#define GODICE_MAX_FLAT_DEG_DEFAULT (uint8_t)54
#define GODICE_WEAK_STABLE_DEFAULT (uint8_t)20
#define GODICE_MOVEMENT_DEG_DEFAULT (uint8_t)50
#define GODICE_ROLL_THRESHOLD_DEFAULT (uint8_t)30

#define GODICE_INIT_PACKET_SIZE 10
#define GODICE_OPEN_LEDS_PACKET_SIZE 7
#define GODICE_TOGGLE_LEDS_PACKET_SIZE 9
#define GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE 1
#define GODICE_GET_COLOR_PACKET_SIZE 1
#define GODICE_GET_CHARGE_LEVEL_PACKET_SIZE 1
#define GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE 9

#ifdef __cplusplus
extern "C" {
#endif

GODICE_ENUM_BEGIN(godice_status_t)
	GODICE_OK = 0,
	GODICE_INVALID_PACKET = 1,
	GODICE_BUFFER_TOO_SMALL = 2,
	GODICE_INVALID_CALLBACK = 3,
GODICE_ENUM_END(godice_status_t)

GODICE_ENUM_BEGIN(godice_blink_mode_t)
	GODICE_BLINK_ONE_BY_ONE = 0,
	GODICE_BLINK_PARALLEL = 1,
GODICE_ENUM_END(godice_blink_mode_t)

GODICE_ENUM_BEGIN(godice_leds_selector_t)
	GODICE_LEDS_BOTH = 0,
	GODICE_LED1 = 1,
	GODICE_LED2 = 2,
GODICE_ENUM_END(godice_leds_selector_t)

GODICE_ENUM_BEGIN(godice_color_t)
	GODICE_BLACK = 0,
	GODICE_RED = 1,
	GODICE_GREEN = 2,
	GODICE_BLUE = 3,
	GODICE_YELLOW = 4,
	GODICE_ORANGE = 5,
GODICE_ENUM_END(godice_color_t)

typedef struct {
	void (*on_dice_color)(void *userdata, int dice_id, godice_color_t color);
	void (*on_dice_stable)(void *userdata, int dice_id, uint8_t number);
	void (*on_charging_state_chaged)(void *userdata, int dice_id, bool charging);
	void (*on_charge_level)(void *userdata, int dice_id, uint8_t level);
	void (*on_dice_roll)(void *userdata, int dice_id);
} godice_callbacks_t;

typedef struct {
	uint8_t number_of_blinks;
	uint8_t light_on_duration_10ms;
	uint8_t light_off_duration_10ms;
	uint8_t color_red;
	uint8_t color_green;
	uint8_t color_blue;
	godice_blink_mode_t blink_mode;
	godice_leds_selector_t leds;
} godice_toggle_leds_t;

godice_status_t godice_incoming_packet(const godice_callbacks_t *cb, void *cb_userdata,
									   int dice_id, int dice_max, const uint8_t *packet, size_t size);

godice_status_t godice_init_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
								   int dice_sensitivity, const godice_toggle_leds_t *toggle_leds);
godice_status_t godice_open_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
										uint8_t red1, uint8_t green1, uint8_t blue1,
										uint8_t red2, uint8_t green2, uint8_t blue2);
godice_status_t godice_toggle_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
										  const godice_toggle_leds_t *toggle_leds);
godice_status_t godice_close_toggle_leds_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size);
godice_status_t godice_get_color_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size);
godice_status_t godice_get_charge_level_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size);
godice_status_t godice_detection_settings_update_packet(uint8_t *buffer, size_t buffer_size, size_t *written_size,
														uint8_t samples_count, uint8_t movement_count,
														uint8_t face_count, uint8_t min_flat_deg,
														uint8_t max_flat_deg, uint8_t weak_stable,
														uint8_t movement_deg, uint8_t roll_threshold);

#ifdef __cplusplus
}
#endif

#endif // __GODICESDK_GODICEAPI_H
