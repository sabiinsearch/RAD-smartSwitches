# RAD-smartSwitches

Project to collect Current & Voltage data from smart switches and communicate with AWS IoT Core.

## Terminal command to Erase ESP32
```cmd
pio run --target erase
```

---

## MQTT CLI Commands (Windows Command Prompt)

Open Command Prompt and navigate to the `certs_user` directory:
```cmd
cd C:\myData\software_projects\RAD\RAD-smartSwitches\certs_user
```

> **Note:** Ensure the ESP32 is powered off while running these commands, as AWS IoT Core allows only one active connection per Client ID (`green-building-1`).

### 1. Subscribe to Command Topic (Receive Commands from Cloud)
Subscribes to `bems/green-building-1/cmd` with QoS 1:
```cmd
mosquitto_sub -d -h a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert Device-certificate.pem.crt --key Device-private.pem.key -i green-building-1 -t bems/green-building-1/cmd -q 1 -v
```
*(To test message reception, publish a test message like `{"message":"1#"}` to `bems/green-building-1/cmd` from the AWS IoT Core Console > MQTT test client).*

### 2. Publish to Telemetry Topic (Send Data to Cloud)
Publishes test JSON telemetry to `bems/green-building-1/tel`:
```cmd
mosquitto_pub -d -h a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert Device-certificate.pem.crt --key Device-private.pem.key -i green-building-1 -t bems/green-building-1/tel -m "{\"status\":\"active\",\"data\":\"test\"}" -q 1
```

### 3. Publish to Status Topic (Online Status)
Publishes online status to `bems/green-building-1/status`:
```cmd
mosquitto_pub -d -h a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert Device-certificate.pem.crt --key Device-private.pem.key -i green-building-1 -t bems/green-building-1/status -m "online" -q 1
```

### 4. Automated Python Test Script
Runs all verification checks (DNS, port 8883, TLS 1.3 handshake, connect, subscribe, publish, and optional command listening):
```cmd
python test_aws_iot.py --listen 30
```
