# Building your own bootloader
This project uses the public mbed-bootloader repository with application specific optimizations.

## Instructions

1. In your home directory (not inside your `mbed-cloud-client-example-bg96` directory), run:

    ```
    git clone git@github.com:ARMmbed/mbed-bootloader.git
    cd mbed-bootloader
    mbed deploy
    ```

2. In this folder you should see 4 files

    * `storage_add.patch`         - 
    * `disable_bl_trace.patch`    - disable bootloader app prints (~6KB) - *NOTE:* If bootloader app trace is needed for debug, do not apply this patch and the flash map must be adjusted to accomodate the additional size. 24KB->32KB bootloader partition size.
    * `disable_qspif_trace.patch` - remove qspif traces (~4KB) from build; this is a bug that is being investigated
    * `mbed_app.json`             - 24KB bootloader flash map

3. Copy the above files into the `mbed-bootloader` home. 

4. Run the following commands:

    ```
    git apply storage_add.patch
    git apply disable_bl_trace.patch
    cd mbed-os
    git apply ../disable_qspif_trace.patch
    cd ..
    mbed compile -t GCC_ARM -m NRF52840_DK --profile tiny.json
    cp ./BUILD/NRF52840_DK/GCC_ARM/mbed-bootloader.hex ~/mbed-cloud-client-example-bg96/tools/mbed-bootloader.hex
    ```
    
    Note: The last copy command placed the bootloader in the tools directory with the name `mbed-bootloader.hex`.
    In order to use this bootloader, make sure `mbed_app.json` in the `mbed-cloud-client-example-bg96` repository has the following set:
    ```
    "target.bootloader_img": "tools/mbed-bootloader.hex",
    ```

    The following commands applied a patch for default application start memory addresses, compiled the bootloader, and moved it into the `tools` folder of the `mbed-cloud-client-example-bg96` repository
