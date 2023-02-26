package org.sample.godicesdklib;

public class GoDiceSDK {
    /**
     * Type of dice shell
     */
    public enum DiceType {
        D4(4),
        D6(6),
        D8(8),
        D10(10),
        D12(12),
        D20(20),
        D10X(100);

        final int max;

        DiceType(int max) {
            this.max = max;
        }
    }

    public enum DiceBlinkMode {
        ONE_BY_ONE(0),
        PARALLEL(1);

        final int raw;

        DiceBlinkMode(int raw) {
            this.raw = raw;
        }
    }

    public enum DiceLedsSelector {
        BOTH(0),
        LED1(1),
        LED2(2);

        final int raw;

        DiceLedsSelector(int raw) {
            this.raw = raw;
        }
    }

    /**
     * Default value to pass as `initializationPacket` `diceSensitivity` parameter
     */
    public static final int DICE_SENSITIVITY_DEFAULT = 30;
    /**
     * Special value to pass as `initializationPacket` or `toggleLedsPacket` `blinksNumber`
     * parameter to make dice keep blinking forever
     */
    public static final int DICE_BLINKS_INFINITE = 255;

    public static final int DICE_SAMPLES_COUNT_DEFAULT = 4;
    public static final int DICE_MOVEMENT_COUNT_DEFAULT = 2;
    public static final int DICE_FACE_COUNT_DEFAULT = 1;
    public static final int DICE_MIN_FLAT_DEG_DEFAULT = 10;
    public static final int DICE_MAX_FLAT_DEG_DEFAULT = 54;
    public static final int DICE_WEAK_STABLE_DEFAULT = 20;
    public static final int DICE_MOVEMENT_DEG_DEFAULT = 50;
    public static final int DICE_ROLL_THRESHOLD_DEFAULT = 30;


    public static final int DICE_BLACK = 0;
    public static final int DICE_RED = 1;
    public static final int DICE_GREEN = 2;
    public static final int DICE_BLUE = 3;
    public static final int DICE_YELLOW = 4;
    public static final int DICE_ORANGE = 5;

    static {
        System.loadLibrary("godicesdklib");
    }

    public interface Listener {
        /**
         * Response to `getColorPacket`
         *
         * @param diceId unique number identifying dice (passed to `incomingPacket` method)
         * @param color color enum (possible values: `DICE_BLACK`, `DICE_RED`, `DICE_GREEN`,
         *              `DICE_BLUE`, `DICE_YELLOW`, `DICE_ORANGE`)
         */
        void onDiceColor(int diceId, int color);

        /**
         * Called after physical dice is stable
         *
         * @param diceId unique number identifying dice (passed to incomingPacket method)
         * @param number current dice top edge value
         */
        void onDiceStable(int diceId, int number);

        /**
         * Called when physical dice is rolling
         *
         * @param diceId unique number identifying dice (passed to incomingPacket method)
         */
        void onDiceRoll(int diceId, int number);

        /**
         * Called when dice charging started or stopped
         *
         * @param diceId unique number identifying dice (passed to `incomingPacket` method)
         * @param charging `true` - dice is charging, `false` - dice is not charging
         */
        void onDiceChargingStateChanged(int diceId, boolean charging);

        /**
         * Response to `getChargeLevelPacket`
         *
         * @param diceId unique number identifying dice (passed to `incomingPacket` method)
         * @param level current charge level (from 0 to 100)
         */
        void onDiceChargeLevel(int diceId, int level);
    }

    /**
     * Object that will get all events from SDK
     */
    public static Listener listener = null;

    /**
     * All values received from dice `read characteristic` should be passed to this method
     *
     * @param diceId unique number identifying dice
     * @param diceType type of shell used with dice (`.D6` if no shell is used)
     * @param packet value received from dice `read characteristic`
     */
    public static native void incomingPacket(int diceId, DiceType diceType, byte[] packet);

    /**
     * First message that client can send to the die after establishing a connection to get the
     * current status of the die and run LED blinking pattern
     *
     * @param diceSensitivity dice sensitivity, default is `DICE_SENSITIVITY_DEFAULT`.
     *                        It may vary based on the selected shell
     * @param blinksNumber number of blinks. DICE_BLINKS_INFINITE - infinite
     * @param onDuration light on duration in seconds (from 0.0 to 2.550)
     * @param offDuration light off duration in seconds (from 0.0 to 2.550)
     * @param color RGB hex color value, 1 byte per component
     * @param blink `.ONE_BY_ONE` - blink LEDs one by one
     *              `.PARALLEL` - blink LEDs in parallel (mixed mode)
     * @param leds in mixed mode specifies LEDs to toggle
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] initializationPacket(int diceSensitivity,
                                                     int blinksNumber,
                                                     float onDuration, float offDuration,
                                                     int color,
                                                     DiceBlinkMode blink,
                                                     DiceLedsSelector leds);

    /**
     * Set LEDs color
     *
     * @param color1 LED 1 color
     * @param color2 LED 2 color
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] openLedsPacket(int color1, int color2);

    /**
     * Run LED blinking pattern
     *
     * @param blinksNumber number of blinks. `DICE_BLINKS_INFINITE` - infinite
     * @param onDuration light on duration in seconds (from 0.0 to 2.550)
     * @param offDuration light off duration in seconds (from 0.0 to 2.550)
     * @param color RGB hex color value, 1 byte per component
     * @param blink `.ONE_BY_ONE` - blink LEDs one by one
     *              `.PARALLEL` - blink LEDs in parallel (mixed mode)
     * @param leds in mixed mode specifies LEDs to toggle
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] toggleLedsPacket(int blinksNumber,
                                                 float onDuration, float offDuration,
                                                 int color,
                                                 DiceBlinkMode blink,
                                                 DiceLedsSelector leds);

    /**
     * Stop running LED blinking pattern
     *
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] closeToggleLedsPacket();

    /**
     * Request color of a dice. Response is delivered to `listener`
     *
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] getColorPacket();

    /**
     * Request charge level of a dice. Response is delivered to `listener`
     *
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] getChargeLevelPacket();

    /**
     * Update dice detection settings
     *
     * @param samplesCount
     * @param movementCount
     * @param faceCount
     * @param minFlatDeg
     * @param maxFlatDeg
     * @param weakStable
     * @param movementDeg
     * @param rollThreshold
     * @return packet value that should be written to dice `write characteristic`
     */
    public static native byte[] detectionSettingsUpdatePacket(int samplesCount,
                                                              int movementCount,
                                                              int faceCount,
                                                              int minFlatDeg,
                                                              int maxFlatDeg,
                                                              int weakStable,
                                                              int movementDeg,
                                                              int rollThreshold);
}
