# Tap2iD SDK for iOS

Swift Package distribution of `Tap2iDVerifierSDK.xcframework`.

**Current release: `2.3.0`**

## Overview

The Tap2iD SDK complies with the ISO/IEC 18013-5:2021 standard, facilitating digital representation for mobile-based credentials, including mobile driver's licenses (mDL). It also verifies the PDF417 barcode on the back of a physical driver's licence, and reads IDs in Apple Wallet with Tap to Present ID.

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
   - Choose **Exact Version** and enter `2.3.0`.
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

## Tap to Present ID

From **2.3.0** the SDK can read a driver's licence from Apple Wallet with Apple's Tap to
Present ID (the ProximityReader framework). The holder taps their phone on
the verifier's iPhone and approves the request; no QR code or NFC engagement is needed.

There are two kinds of request:

| Request | What happens | Returned to the app |
|---|---|---|
| `.display(document:elements:)` | iOS shows the holder's details; nothing is passed to your app | Only the outcome — approved, rejected or dismissed |
| `.data` | The attributes in your Verify with Credence profile are read and verified like `verifyMdoc` | A `VerificationResult` |

### Prerequisites

| | Display request | Data request |
|---|---|---|
| Apple entitlement | `identity.display` | `identity.read` |
| Brand ID and Key ID in Verify with Credence | — | Required |
| Profile attributes listed in the entitlement | — | Required |

### Step 1 — Get the entitlements from Apple

Tap to Present ID entitlements are granted by Apple. Request them for your app through your
Apple Developer account: the display entitlement for display requests, and the read
entitlement — with every document element your profile needs (see the
[mapping table](#profile-attributes-and-entitlement-elements)) — for data requests.

### Step 2 — Add the entitlements to your app

1. In Xcode, select your app target → **Signing & Capabilities**. If the target has no
   `.entitlements` file yet, add any capability (for example **Near Field Communication
   Tag Reading**) and Xcode creates one.
2. Open the `.entitlements` file as source code and add the keys Apple granted:

   ```xml
   <!-- Display requests -->
   <key>com.apple.developer.proximity-reader.identity.display</key>
   <true/>

   <!-- Data requests -->
   <key>com.apple.developer.proximity-reader.identity.read</key>
   <dict>
       <key>document-types</key>
       <array>
           <string>drivers-license</string>
       </array>
       <key>document-elements</key>
       <array>
           <string>given-name</string>
           <string>family-name</string>
           <string>date-of-birth</string>
           <string>portrait</string>
           <!-- every element your profile requests -->
       </array>
   </dict>
   ```

3. Build to a device. Add only what Apple granted: a key or element that is not in your
   provisioning profile stops the app from signing.

### Step 3 — Register with Apple in Verify with Credence (data requests)

Add your Apple **Brand ID** and **Key ID** to your account in Verify with Credence. For
every data request the SDK fetches a signed reader token from Verify with Credence, so
your app holds no key material. Without this, data requests fail with error **136**.

### Step 4 — Match your profile to the entitlement (data requests)

The SDK requests the `org.iso.18013.5.1.mDL` attributes in your verification profile.
Each one must appear in `document-elements`. Before contacting Apple, the SDK compares the
two and fails with error **134**, naming the attributes the entitlement does not allow.
Attributes Apple has no element for are skipped and reported as not shared.

#### Profile attributes and entitlement elements

| Profile attribute | `document-elements` entry |
|---|---|
| `given_name` | `given-name` |
| `family_name` | `family-name` |
| `birth_date` | `date-of-birth` |
| `age_in_years`, `age_over_NN` | `age` |
| `portrait` | `portrait` |
| `sex` | `sex` |
| `resident_address`, `resident_city`, `resident_state`, `resident_postal_code`, `resident_country` | `address` |
| `height` | `height` |
| `weight` | `weight` |
| `eye_colour` | `eye-color` |
| `hair_colour` | `hair-color` |
| `signature_usual_mark` | `signature-usual-mark` |
| `nationality` | `nationality` |
| `birth_place` | `place-of-birth` |
| `issuing_authority`, `issuing_jurisdiction`, `issuing_country` | `issuing-authority` |
| `driving_privileges` | `driving-privileges` |
| `document_number` | `document-number` |
| `issue_date` | `document-issue-date` |
| `expiry_date` | `document-expiration-date` |
| `organ_donor` (AAMVA) | `organ-donor-status` |
| `veteran` (AAMVA) | `veteran-status` |
| `DHS_compliance` (AAMVA) | `document-dhs-compliance-status` |
| `domestic_driving_privileges` (AAMVA) | `driving-privileges` |

### Checking support

```swift
if Tap2iDVerifySDK.shared.isProximityReaderSupported() {
    // show the Tap to Present ID option
}
```

`isProximityReaderSupported()` returns whether this device can act as a Tap to Present ID
reader. You do not need to import `ProximityReader` to call it.

### Display request

```swift
let request = ProximityReaderRequest.display(
    document: .driversLicense,
    elements: [.givenName, .familyName, .ageAtLeast(21)]
)
if let error = Tap2iDVerifySDK.shared.verifyMdocWithEntitlement(request: request, delegate: self) {
    // .sdkNotInitialized — initSdk has not completed
}
```

Apple allows four display elements: `givenName`, `familyName`, `age` and `ageAtLeast(_:)`.
`document` can also be `.nationalIDCard(region:)` (iOS 18 and later) or `.photoID`
(iOS 26 and later).

### Data request

```swift
if let error = Tap2iDVerifySDK.shared.verifyMdocWithEntitlement(request: .data, delegate: self) {
    // .sdkNotInitialized — initSdk has not completed
}
```

You do not pass elements — they come from your verification profile. The response is
verified like `verifyMdoc`: issuer and device signatures, digests, MSO validity and IACA
trust. Elements Apple added after the running iOS version (17.2, 26 and 26.4) are skipped
on older devices.

### ProximityReaderDelegate

All callbacks are delivered on the main thread.

```swift
extension TestSDK: ProximityReaderDelegate {

    func onProximityReaderStageStarted(stage: ProximityReaderStage) { }

    func onProximityReaderStageCompleted(stage: ProximityReaderStage) { }

    func onProximityReaderError(stage: ProximityReaderStage?, error: CoreCredenceErrorStruct) {
        // error.combinedCode, error.errorMessage
    }

    // Display requests only.
    func onDisplayRequestCompleted(outcome: ProximityDisplayOutcome) { }

    // Data requests only.
    func onVerificationCompleted(verificationResult: VerificationResult) { }
}
```

Stages run in order: `PREPARE_READER`, `REQUEST_DOCUMENT`, then `VALIDATE_RESPONSE` for a
data request. Transactions are logged to Verify with Credence like any other engagement.
A request the holder cancels is not logged.

### Errors

Tap to Present ID errors arrive on `onProximityReaderError` with a Device Engagement code.
The data-request setup errors are:

| Code | Meaning | What to check |
|---|---|---|
| 132 | Reader token could not be obtained | Network connection and the account's Tap to Present ID setup |
| 133 | The profile has no driver's licence attribute Tap to Present ID can request | The profile's `org.iso.18013.5.1.mDL` attributes |
| 134 | The profile requests attributes the entitlement does not allow | The message names them; add them to `document-elements` or remove them from the profile |
| 135 | The app has no Tap to Present ID data entitlement | `com.apple.developer.proximity-reader.identity.read` |
| 136 | The account is not registered with Apple for Tap to Present ID | Brand ID and Key ID in Verify with Credence |

Errors from Apple's framework (a cancelled request, an expired session, an unsupported
device) use codes 120–131; see the
[Guide for Credential Verification Errors](https://github.com/CredenceID/Tap2iD-SDK-iOS/wiki/Guide-for-Credential-Verification-Errors).

## Upgrading from 2.2.0

No source changes are needed.

- **Tap to Present ID** is new — see [Tap to Present ID](#tap-to-present-id).
- **PDF417 transactions are logged with the `QR_CODE` engagement.** They were logged as
  `BARCODE_SCAN`, which Verify with Credence showed as "Not Applicable".
- **PDF417 age checks.** When a profile lists `age_over_NN` only on its mDL request, a
  PDF417 scan now returns that `age_over_NN` in `fields`. Previously a barcode scan never
  produced an age result for such a profile.

## Upgrading from 2.0.1

### Date elements are returned as text

Date elements — `birth_date`, `expiry_date`, `issue_date` and the rest — now come back as
the string the wallet sent, rather than as a `Date`:

```swift
// 2.0.1
let dob = attributes["birth_date"] as? Date   // 1971-09-01 00:00:00 +0000

// 2.2.0
let dob = attributes["birth_date"] as? String // "1971-09-01"
```

A full-date carries no time, so returning a `Date` forced it onto midnight UTC and every
consumer rendered the trailing `00:00:00 +0000`. A tdate, conversely, would have lost its
time if it were reformatted to a day. The original text is faithful to both:
`"1971-09-01"` for a full-date, `"2020-05-04T10:30:00Z"` for a tdate.

### An element that nests its value under its own name is now read

Some wallets send `{"birth_date": 1004("1971-09-01")}` where the bare tagged date belongs.
The issuer signs that shape, so it arrives digest-valid and previously failed to parse. The
nested value is now unwrapped and read, and a warning is logged when that happens.

### Abandoned transactions report a connection timeout

A transaction cancelled or abandoned before it completes now delivers
`DataRetrievalError.connectionTimeout` through `onVerificationStageError`, and writes an
error transaction log. Previously a host app calling `stopMonitoring()` directly could end
the transaction with no error reported at all.

### `CoreCredenceErrorStruct.combinedCode` is now public

It was internal, so host apps could not read the combined error code off the struct they
were handed. No source change is needed — this only widens access.

The eval-mode watermark now reads `CREDENCE ID`; it was misspelled `CREDNCE ID`.

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
- [Guide for Credential Verification Errors](https://github.com/CredenceID/Tap2iD-SDK-iOS/wiki/Guide-for-Credential-Verification-Errors)
- [Sample App](https://github.com/CredenceID/Tap2iD-SDK-iOS)

---

© 2026 Credence ID, LLC. All rights reserved.
