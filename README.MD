Project to collect GPS and Battery Current Voltage data and push to cloud

Terminal command to Erase ESP32 "pio run --target erase"


**** To get MQTT in terminal ****
Go to terminal in command prompt and Change directory to folder with certificates

$mosquitto_pub --cafile AmazonRootCA1.pem --cert truck-1-certificate.pem.crt --key truck-1-private.pem.key -h awmminjiukh34-ats.iot.ap-south-1.amazonaws.com -p 8883 -i truck-1 -q 1 -t trucks/truck-1/temperature -q 1 -m "Hello"

$mosquitto_sub -d -h awmminjiukh34-ats.iot.ap-south-1.amazonaws.com -p 8883 --cafile AmazonRootCA1.pem --cert truck-1-certificate.pem.crt --key truck-1-private.pem.key -i truck-1 -t trucks/truck-1/temperature -q 1 -v


