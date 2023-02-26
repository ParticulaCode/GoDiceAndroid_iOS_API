# Android SDK

## SDK

To build SDK library run `./gradlew :godicesdklib:assembleRelease` command. This command will produce `godicesdklib/build/outputs/aar/godicesdklib-release.aar`.

Library defines `GoDiceSDK` class with methods to process incoming and form outgoing packets. Events recognized from incoming packets are delivered to `GoDiceSDK.listener` supplied by SDK user with `GoDiceSDK.Listener` interface. See doc comments in `godicesdklib/src/main/java/org/sample/godicesdklib/GoDiceSDK.java` file for reference.

## Demo app

To run demo app build it with `./gradlew assembleDebug`, deploy with `adb install app/build/outputs/apk/debug/app-debug.apk` and start `godicesdk` app. Alternatively open this directory as project in Android Studio and run `app` target.

While app is running it will connect to any advertising dice, turn on red and green LEDs for 3 seconds, request it's color and charge level. Data received from dice will be logged in app.
