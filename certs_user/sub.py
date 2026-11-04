#!/usr/bin/env python3
"""Subscribe to AWS IoT command topic and print complete received messages.
Usage:
    python sub.py
    python sub.py bems/green-building-1/cmd
"""
import os
import sys
import time
import ssl

# Ensure immediate unbuffered console output
sys.stdout.reconfigure(line_buffering=True)

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("Error: paho-mqtt not installed. Run: pip install paho-mqtt", flush=True)
    sys.exit(1)

ENDPOINT = "a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com"
PORT = 8883
THING = "green-building-1"
TOPIC = "bems/green-building-1/cmd"

# Locate certificate files
script_dir = os.path.dirname(os.path.abspath(__file__))
ca_path = os.path.join(script_dir, "AmazonRootCA1.pem")
cert_path = os.path.join(script_dir, "Device-certificate.pem.crt")
key_path = os.path.join(script_dir, "Device-private.pem.key")

for f in (ca_path, cert_path, key_path):
    if not os.path.exists(f):
        print("Error: File not found: %s" % f, flush=True)
        sys.exit(1)

def on_connect(client, userdata, flags, rc, props=None):
    if hasattr(rc, "is_failure") and rc.is_failure:
        print("[FAIL] Connection failed: %s" % rc, flush=True)
        return
    print("\n[OK] Connected to AWS IoT (%s)" % ENDPOINT, flush=True)
    print("[OK] Client ID: %s" % THING, flush=True)
    topic = userdata.get("topic", TOPIC)
    client.subscribe(topic, qos=1)
    print("[OK] Subscribed to topic: %s" % topic, flush=True)
    print("[*] Waiting for incoming messages from AWS IoT...\n" + "="*50, flush=True)

def on_message(client, userdata, msg):
    try:
        payload_str = msg.payload.decode("utf-8")
    except UnicodeDecodeError:
        payload_str = str(msg.payload)
    print("\n>>> [MESSAGE RECEIVED FULL PAYLOAD] <<<", flush=True)
    print("Topic  : %s" % msg.topic, flush=True)
    print("Payload: %s" % payload_str, flush=True)
    print("=" * 50, flush=True)

def on_disconnect(client, userdata, flags, rc, props=None):
    print("\n[INFO] Disconnected from AWS IoT (Code: %s)." % rc, flush=True)
    print("Note: If AWS disconnected, check that ESP32 is turned off (Client ID '%s' allows only 1 connection)." % THING, flush=True)

def main():
    topic = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("-") else TOPIC
    print("Connecting to AWS IoT Core...", flush=True)
    print("Endpoint: %s" % ENDPOINT, flush=True)
    print("Topic   : %s" % topic, flush=True)

    userdata = {"topic": topic}
    if hasattr(mqtt, "CallbackAPIVersion"):
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=THING, userdata=userdata)
    else:
        client = mqtt.Client(client_id=THING, userdata=userdata)

    client.tls_set(ca_certs=ca_path, certfile=cert_path, keyfile=key_path)
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect

    try:
        client.connect(ENDPOINT, PORT, keepalive=60)
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nStopping subscriber...", flush=True)
        client.disconnect()

if __name__ == "__main__":
    main()
