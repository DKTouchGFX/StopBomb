# STM32C071RB-NUCLEO-RVA15MD TBS

The TBS is based on the Team Source Display TSS013004A-AD with the optional adapter board. The default IDE is set to STM32CubeIDE, to change IDE open TSDHD-TSS013-1.ioc with STM32CubeMX and select from the supported IDEs.

This TBS is configured for 240 x 240 pixels 16bpp screen resolution.

The adapter board has a USB to serial converter interface mounted, which is connected to USART1 on the MCU. Using the interface requires a driver, which can be downloaded from https://www.wch-ic.com/downloads/ch341ser_exe.html.

As an alternative to using an external ST-Link device, code can me loaded onto the internal memory of the MCU using the serial interface on the adapter board using the bootloader built into the STM32 MCU. The boot loader can be entered using the "modem" signals on the serial interface by pulling the RTS signal, which is connected to the reset pin, low and then holding the CTS signal high while the RTS signal is released.
This is not currently supported for this device in STM32CubeProgrammer, instead a custom Ruby script to load code is supplied with the TBS.
By default, the script enters the built-in bootloader state, and then loads a custom bootloader based on the ST OpenBootloader, which can program the external as well as the internal flash. This bootloader is loaded into the SRAM of the MCU, so it can program the entire internal flash. For more information about OpenBootloader, visit https://github.com/STMicroelectronics/stm32-mw-openbl.

There are two recommended ways to use the script:
    
    - The "Run Target" button in the designer will compile the project and load it using the script. To save time, there is an option to flash only the internal flash if there has been no changes to data in external memory.
    
    - The script can also be run from the command line. TouchGFX includes "TouchGFX 4.xx.x Environment", which is a command line environment with Ruby and all the required Ruby Gems installed, which can be used to execute the script. This is done by navigating to the "Loading" folder in the project, and then executing "ruby Load_OpenBootloader.rb <file> (internal)" The <file> argument can be any executable file type supported by STM32CubeProgrammer (e.g. .hex or .elf).
    Additionally, "internal" can be supplied as an optional argument, which will only flash the internal flash as described above.

If using an ST-Link is desired, for example for debug purposes, an external loader is located in the "Loading" subfolder in the project. This is used by default in the included STM32CubeIDE project, and can be used with STM32CubeProgrammer by copying it into the /bin/ExternalLoader subfolder of the STM32CubeProgrammer installation directory.

Performance testing can be performed using the GPIO pins designated with the following signals:
 - VSYNC_FREQ  - GPIO PA3
 - RENDER_TIME - GPIO PA4
 - FRAME_RATE  - GPIO PA5
If the pins are needed for other functionality, they can safely be reassigned as needed.