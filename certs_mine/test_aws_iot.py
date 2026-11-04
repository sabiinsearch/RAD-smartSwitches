#!/usr/bin/env python3
"""Test the connection to the Green Building BEMS AWS IoT endpoint.

Setup (Python 3.8 or newer):
    pip install "paho-mqtt>=2.0"

Run from the folder that holds these three files:
    *certificate.pem.crt   the device certificate
    *private.pem.key       the device private key
    AmazonRootCA1.pem      the Amazon root certificate

    python test_aws_iot.py

Or give the paths directly:
    python test_aws_iot.py --cert PATH --key PATH --ca PATH

The test uses the device's client ID, so power off the ESP32 while it runs.
Otherwise AWS disconnects whichever of the two connected first.

To test receiving commands, publish any text to the cmd topic from
AWS IoT Core > MQTT test client while this script is listening.
Keep relay loads disconnected if a controller is attached to this link.

Exit code: 0 if all required checks pass, 1 otherwise.
"""
import argparse
import glob
import os
import socket
import ssl
import sys
import threading
import time

try:
    import paho.mqtt.client as mqtt
    NEW_ENOUGH = hasattr(mqtt, "CallbackAPIVersion")
except ImportError:
    mqtt = None
    NEW_ENOUGH = False

ENDPOINT = "a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com"
THING = "green-building-1"
PORT = 8883
TIMEOUT = 15


def ok(message):
    print("PASS  " + message)


def info(message):
    print("INFO  " + message)


def fail(message, hint=""):
    print("FAIL  " + message)
    if hint:
        print("      What to check: " + hint)
    sys.exit(1)


def find_file(explicit, pattern, name):
    if explicit:
        if not os.path.isfile(explicit):
            fail("%s not found: %s" % (name, explicit))
        return explicit
    script_dir = os.path.dirname(os.path.abspath(__file__))
    for folder in (os.getcwd(), script_dir):
        matches = glob.glob(os.path.join(glob.escape(folder), pattern))
        if len(matches) == 1:
            return matches[0]
        if len(matches) > 1:
            fail("More than one %s in %s" % (name, folder),
                 "Choose one with --cert, --key or --ca.")
    fail("%s not found (looking for %s)" % (name, pattern),
         "Run this from the folder holding the files, or give the path with --cert, --key or --ca.")


def check_network(endpoint):
    try:
        addresses = socket.getaddrinfo(endpoint, PORT, type=socket.SOCK_STREAM)
    except socket.gaierror as err:
        fail("Could not find the server %s (%s)" % (endpoint, err),
             "The endpoint spelling, DNS and internet access. It should be " + ENDPOINT)
    ok("Server name found (%d address(es))" % len(addresses))

    try:
        socket.create_connection((endpoint, PORT), timeout=TIMEOUT).close()
    except OSError as err:
        fail("Could not reach port %d (%s)" % (PORT, err),
             "A firewall or network may be blocking outbound port %d. Try a phone hotspot." % PORT)
    ok("Port %d is reachable" % PORT)


def check_tls(endpoint, ca, cert, key):
    try:
        context = ssl.create_default_context(cafile=ca)
        context.load_cert_chain(cert, key)
    except (ssl.SSLError, OSError) as err:
        fail("Could not load the certificate files (%s)" % err,
             "That the certificate and private key are a matching pair, and that the files are unedited.")
    ok("Certificate files loaded, and the certificate and key match")

    try:
        with socket.create_connection((endpoint, PORT), timeout=TIMEOUT) as raw:
            with context.wrap_socket(raw, server_hostname=endpoint) as tls:
                ok("Secure connection made, server trusted (%s)" % tls.version())
    except ssl.SSLCertVerificationError as err:
        fail("Could not verify the server (%s)" % err.verify_message,
             "That --ca is AmazonRootCA1.pem, and that this computer's clock is correct.")
    except (ssl.SSLError, OSError) as err:
        fail("Secure connection failed (%s)" % err,
             "Network filtering or a proxy may be interfering with port %d." % PORT)


def check_mqtt(endpoint, thing, ca, cert, key, listen):
    topics = {name: "bems/%s/%s" % (thing, name) for name in ("cmd", "tel", "status")}
    connected, subscribed, closed = threading.Event(), threading.Event(), threading.Event()
    published = {}
    state = {"connect_code": None, "granted": None, "received": 0}

    def on_connect(client, userdata, flags, reason_code, properties):
        state["connect_code"] = reason_code
        connected.set()

    def on_subscribe(client, userdata, mid, reason_codes, properties):
        state["granted"] = reason_codes[0]
        subscribed.set()

    def on_publish(client, userdata, mid, reason_code, properties):
        published.setdefault(mid, threading.Event()).set()

    def on_message(client, userdata, message):
        state["received"] += 1
        text = message.payload.decode("utf-8", "replace")
        info("Received on %s: %s" % (message.topic, text))

    def on_disconnect(client, userdata, flags, reason_code, properties):
        closed.set()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=thing, protocol=mqtt.MQTTv311, clean_session=True)
    client.tls_set(ca_certs=ca, certfile=cert, keyfile=key)
    client.on_connect = on_connect
    client.on_subscribe = on_subscribe
    client.on_publish = on_publish
    client.on_message = on_message
    client.on_disconnect = on_disconnect
    # A dropped connection is reported as a failure, not retried.
    client.reconnect_delay_set(min_delay=3600, max_delay=3600)

    denied_hint = ("That the certificate is Active, belongs to this AWS account and region (ap-south-1), "
                   "has the bems-device policy attached, and that no other device is using client ID %s." % thing)

    def wait(event, what):
        deadline = time.time() + TIMEOUT
        while time.time() < deadline:
            if event.wait(0.1):
                return
            if closed.is_set():
                fail("AWS closed the connection while %s" % what, denied_hint)
        fail("Timed out while %s" % what, denied_hint)

    client.connect_async(endpoint, PORT, keepalive=30)
    client.loop_start()
    try:
        wait(connected, "connecting")
        if state["connect_code"].is_failure:
            fail("AWS refused the connection (%s)" % state["connect_code"], denied_hint)
        ok("Connected to AWS IoT as %s" % thing)

        client.subscribe(topics["cmd"], qos=1)
        wait(subscribed, "subscribing to " + topics["cmd"])
        if state["granted"].is_failure:
            fail("Subscribe denied for " + topics["cmd"], denied_hint)
        ok("Subscribed to " + topics["cmd"])

        for name, text in (("tel", "connectivity-test"), ("status", "online")):
            result = client.publish(topics[name], text, qos=1)
            wait(published.setdefault(result.mid, threading.Event()), "publishing to " + topics[name])
            ok("Published to %s, AWS acknowledged it" % topics[name])

        if listen > 0:
            info("Listening for %d seconds. Publish text to %s from the AWS MQTT test client now." % (listen, topics["cmd"]))
            deadline = time.time() + listen
            while time.time() < deadline:
                if closed.is_set():
                    fail("AWS closed the connection while listening", denied_hint)
                time.sleep(0.2)
            if state["received"]:
                ok("Received %d message(s) on %s" % (state["received"], topics["cmd"]))
            else:
                info("No message arrived on %s, so receiving was not confirmed" % topics["cmd"])
    finally:
        client.on_disconnect = None
        client.disconnect()
        client.loop_stop()


def main():
    parser = argparse.ArgumentParser(description="Test the connection to AWS IoT for the BEMS device.")
    parser.add_argument("--endpoint", default=ENDPOINT, help="AWS IoT endpoint")
    parser.add_argument("--thing", default=THING, help="client ID and topic name")
    parser.add_argument("--cert", help="device certificate file")
    parser.add_argument("--key", help="device private key file")
    parser.add_argument("--ca", help="AmazonRootCA1.pem file")
    parser.add_argument("--listen", type=int, default=30, help="seconds to wait for a command (0 to skip)")
    args = parser.parse_args()

    if not NEW_ENOUGH:
        fail("paho-mqtt 2.0 or newer is required", 'Run: pip install "paho-mqtt>=2.0"')

    cert = find_file(args.cert, "*certificate.pem.crt", "Device certificate")
    key = find_file(args.key, "*private.pem.key", "Private key")
    ca = find_file(args.ca, "AmazonRootCA1.pem", "AmazonRootCA1.pem")

    print("Endpoint: %s" % args.endpoint)
    print("Client ID: %s\n" % args.thing)
    check_network(args.endpoint)
    check_tls(args.endpoint, ca, cert, key)
    check_mqtt(args.endpoint, args.thing, ca, cert, key, args.listen)
    print("\nRESULT: Connection test passed.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped.")
        sys.exit(1)
