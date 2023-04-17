# AWS_MQTT

This code uses the AWS IoT SDK for connection to AWS. To download the AWS SDk used here:

    git clone --recursive https://github.com/espressif/esp-aws-iot

    cd esp-aws-iot/
    git checkout release/v3.1.x

    git submodule update –recursive


These files are required for our AWS_MQTT code to work. Add this folder as an extra component in the CMakeLists

    set(EXTRA_COMPONENT_DIRS "esp-aws-iot")

## OTA

For OTA, send the following message on esp/command

{
  "update": "http://denbu-ota-test.s3.ap-northeast-1.amazonaws.com/AWS_MQTT.bin"
}