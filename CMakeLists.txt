cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(valliid-dmx-usb)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (TARGET tinyusb_device)
    add_executable(valliid-dmx-usb
            valliid-dmx-usb.cpp
            )

    # pull in common dependencies
    target_link_libraries(valliid-dmx-usb pico_multicore pico_stdlib hardware_dma hardware_pio hardware_spi)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(valliid-dmx-usb 1)
    pico_enable_stdio_uart(valliid-dmx-usb 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(valliid-dmx-usb)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
