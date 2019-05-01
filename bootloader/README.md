# Building your own bootloader

## Instructions

1. In your home directory (not inside your `mbed-cloud-client-example-bg96` directory), run:

    ```
    git clone git@github.com:ARMmbed/mbed-bootloader.git
    cd mbed-bootloader
    mbed deploy
    ```

2. In this folder you should see 2 files

    * `storage_add.patch`
    * `mbed_app.json`

3. Copy the above files into the `mbed-bootloader` home. 

4. Run the following commands:

    ```
    git apply storage_add.patch
    mbed compile -t GCC_ARM -m NRF52840_DK --profile tiny.json
    cp ./BUILD/NRF52840_DK/GCC_ARM/mbed-bootloader.hex ~/mbed-cloud-client-example-bg96/tools/mbed-bootloader.hex
    ```

    The following commands applied a patch for default application start memory addresses, compiled the bootloader, and moved it into the `tools` folder of the `mbed-cloud-client-example-bg96` repository
