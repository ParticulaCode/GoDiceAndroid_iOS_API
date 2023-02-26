# iOS SDK

## SDK

To build SDK framework go to `GoDiceSDK` directory and run `xcodebuild` command. This command will produce `GoDiceSDK/build/Release-iphoneos/GoDiceSDK.framework`.

Framework defines `GoDiceSDK` class with methods to process incoming and form outgoing packets. Events recognized from incoming packets are delivered to intance's delegate with `GoDiceSDKDelegate` interface. See doc comments in `GoDiceSDK.h` framework header for reference.

## Demo app

To run demo app open `demoapp/demoapp.xcodeproj` with XCode, setup code sign options, build and deploy it.
While app is running it will connect to any advertising dice, turn on red and green LEDs for 3 seconds, request it's color and charge level. Data received from dice will be logged in app.
