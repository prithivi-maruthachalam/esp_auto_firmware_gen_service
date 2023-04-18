'''
    Flash app to act as REST server. Handles incoming
    code generation requests.
'''

from flask import Flask, request, jsonify
import uuid
import os
import json
import subprocess
import threading
import atexit
import shutil
import logging
import boto3

'''
    Application Constants
'''
BUILDS_BASE_PATH = "/tmp/builds"
OUT_FILENAME = "Auto_Build.bin"
IDF_PATH = "/home/ubuntu/esp/esp-idf"
BUCKET = "denbu-ota-files"
OTA_TOPIC = "esp/command"
S3_BUCKET_BASE_URL = "https://denbu-ota-files.s3.ap-south-1.amazonaws.com"

# Setup logging
logging.basicConfig(level=logging.DEBUG)

# Create the flash app for the server
server_app = Flask(__name__)

# Create the Builds Base Directory if it doesn't exist
os.makedirs(BUILDS_BASE_PATH, exist_ok=True)

'''
    The following is expected that this is run in an environment 
    with AWS Credentials already present. (aws-cli for example). 
    Boto3 will automatically use the credentials from that environment.
'''

# Get reference to s3 bucket
s3 = boto3.resource("s3")
ota_bucket = s3.Bucket(BUCKET)

# Get reference to IoT DataPlane client
iot_client = boto3.client('iot-data')


def run_code_generation(req_id, req_base, json_file_path, esp_proj_src):
    """
        Function that performs the esp32 code generation.
    """

    log = logging.getLogger(f'code_gen-{req_id}')
    log.info('Code Generation Started')

    # Setup log file
    log_file = open(os.path.join(req_base, "logs.txt"), "w")

    # Run source code generation
    esp_proj_src_main = os.path.join(esp_proj_src, "main")
    log.info(
        f'Generating source code from {json_file_path} into {esp_proj_src_main}')
    status = subprocess.call(['python', 'code_gen.py', '-f',
                              json_file_path, '-o', esp_proj_src_main], stdout=log_file, stderr=log_file)
    if status == 0:
        log.info(f'Source code generation successfull')
    else:
        log.error(f'Source code generation error | status {status}')
        return

    # Build esp32 firmware
    log.info(f'Building esp32 firmware from {esp_proj_src}')
    status = subprocess.call([f'. {os.path.join(IDF_PATH, "export.sh")} && idf.py build'],
                             cwd=os.path.abspath(esp_proj_src), shell=True, stdout=log_file, stderr=log_file)
    if status == 0:
        log.info(f'ESP32 firmware generation successfull')
    else:
        log.error(f'ESP32 firmware generation error | status {status}')
        return

    # Close log file
    log_file.close()

    # Copy firmware binary to base dir
    binary_file_path_old = os.path.join(esp_proj_src, "build", OUT_FILENAME)
    binary_file_path_new = os.path.join(req_base, OUT_FILENAME)
    shutil.copyfile(binary_file_path_old, binary_file_path_new)
    log.info(f'Placed firmware binary : {binary_file_path_new}')

    # Delete source files
    shutil.rmtree(esp_proj_src)
    log.info('Deleted esp source files')

    # Upload firmware binary to s3
    try:
        log.info(f'Uploading {binary_file_path_new} to s3')
        ota_bucket.upload_file(os.path.abspath(
            binary_file_path_new), f'{req_id}/{OUT_FILENAME}')
        log.info(f'Uploaded to s3')
    except:
        log.error('Error uploading to s3')
        return

    # Send MQTT message with public url
    file_url = f'{S3_BUCKET_BASE_URL}/{req_id}/{OUT_FILENAME}'
    log.debug(f'Uploaded file {file_url}')
    iot_client.publish(
        topic=OTA_TOPIC,
        contentType='OTA location',
        payloadFormatIndicator='UTF8_DATA',
        payload=file_url)
    log.info(f'Published to MQTT')


@server_app.route("/gen_code", methods=['POST'])
def gen_code():
    """
        Request handler for the /gen_code endpoint. Expects the json configuation
        as in the request body. Generates esp32 firmware binary based on the 
        json configuration, uploads it to s3 and sends a message to the esp through AWS IoT MQTT.

        Responses::
            - Unsupported Media Type for Body (not json) - {"error" : "Invalid Request Body Type"}, 415
            - Invalid Request Body (bad json) - {"error": "Error Parsing Request Body"}, 400
            - Request Accepted (but not completed) - {"req_id": <The Request ID>}, 202
    """

    print()
    if request.is_json:
        # Generate new unique request ID
        request_id = str(uuid.uuid4())
        log = logging.getLogger(request_id)
        log.info(f'Starting run for UUID : {request_id}')

        try:
            data = request.get_json()   # Parse json body into dictionary
            log.info('Request body parsed')
        except:
            log.error("Error Parsing Request Body")
            return jsonify({"error": "Error Parsing Request Body"}), 400

        # Create base folder for this request
        request_base_path = os.path.join(BUILDS_BASE_PATH, request_id)
        os.makedirs(request_base_path, exist_ok=True)
        log.info('Created base dir for request')

        # Save json config to input file in tmp folder
        input_file_path = os.path.join(request_base_path, "input.json")
        with open(input_file_path, "w") as input_file:
            json.dump(data, input_file)
        log.info('Created json input file')

        # Copy esp source files to the tmp folder
        esp_source_path = os.path.join(request_base_path, "esp_project_source")
        shutil.copytree("./esp_source_project",
                        esp_source_path)
        log.info('Copied esp project source files')

        # Start code generation as a separate thread
        threading.Thread(target=run_code_generation, args=(
            request_id,
            request_base_path,
            input_file_path,
            esp_source_path)).start()

        return jsonify({"req_id": f'{request_id}'}), 202

    else:
        return jsonify({"error": "Invalid Request Body"}), 415


def exit_handler():
    """
        This function is called using the atexit module before exiting
        from this script. It is used to clear all the temp files created
        for building binaries.
    """
    print(f'Removing all builds')
    shutil.rmtree(BUILDS_BASE_PATH)


# register exit handler
atexit.register(exit_handler)

if __name__ == "__main__":
    server_app.run("0.0.0.0", port=5000)
