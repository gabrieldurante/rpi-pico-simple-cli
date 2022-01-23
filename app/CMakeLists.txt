if (TARGET tinyusb_device)
    add_executable(cli_app app.c)

    # pull in common dependencies
    target_link_libraries(cli_app
                          pico_stdlib
                          utils)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(cli_app 1)
    pico_enable_stdio_uart(cli_app 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(cli_app)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building cli_app because TinyUSB submodule is not initialized in the SDK")
endif()
