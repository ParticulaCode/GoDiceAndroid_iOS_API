#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GoDiceSDK/godiceapi.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, GoDiceType) {
	D4 = 4,
	D6 = 6,
	D8 = 8,
	D10 = 10,
	D12 = 12,
	D20 = 20,
	D10X = 100,
};

@protocol GoDiceSDKDelegate <NSObject>

@optional
/// Response to `getColorPacket`
/// @param diceId unique number identifying dice (passed to `incomingPacket` method)
/// @param color color enum (possible values: `GODICE_BLACK`, `GODICE_RED`, `GODICE_GREEN`,
/// `GODICE_BLUE`, `GODICE_YELLOW`, `GODICE_ORANGE`)
- (void)onDiceWithId:(NSInteger)diceId color:(godice_color_t)color;

/// Called after physical dice roll
/// @param diceId unique number identifying dice (passed to `incomingPacket` method)
/// @param roll current dice top edge value
- (void)onDiceWithId:(NSInteger)diceId roll:(NSInteger)roll;

/// Called when dice charging started or stopped
/// @param diceId unique number identifying dice (passed to `incomingPacket` method)
/// @param charging charging `true` - dice is charging, `false` - dice is not charging
- (void)onDiceWithId:(NSInteger)diceId chargingStateChanged:(BOOL)charging;

/// Response to `getChargeLevelPacket`
/// @param diceId unique number identifying dice (passed to `incomingPacket` method)
/// @param level current charge level (from 0 to 100)
- (void)onDiceWithId:(NSInteger)diceId chargeLevel:(NSUInteger)level;

@end

@interface GoDiceSDK: NSObject

@property (class, readonly) GoDiceSDK *shared;

/// Object that will get all events from SDK
@property (nonatomic, weak) id<GoDiceSDKDelegate> delegate;

/// All values received from dice `read characteristic` should be passed to this method
/// @param data value received from dice `read characteristic`
/// @param diceId unique number identifying dice
/// @param diceType type of shell used with dice (`.D6` if no shell is used)
- (void)incomingPacket:(NSData*)data fromDiceWithId:(NSInteger)diceId type:(GoDiceType)diceType;

/// First message that client can send to the die after establishing a connection to get the
/// current status of the die and run LED blinking pattern
/// @param diceSensitivity dice sensitivity, default is `GODICE_SENSITIVITY_DEFAULT`.
/// @param blinksNumber number of blinks. `GODICE_BLINKS_INFINITE` - infinite
/// @param lightOnDuration light on duration in seconds (from 0.0 to 2.550)
/// @param lightOffDuration light off duration in seconds (from 0.0 to 2.550)
/// @param color LED color
/// @param blinkMode `.GODICE_BLINK_ONE_BY_ONE` - blink LEDs one by one
/// `.GODICE_BLINK_PARALLEL` - blink LEDs in parallel (mixed mode)
/// @param leds in mixed mode specifies LEDs to toggle
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)initializationPacketForDiceSensitivity:(uint8_t)diceSensitivity
									 blinksNumber:(uint8_t)blinksNumber
								  lightOnDuration:(NSTimeInterval)lightOnDuration
								 lightOffDuration:(NSTimeInterval)lightOffDuration
											color:(UIColor*)color
										blinkMode:(godice_blink_mode_t)blinkMode
											 leds:(godice_leds_selector_t)leds;


/// Set LEDs color
/// @param color1 LED 1 color
/// @param color2 LED 2 color
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)openLedsPacketForColor1:(UIColor*)color1 color2:(UIColor*)color2;

/// Run LED blinking pattern
/// @param blinksNumber number of blinks. `GODICE_BLINKS_INFINITE` - infinite
/// @param lightOnDuration light on duration in seconds (from 0.0 to 2.550)
/// @param lightOffDuration light off duration in seconds (from 0.0 to 2.550)
/// @param color LED color
/// @param blinkMode `.GODICE_BLINK_ONE_BY_ONE` - blink LEDs one by one
/// `.GODICE_BLINK_PARALLEL` - blink LEDs in parallel (mixed mode)
/// @param leds in mixed mode specifies LEDs to toggle
- (NSData*)toggleLedsPacketForBlinksNumber:(uint8_t)blinksNumber
						   lightOnDuration:(NSTimeInterval)lightOnDuration
						  lightOffDuration:(NSTimeInterval)lightOffDuration
									 color:(UIColor*)color
								 blinkMode:(godice_blink_mode_t)blinkMode
									  leds:(godice_leds_selector_t)leds;

/// Stop running LED blinking pattern and turn off LEDs
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)closeToggleLedsPacket;

/// Request color of a dice. Response is delivered to `delegate`
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)getColorPacket;

/// Request charge level of a dice. Response is delivered to `delegate`
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)getChargeLevelPacket;

/// Update dice detection settings
/// @param samplesCount
/// @param movementCount
/// @param faceCount
/// @param minFlatDeg
/// @param maxFlatDeg
/// @param weakStable
/// @param movementDeg
/// @param rollThreshold
/// @return packet value that should be written to dice `write characteristic`
- (NSData*)detectionSettingsUpdatePacketForSamplesCount:(uint8_t)samplesCount
										  movementCount:(uint8_t)movementCount
											  faceCount:(uint8_t)faceCount
											 minFlatDeg:(uint8_t)minFlatDeg
											 maxFlatDeg:(uint8_t)maxFlatDeg
											 weakStable:(uint8_t)weakStable
											movementDeg:(uint8_t)movementDeg
										  rollThreshold:(uint8_t)rollThreshold;

@end

NS_ASSUME_NONNULL_END

//! Project version number for GoDiceSDK.
FOUNDATION_EXPORT double GoDiceSDKVersionNumber;

//! Project version string for GoDiceSDK.
FOUNDATION_EXPORT const unsigned char GoDiceSDKVersionString[];
