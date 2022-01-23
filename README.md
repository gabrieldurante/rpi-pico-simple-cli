# rpi-pico-simple-cli

source build.env install-toolchain (to install rpi pico toolchain in your HOME folder)   
source build.env build (to build the cli_app.uf2 binary)   

write the UF2 image in BOOTSEL mode (mass storage)   

open terminal and attach to COM port   

say 'help" to check registered commands (if any)   

cli_test_get_single_arg -t34234   
cli_test_get_double_args -t88 -v92.009   
cli_test_get_triple_args -t2 -v0.5531 -jmystring   
