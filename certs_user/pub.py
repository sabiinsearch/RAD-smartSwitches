#!/usr/bin/env python3
"""Publish a command to the ESP32 via AWS IoT MQTT.
Usage:
    python pub.py '{"message":"A#"}'           # JSON command (ESP32 extracts 'message')
    python pub.py "1#"                          # Raw command string
    python pub.py "online" --topic bems/green-building-1/status
"""
import os
import sys
import time
import logging

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("Error: paho-mqtt not installed. Run: pip install paho-mqtt")
    sys.exit(1)

# Enable paho internal logging so we can see TLS / connection errors
logging.basicConfig(level=logging.DEBUG, format="[paho] %(message)s")
logger = logging.getLogger(__name__)

ENDPOINT    = "a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com"
PORT        = 8883
THING       = "green-building-1"
CLIENT_ID   = THING + "-publisher"          # Different from the ESP32 client ID
CMD_TOPIC   = "bems/green-building-1/cmd"  # Topic the ESP32 subscribes to

script_dir  = os.path.dirname(os.path.abspath(__file__))
ca_path     = os.path.join(script_dir, "AmazonRootCA1.pem")
cert_path   = os.path.join(script_dir, "Device-certificate.pem.crt")
key_path    = os.path.join(script_dir, "Device-private.pem.key")

# Verify certificate files exist
for label, path in [("CA cert", ca_path), ("Device cert", cert_path), ("Private key", key_path)]:
    if os.path.exists(path):
        print("[OK] %s found: %s" % (label, path))
    else:
        print("[FAIL] %s NOT FOUND: %s" % (label, path))
        sys.exit(1)

def main():
    payload = sys.argv[1] if len(sys.argv) > 1 else '{"message":"A#"}'
    topic   = CMD_TOPIC
    if "--topic" in sys.argv:
        idx = sys.argv.index("--topic")
        if idx + 1 < len(sys.argv):
            topic = sys.argv[idx + 1]

    print("\nPublishing to AWS IoT...")
    print("Endpoint  : %s" % ENDPOINT)
    print("Topic     : %s" % topic)
    print("Payload   : %s" % payload)
    print("Client ID : %s\n" % CLIENT_ID)

    result_flag = {"published": False, "failed": False}

    def on_connect(client, userdata, flags, rc, props=None):
        # rc is ReasonCode (paho v2) or int (paho v1)
        rc_val = rc.value if hasattr(rc, "value") else int(rc)
        if rc_val != 0:
            print("[FAIL] MQTT CONNACK rejected. Reason code: %s (%s)" % (rc_val, rc))
            result_flag["failed"] = True
            client.disconnect()
            return
        print("[OK] Connected to AWS IoT broker.")
        print("[..] Publishing message...")
        res = client.publish(topic, payload, qos=1)
        try:
            res.wait_for_publish(timeout=10)
            print("[OK] Published! mid=%s is_published=%s" % (res.mid, res.is_published()))
            result_flag["published"] = True
        except Exception as e:
            print("[FAIL] wait_for_publish error: %s" % e)
            result_flag["failed"] = True
        client.disconnect()

    def on_connect_fail(client, userdata):
        # Called by paho v2 when TCP/TLS connection fails before CONNACK
        print("[FAIL] TCP/TLS connection failed (on_connect_fail). Check certificates and policy.")
        result_flag["failed"] = True

    def on_disconnect(client, userdata, flags=None, rc=None, props=None):
        rc_val = rc.value if hasattr(rc, "value") else rc
        print("[INFO] Disconnected. rc=%s" % rc_val)

    def on_log(client, userdata, level, buf):
        # Shows paho's internal TLS / socket messages
        print("[LOG] %s" % buf)

    # Build client — detect paho API version
    if hasattr(mqtt, "CallbackAPIVersion"):
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
    else:
        client = mqtt.Client(client_id=CLIENT_ID)

    client.on_connect      = on_connect
    client.on_disconnect   = on_disconnect
    client.on_log          = on_log
    if hasattr(client, "on_connect_fail"):
        client.on_connect_fail = on_connect_fail

    # Apply TLS certificates
    try:
        client.tls_set(ca_certs=ca_path, certfile=cert_path, keyfile=key_path)
        print("[OK] TLS certificates loaded.")
    except Exception as e:
        print("[FAIL] tls_set() error: %s" % e)
        sys.exit(1)

    # Connect + wait up to 15s for TLS handshake + QoS-1 PUBACK
    try:
        print("[..] Connecting to %s:%d ..." % (ENDPOINT, PORT))
        client.connect(ENDPOINT, PORT, keepalive=30)
        client.loop_start()
        deadline = time.time() + 15
        while not result_flag["published"] and not result_flag["failed"] and time.time() < deadline:
            time.sleep(0.1)
        client.loop_stop()
    except Exception as e:
        print("[FAIL] connect() exception: %s" % e)
        sys.exit(1)

    if result_flag["published"]:
        print("\n=== SUCCESS: Message delivered to AWS IoT ===")
    else:
        print("\n=== FAILED: Message not confirmed ===")
        print("If you see TLS errors above, the certificate may not be active or policy not attached in AWS IoT console.")

if __name__ == "__main__":
    main()
