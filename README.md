The full documentation for this example is [available on our documentation site](https://cloud.mbed.com/docs/current/connecting/device-management-client-tutorials.html)

## Setup

### Building with Mbed CLI

1. Import the application into your desktop:
    ```
    mbed import https://github.com/ARMmbed/mbed-cloud-client-example-bg96
    cd mbed-cloud-client-example-bg96
    ```
2. Configure the API key for your Pelion Portal account.
     If you don't have an API key available, then login in [Pelion IoT Platform portal](https://portal.mbedcloud.com/), navigate to 'Access Management', 'API keys' and create a new one. Then specify the API key as global `mbed` configuration:
    ```
    mbed config -G CLOUD_SDK_API_KEY <your-api-key>
    ```
3. Create the device management and update certificates:
    ```
    mbed dm init -d "company.com" --model-name "product-model" -q --force
    ```
4.  Set the apn for your SIM card/operator in `mbed_app.json`:
    ```
    "nsapi.default-cellular-apn": "\"YOUR_APN\"",
    ```
5. Apply Mbed OS patch to fix the QSPIF driver
    ```
    cd mbed-os
    git apply ../patches/qspif.patch
    cd ..
    ```
5. Apply Mbed Cloud Client patch to fix the IAR compilation
    ```
    cd mbed-cloud-client
    git apply ../patches/cloud_client.patch
    cd ..
	```
6. Compile and program(-f):
    ```
    mbed compile -t [compiler] -m NRF52840_DK -f
    ```
    Where supported compiler options are: `GCC_ARM` or `IAR`. **Hint**: Make sure Mbed CLI is configured for your compiler. See instructions for that [here](https://os.mbed.com/docs/mbed-os/v5.12/tools/after-installation-configuring-mbed-cli.html).

### Bootloader

This repository has a pre-compiled binary that gets compiled in as the bootloader. To build your own bootloader, please see the `bootloader` folder for instructions

## Additional Details

### ERF3000 BG96 shield connections

|BG96 signal (3V3)  | NRF52840_DK Arduino Pin| Source Setting|
|-------------------|------------------------|---------------|
|Reset (active high)| P1_5                     |AppPower.cpp   |
|PWRKEY             | P1_6                    |AppPower.cpp   |
|VBAT_3V8_EN        | Not connected, enabled by default |    |
|UART TX            | p42                     |mbed_app.json  |
|UART RX            | p40                     |mbed_app.json  |
|UART RTS           | p36                    |mbed_app.json  |
|UART CTS           | p35                     |mbed_app.json  |

The RESET and PWRKey signals are used to bring the BG96 up during system startup.  The current implementation overrides the weakly defined `mbed_main` function in `AppPower.cpp`.  `mbed_main` is called before the RTOS is initialized and provides a convenient location for system init functions that are compatible with the automated test framework used with mbed os.
