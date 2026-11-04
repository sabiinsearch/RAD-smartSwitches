# RAD-smartSwitches

Project to collect Current & Voltage data from smart switches and communicate with AWS IoT Core.

## Terminal command to Erase ESP32
```cmd
pio run --target erase
```

---

## ⚠️ Important Note on AWS IoT Client ID Conflict

AWS IoT Core allows **only ONE active connection** at a time for Client ID `green-building-1`.
- If your **ESP32 is turned ON** and you run `mosquitto_sub` or CLI scripts on your PC using `green-building-1`, AWS will repeatedly disconnect the CLI and the ESP32 (`Error: Connection lost`).
- **To test from PC CLI:** Power OFF / unplug the ESP32 board while testing from Command Prompt.
- **To test while ESP32 is running:** Use the **AWS IoT Core Web Console -> MQTT test client** to subscribe and publish, so you do not disconnect the physical device.

---

## Testing via Python Helper Scripts (Easiest & Detailed)

Navigate to the `certs_user` folder in Command Prompt:
```cmd
cd C:\myData\software_projects\RAD\RAD-smartSwitches\certs_user
```

### 1. Subscribe and display entire message payload:
```cmd
python sub.py
```
*(Prints clear status when connected, and prints the full unabridged message whenever one arrives)*

### 2. Publish telemetry to AWS IoT:
```cmd
python pub.py "{\"status\":\"active\",\"data\":\"test\"}"
```

---

## Testing via Mosquitto CLI

Open Command Prompt and navigate to `certs_user`:
```cmd
cd C:\myData\software_projects\RAD\RAD-smartSwitches\certs_user
```

### 1. Subscribe to Command Topic (bems/green-building-1/cmd)
```cmd
mosquitto_sub -v -h a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert Device-certificate.pem.crt --key Device-private.pem.key -i green-building-1 -t bems/green-building-1/cmd -q 1
```
*Note: With `-v`, the prompt will stay open and display `bems/green-building-1/cmd <full message>` as soon as a message is published from AWS IoT Core test client.*

### 2. Publish to Telemetry Topic (bems/green-building-1/tel)
```cmd
mosquitto_pub -h a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert Device-certificate.pem.crt --key Device-private.pem.key -i green-building-1 -t bems/green-building-1/tel -m "{\"status\":\"active\",\"data\":\"test\"}" -q 1
```

### 3. Automated Connection & Verification Script
```cmd
python test_aws_iot.py --listen 30
```
