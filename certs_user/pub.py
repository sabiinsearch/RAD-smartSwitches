#!/usr/bin/env python3
"""Publish to AWS IoT telemetry/status topic.
Usage:
    python pub.py "{\"status\":\"active\",\"data\":\"test\"}"
    python pub.py "online" --topic bems/green-building-1/status
"""
import os
import sys
import time
import ssl

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("Error: paho-mqtt not installed. Run: pip install paho-mqtt")
    sys.exit(1)

ENDPOINT = "a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com"
PORT = 8883
THING = "green-building-1"
DEFAULT_TOPIC = "bems/green-building-1/tel"

script_dir = os.path.dirname(os.path.abspath(__file__))
ca_path = os.path.join(script_dir, "AmazonRootCA1.pem")
cert_path = os.path.join(script_dir, "Device-certificate.pem.crt")
key_path = os.path.join(script_dir, "Device-private.pem.key")

def main():
    payload = sys.argv[1] if len(sys.argv) > 1 else '{"status":"active","message":"hello_from_pub"}'
    topic = DEFAULT_TOPIC
    if "--topic" in sys.argv:
        idx = sys.argv.index("--topic")
        if idx + 1 < len(sys.argv):
            topic = sys.argv[idx + 1]

    print("Publishing to AWS IoT...")
    print("Endpoint: %s" % ENDPOINT)
    print("Topic   : %s" % topic)
    print("Payload : %s" % payload)

    published = False

    def on_connect(client, userdata, flags, rc, props=None):
        nonlocal published
        if hasattr(rc, "is_failure") and rc.is_failure:
            print("[FAIL] Connection failed: %s" % rc)
            return
        print("[OK] Connected to AWS IoT.")
        res = client.publish(topic, payload, qos=1)
        res.wait_for_publish(timeout=5)
        print("[OK] Message published successfully (mid=%s)" % res.mid)
        published = True
        client.disconnect()

    if hasattr(mqtt, "CallbackAPIVersion"):
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=THING)
    else:
        client = mqtt.Client(client_id=THING)

    client.tls_set(ca_certs=ca_path, certfile=cert_path, keyfile=key_path)
    client.on_connect = on_connect

    try:
        client.connect(ENDPOINT, PORT, keepalive=30)
        client.loop_start()
        time.sleep(2)
        client.loop_stop()
    except Exception as e:
        print("[FAIL] Error: %s" % e)

    if not published:
        print("[FAIL] Could not confirm publication.")

if __name__ == "__main__":
    main()
