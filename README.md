The full documentation for this example is [available on our documentation site](https://cloud.mbed.com/docs/current/connecting/device-management-client-tutorials.html)

## Instructions

```
# Get tagged libraries
mbed deploy
# Use cell configuration
cp configs/cellular-bg96.json mbed_app.json
# Compile and flash
mbed compile -t GCC_ARM -m NRF52840_DK -f
```

## Bootloader

This repository has a pre-compiled binary that gets compiled in as the bootloader. To build your own bootloader, please see the `bootloader` folder for instructions