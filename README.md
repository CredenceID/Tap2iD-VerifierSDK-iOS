# Tap2iD SDK for iOS

## Overview

The Tap2iD SDK complies with the ISO/IEC 18013-5:2021 standard, facilitating digital representation for mobile-based credentials, including mobile driver's licenses (mDL). 

## System Requirements

- **Supported iOS Versions**: iOS 17.0 and later
- **Dependencies**: Xcode 15.0 or later
- **Hardware Requirements**: iPhone 11 or newer with NFC enabled

## Installation

### Swift Package Manager

To add the Tap2iD SDK to your Xcode project using Swift Package Manager:

1. **Open Xcode**: Launch your project in Xcode.
2. **Add Package Dependency**:
   - Go to `File > Swift Packages > Add Package Dependency`.
   - Enter `https://github.com/CredenceID/Tap2iD-VerifierSDK-Swift.git` in the repository URL field.
3. **Specify Version**:
   - Choose "Up to Next Major" and specify `X.X.X` as the earliest version.
4. **Add to Target**:
   - Select "Tap2iD-VerifierSDK-Swift" when Xcode resolves the version and add it to your app target.

For troubleshooting, refer to Apple's [Adding Package Dependencies to Your App](https://developer.apple.com/library/archive/documentation/Xcode/Adding_Package_Dependencies_to_Your_App/) guide.

## Enabling BLE and NFC Support

### Bluetooth Support

1. **Open `Info.plist`**:
   - Locate and open the `Info.plist` file in your Xcode project.

2. **Add Bluetooth Usage Description Keys**:
   Include the following keys to request Bluetooth permissions:

   ```xml
   <key>NSBluetoothAlwaysUsageDescription</key>
   <string>Your app requires Bluetooth access to connect to nearby devices even in the background.</string>
   
   <key>NSBluetoothPeripheralUsageDescription</key>
   <string>Your app needs to advertise and communicate with other Bluetooth devices.</string>
   
   <key>NSBluetoothWhenInUseUsageDescription</key>
   <string>Your app requires Bluetooth access to connect to nearby devices while in use.</string>


# NFC Support

NFC support has been detailed separately to ensure clarity and focus on each individual technology's configuration. Below are the steps to enable NFC support for the Tap2iD Verifier-SDK.

## Enable NFC Capability

1. **Select Your Project**:
   - Open Xcode and select your project in the Project Navigator.

2. **Choose Your Target**:
   - In the Targets list, select your app target.

3. **Add NFC Capability**:
   - Go to the "Signing & Capabilities" tab.
   - Click the "+" button and choose "Near Field Communication Tag Reading" from the list of available capabilities.

## Modify `Info.plist`

To request NFC permissions, you need to update your `Info.plist` file. Add the following entries to explain why your app needs NFC access:

```xml
<key>NFCReaderUsageDescription</key>
<string>YOUR_PRIVACY_DESCRIPTION</string>

<key>com.apple.developer.nfc.readersession.iso7816.select-identifiers</key>
<array>
    <string>D2760000850101</string>
</array>

<key>com.apple.developer.nfc.readersession.felica.systemcodes</key>
<array>
    <string>12FC</string>
</array>

```


## Example Usage

Here is an example of how to use the Tap2iD Verify SDK:

```swift

import Tap2iDVerifierSDK

class TestSDK {

    // Initializes the SDK with the provided API key and returns any error or message via the result closure
    func initSDK(apiKey: String, result: @escaping (String?, String?, String?) -> Void) {
        // Create the configuration object using the provided API key
        let sdkConfig = CoreSdkConfig(apiKey: apiKey)

        // Initialize the SDK with the configuration and handle any error or message returned
        Tap2iDVerifySDK.shared.initSdk(config: sdkConfig) { (error, message, profile) in
            // Execute the result closure with the error or message
            result(error, message, profile)
        }
    }

    // Starts the QR code engagement flow and returns any error via the result closure
    func startQrEngagement(capturedQr: String, result: @escaping (Error?) -> Void) {
        // Begin the verification process using the captured QR code
        let error = Tap2iDVerifySDK.shared.verifyMdoc(engagementConfig: .qrCode(capturedQr), delegate: self)
        
        // If there's an error, pass it back via the result closure
        if let error = error {
            result(error)
        }
    }

    // Starts the NFC engagement flow using native NFC functionality
    func startNativeNfcEngagement(result: @escaping (Error?) -> Void) {
        // Begin the NFC verification process
        let error = Tap2iDVerifySDK.shared.verifyMdoc(engagementConfig: .nfc, delegate: self)
        
        // If there's an error, pass it back via the result closure
        if let error = error {
            result(error)
        }
    }

    // Starts the external NFC reader engagement flow and returns any error via the result closure
    func startExternalNfcReaderEngagement(result: @escaping (Error?) -> Void) {
        // Begin the NFC verification process with an external NFC reader
        let error = Tap2iDVerifySDK.shared.verifyMdoc(engagementConfig: .nfcExternalReader, delegate: self, readerDelegate: self)
        
        // If there's an error, pass it back via the result closure
        if let error = error {
            result(error)
        }
    }
}

// MARK: - Tap2iDVerifySDKDelegate Extension
// This extension implements the Tap2iDVerifySDKDelegate protocol to handle the verification process
extension TestSDK: Tap2iDVerifySDKDelegate {

    // Called when a verification stage starts
    func onVerificationStageStarted(stage: VerificationStage) {
        // Placeholder for handling when the verification stage starts
        // You can add logic for logging, UI updates, etc.
    }

    // Called when an error occurs during a verification stage
    func onVerificationStageError(stage: VerificationStage?, error: CoreCredenceErrorStruct?) {
        // Placeholder for handling verification stage errors
        // You can add error handling logic here
    }

    // Called when a verification stage is completed successfully
    func onVerificationStageCompleted(stage: VerificationStage) {
        // Placeholder for handling when a verification stage completes
        // You can add logic for UI updates, etc.
    }

    // Called when the verification process is completed
    func onVerificationCompleted(result: MdlAttributes, validationResult: [CoreCredenceErrorStruct]) {
        // Placeholder for handling when the verification is completed
        // You can access the result (MdlAttributes) and validationResult (errors) for processing
    }
}

// MARK: - NfcExternalReaderDelegate Extension
// This extension implements the NfcExternalReaderDelegate protocol to handle NFC reader events
extension TestSDK: NfcExternalReaderDelegate {
    
    // Called when NFC readers are detected
    func didDetectReaders() {
        // Placeholder for handling when NFC readers are detected
        // You can add logic for UI updates or reader selection
    }

    // Called when the connection to an NFC reader is disconnected
    func didDisconnectFromReader() {
        // Placeholder for handling when the NFC reader is disconnected
        // You can add logic for UI updates, disconnection handling, etc.
    }

    // Called when a smart card is detected by the external NFC reader
    func didDetectSmartCard() {
        // Placeholder for handling when a smart card is detected
        // You can add logic for processing or UI updates
    }

    // Called when the smart card is disconnected
    func didDisconnectFromSmartCard() {
        // Placeholder for handling when the smart card is disconnected
        // You can add logic for cleanup, UI updates, etc.
    }
}

```
