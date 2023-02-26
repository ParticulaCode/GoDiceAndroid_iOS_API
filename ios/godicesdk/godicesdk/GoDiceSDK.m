#import "GoDiceSDK.h"
#import "godiceapi.h"

static void onDiceColorCallback(void *userdata, int diceId, godice_color_t color) {
	GoDiceSDK *instance = (__bridge GoDiceSDK *)userdata;
	if ([[instance delegate] respondsToSelector:@selector(onDiceWithId:color:)]) {
		[instance.delegate onDiceWithId:diceId color:color];
	}
}

static void onDiceStableCallback(void *userdata, int diceId, uint8_t number) {
	GoDiceSDK *instance = (__bridge GoDiceSDK *)userdata;
	if ([[instance delegate] respondsToSelector:@selector(onDiceWithId:roll:)]) {
		[instance.delegate onDiceWithId:diceId roll:number];
	}
}


static void onChargingStateCallback(void *userdata, int diceId, bool charging) {
	GoDiceSDK *instance = (__bridge GoDiceSDK *)userdata;
	if ([[instance delegate] respondsToSelector:@selector(onDiceWithId:chargingStateChanged:)]) {
		[instance.delegate onDiceWithId:diceId chargingStateChanged:charging];
	}
}

static void onChargeLevelCallback(void *userdata, int diceId, uint8_t level) {
	GoDiceSDK *instance = (__bridge GoDiceSDK *)userdata;
	if ([[instance delegate] respondsToSelector:@selector(onDiceWithId:chargeLevel:)]) {
		[instance.delegate onDiceWithId:diceId chargeLevel:level];
	}
}

static godice_callbacks_t g_callbacks = {
	onDiceColorCallback,
	onDiceStableCallback,
	onChargingStateCallback,
	onChargeLevelCallback,
};

@implementation GoDiceSDK

+ (GoDiceSDK*)shared {
    static GoDiceSDK *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (void)incomingPacket:(NSData*)data fromDiceWithId:(NSInteger)diceId type:(GoDiceType)diceType {
	godice_incoming_packet(&g_callbacks, (__bridge void *)(self), (int)diceId, (int)diceType, (uint8_t*)data.bytes, data.length);
}

- (NSData*)initializationPacketForDiceSensitivity:(uint8_t)diceSensitivity
									 blinksNumber:(uint8_t)blinksNumber
								  lightOnDuration:(NSTimeInterval)lightOnDuration
								 lightOffDuration:(NSTimeInterval)lightOffDuration
											color:(UIColor*)color
										blinkMode:(godice_blink_mode_t)blinkMode
											 leds:(godice_leds_selector_t)leds {

	uint8_t buffer[GODICE_INIT_PACKET_SIZE];
	size_t writtenSize;
	CGFloat red, green, blue;
	[color getRed:&red green:&green blue:&blue alpha:nil];
	godice_toggle_leds_t toggleLeds = {
		.number_of_blinks = blinksNumber,
		.light_on_duration_10ms = (uint8_t)MIN(MAX((NSInteger)(lightOnDuration * 100), 0), 255),
		.light_off_duration_10ms = (uint8_t)MIN(MAX((NSInteger)(lightOffDuration * 100), 0), 255),
		.color_red = (uint8_t)(red * 255),
		.color_green = (uint8_t)(green * 255),
		.color_blue = (uint8_t)(blue * 255),
		.blink_mode = blinkMode,
		.leds = leds,
	};
	if (godice_init_packet(buffer, sizeof(buffer), &writtenSize, (int)diceSensitivity, &toggleLeds) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)openLedsPacketForColor1:(UIColor*)color1 color2:(UIColor*)color2 {
	uint8_t buffer[GODICE_OPEN_LEDS_PACKET_SIZE];
	size_t writtenSize;
	CGFloat red1, green1, blue1, red2, green2, blue2;
	[color1 getRed:&red1 green:&green1 blue:&blue1 alpha:nil];
	[color2 getRed:&red2 green:&green2 blue:&blue2 alpha:nil];
	if (godice_open_leds_packet(buffer, sizeof(buffer), &writtenSize,
		(uint8_t)(red1 * 255), (uint8_t)(green1 * 255), (uint8_t)(blue1 * 255),
		(uint8_t)(red2 * 255), (uint8_t)(green2 * 255), (uint8_t)(blue2 * 255)) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)toggleLedsPacketForBlinksNumber:(uint8_t)blinksNumber
						   lightOnDuration:(NSTimeInterval)lightOnDuration
						  lightOffDuration:(NSTimeInterval)lightOffDuration
									 color:(UIColor*)color
								 blinkMode:(godice_blink_mode_t)blinkMode
									  leds:(godice_leds_selector_t)leds {

	uint8_t buffer[GODICE_TOGGLE_LEDS_PACKET_SIZE];
	size_t writtenSize;
	CGFloat red, green, blue;
	[color getRed:&red green:&green blue:&blue alpha:nil];
	godice_toggle_leds_t toggleLeds = {
		.number_of_blinks = blinksNumber,
		.light_on_duration_10ms = (uint8_t)MIN(MAX((NSInteger)(lightOnDuration * 100), 0), 255),
		.light_off_duration_10ms = (uint8_t)MIN(MAX((NSInteger)(lightOffDuration * 100), 0), 255),
		.color_red = (uint8_t)(red * 255),
		.color_green = (uint8_t)(green * 255),
		.color_blue = (uint8_t)(blue * 255),
		.blink_mode = blinkMode,
		.leds = leds,
	};
	if (godice_toggle_leds_packet(buffer, sizeof(buffer), &writtenSize, &toggleLeds) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)closeToggleLedsPacket {
	uint8_t buffer[GODICE_CLOSE_TOGGLE_LEDS_PACKET_SIZE];
	size_t writtenSize;
	if (godice_close_toggle_leds_packet(buffer, sizeof(buffer), &writtenSize) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)getColorPacket {
	uint8_t buffer[GODICE_GET_COLOR_PACKET_SIZE];
	size_t writtenSize;
	if (godice_get_color_packet(buffer, sizeof(buffer), &writtenSize) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)getChargeLevelPacket {
	uint8_t buffer[GODICE_GET_CHARGE_LEVEL_PACKET_SIZE];
	size_t writtenSize;
	if (godice_get_charge_level_packet(buffer, sizeof(buffer), &writtenSize) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

- (NSData*)detectionSettingsUpdatePacketForSamplesCount:(uint8_t)samplesCount
										  movementCount:(uint8_t)movementCount
											  faceCount:(uint8_t)faceCount
											 minFlatDeg:(uint8_t)minFlatDeg
											 maxFlatDeg:(uint8_t)maxFlatDeg
											 weakStable:(uint8_t)weakStable
											movementDeg:(uint8_t)movementDeg
										  rollThreshold:(uint8_t)rollThreshold {
	uint8_t buffer[GODICE_DETECTION_SETTINGS_UPDATE_PACKET_SIZE];
	size_t writtenSize;
	if (godice_detection_settings_update_packet(buffer, sizeof(buffer), &writtenSize,
												samplesCount, movementCount, faceCount,
												minFlatDeg, maxFlatDeg, weakStable,
												movementDeg, rollThreshold) != GODICE_OK) {
		return nil;
	}
	return [[NSData alloc] initWithBytes:buffer length:writtenSize];
}

@end
