# Tap2iD SDK for iOS

Swift Package distribution of `Tap2iDVerifierSDK.xcframework`.

**Current release: `2.0.1`**

## Overview

The Tap2iD SDK complies with the ISO/IEC 18013-5:2021 standard, facilitating digital representation for mobile-based credentials, including mobile driver's licenses (mDL). It also verifies the PDF417 barcode on the back of a physical driver's licence.

## System Requirements

- **Supported iOS Versions**: iOS 17.0 and later
- **Dependencies**: Xcode 15.0 or later
- **Hardware Requirements**: iPhone 11 or newer with NFC enabled
- **Build machine**: simulator builds require an **Apple Silicon** Mac (see [Simulator architecture](#simulator-architecture))

## Installation

### Swift Package Manager

1. **Open Xcode**: Launch your project in Xcode.
2. **Add Package Dependency**:
   - Go to `File > Add Package Dependencies…`.
   - Enter `https://github.com/CredenceID/Tap2iD-VerifierSDK-iOS.git`.
3. **Specify Version**:
   - Choose **Exact Version** and enter `2.0.1`.
   - Pin exactly rather than by range, so the binary your app builds against is reproducible.
4. **Add to Target**:
   - Select the **`Tap2iD-VerifyerSDK-Swift`** library product and add it to your app target.
   - Import it in code as `import Tap2iDVerifierSDK`.

For troubleshooting, refer to Apple's [Adding Package Dependencies to Your App](https://developer.apple.com/library/archive/documentation/Xcode/Adding_Package_Dependencies_to_Your_App/) guide.

### Simulator architecture

From **2.0.1** the simulator slice is **arm64 only**.

| Mac | Device builds | Simulator builds |
|---|---|---|
| Apple Silicon | supported | supported |
| Intel | supported | **not supported** |

On an Intel Mac a simulator build fails to link. Build to a physical device, or use an Apple Silicon Mac. SDK 2.0.0 and earlier shipped a universal simulator slice, so this is a change if you are upgrading.

## Enabling BLE and NFC Support

### Bluetooth Support

1. **Open `Info.plist`**
2. **Add Bluetooth Usage Description Keys**:

   ```xml
   <key>NSBluetoothAlwaysUsageDescription</key>
   <string>Your app requires Bluetooth access to connect to nearby devices even in the background.</string>

   <key>NSBluetoothPeripheralUsageDescription</key>
   <string>Your app needs to advertise and communicate with other Bluetooth devices.</string>

   <key>NSBluetoothWhenInUseUsageDescription</key>
   <string>Your app requires Bluetooth access to connect to nearby devices while in use.</string>
   ```

### NFC Support

#### Enable NFC Capability

1. Select your project in the Project Navigator.
2. Select your app target.
3. In **Signing & Capabilities**, click **+** and add **Near Field Communication Tag Reading**.

#### Modify `Info.plist`

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

```swift
import Tap2iDVerifierSDK

class TestSDK {

    // Initialise the SDK. The callback receives a LicenseKeyVerificationResult
    // carrying isValid, expiryDate, profileName and error.
    func initSDK(apiKey: String, result: @escaping (LicenseKeyVerificationResult) -> Void) {
        let sdkConfig = CoreSdkConfig(apiKey: apiKey)
        Tap2iDVerifySDK.shared.initSdk(config: sdkConfig) { licenseResult in
            result(licenseResult)
        }
    }

    // QR code engagement. Progress and the result arrive on the delegate;
    // the return value is non-nil only if verification could not be started.
    func startQrEngagement(capturedQr: String, result: @escaping (Error?) -> Void) {
        if let error = Tap2iDVerifySDK.shared.verifyMdoc(
            engagementConfig: .qrCode(capturedQr),
            delegate: self
        ) {
            result(error)
        }
    }

    // Native NFC engagement.
    func startNativeNfcEngagement(result: @escaping (Error?) -> Void) {
        if let error = Tap2iDVerifySDK.shared.verifyMdoc(
            engagementConfig: .nfc,
            delegate: self
        ) {
            result(error)
        }
    }

    // External NFC reader engagement.
    func startExternalNfcReaderEngagement(result: @escaping (Error?) -> Void) {
        if let error = Tap2iDVerifySDK.shared.verifyMdoc(
            engagementConfig: .nfcExternalReader,
            delegate: self,
            readerDelegate: self
        ) {
            result(error)
        }
    }

    // PDF417 barcode from the back of a driver's licence. This path uses the
    // dedicated classifier API: it is async, returns a typed result directly,
    // and emits no VerificationStage callbacks.
    func startPdf417Verification(
        barcode: String,
        completion: @escaping (Result<Pdf417VerificationResult, Error>) -> Void
    ) {
        Task {
            do {
                let request = Pdf417VerificationRequest(pdf417Value: barcode)
                let result  = try await Tap2iDVerifySDK.shared.verifyPdf417(request: request)
                await MainActor.run { completion(.success(result)) }
            } catch {
                await MainActor.run { completion(.failure(error)) }
            }
        }
    }

    // Re-fetch the reader profile at runtime, without restarting the app.
    func refreshConfiguration(completion: @escaping (ConfigurationRefreshResult) -> Void) {
        // Not guaranteed to be called on the main thread.
        Tap2iDVerifySDK.shared.refreshConfiguration(completion: completion)
    }
}

// MARK: - Tap2iDVerifySDKDelegate

extension TestSDK: Tap2iDVerifySDKDelegate {

    func onVerificationStageStarted(stage: VerificationStage) {
        // Logging, UI updates, etc.
    }

    func onVerificationStageError(stage: VerificationStage?, error: CoreCredenceErrorStruct?) {
        // Error handling.
    }

    func onVerificationStageCompleted(stage: VerificationStage) {
        // UI updates, etc.
    }

    func onVerificationCompleted(verificationResult: VerificationResult?) {
        guard let result = verificationResult else { return }
        // result.status, result.documents
    }
}

// MARK: - NfcExternalReaderDelegate

extension TestSDK: NfcExternalReaderDelegate {

    func didDetectReaders() { }

    func didDisconnectFromReader() { }

    func didDetectSmartCard() { }

    func didDisconnectFromSmartCard() { }
}
```

### Reading a PDF417 result

```swift
print(result.verdict.rawValue)      // authentic | likelyAuthentic | likelyFraudulent | error
print(result.confidenceLevel)
print(result.stateCode ?? "-")

let crypto = result.cryptoVerification
if crypto.checked {
    print("signature (\(crypto.authority)): \(crypto.verified ? "valid" : "invalid")")
} else {
    print("no verifiable signature on this barcode")
}
```

`checked == false` means the barcode carried no verifiable signature — which is not the same as failing one.

Disclosed fields require an `aamva.pdf417` entry in your Verify with Credence profile. Without one you still receive a verdict and confidence score, but no fields.

## Upgrading from 2.0.0

`EngagementConfig.pdf417` has been removed. Replace:

```swift
// 2.0.0
verifyMdoc(engagementConfig: .pdf417(value), delegate: self)

// 2.0.1
let result = try await Tap2iDVerifySDK.shared.verifyPdf417(
    request: Pdf417VerificationRequest(pdf417Value: value)
)
```

The result type changes from `VerificationResult` to `Pdf417VerificationResult`, and results arrive by return value rather than through the delegate.

See also the [simulator architecture](#simulator-architecture) change above.

## Documentation

- [Release Notes](https://github.com/CredenceID/Tap2iD-SDK-iOS/releases)
- [API Documentation](https://github.com/CredenceID/Tap2iD-SDK-iOS/wiki/Tap2iD-SDK-API-Documentation)
- [Integration Guide](https://github.com/CredenceID/Tap2iD-SDK-iOS/wiki/Guide-to-Integrate-Tap2iD-iOS-SDK)
- [Sample App](https://github.com/CredenceID/Tap2iD-SDK-iOS)

---

© 2026 Credence ID, LLC. All rights reserved.
