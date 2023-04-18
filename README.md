# Service for esp32 auto code generation

## Server setup (EC2)
- `sudo apt update && sudo apt upgrade -y`
- Make sure python is installed. If only python3 is available, install `sudo apt install python-is-python3`
- Install esp-idf
    - The newest version doesn't work. Make sure you install the right version of esp-idf
    - Clone the esp-idf repository and set it up as per the instructions on espressif.
    - Make sure you export the IDF_PATH variable in the .bashrc file so that it is available on every shell.
- Clone this repository
    - The following certificates are not included in the repository
        - `esp_auto_firmware_gen_service/esp_source_project/main/certs/client.crt`
        - `esp_auto_firmware_gen_service/esp_source_project/main/certs/client.key`
        - `esp_auto_firmware_gen_service/esp_source_project/main/certs/root_cert_auth.pem`
    - Get these files and put them in the following path within the repository: `esp_auto_firmware_gen_service/esp_source_project/main/certs/` (I recommend scp if you have ssh setup)
- Install the requirements for the server : `pip install -r requirements.txt`
- Setup AWS permissions
    - Get the access key and secret to an aws role/account that has the following permissions
        - Upload to s3
        - Publish to AWS IoT Core Data Plane
- Flask by default, listens on port **5000**. Make sure that the inbound rules for the ec2 instance allow custom traffic on this port from any IP address.
- Run the server - `python server.py`