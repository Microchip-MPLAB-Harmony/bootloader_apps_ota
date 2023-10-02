---
grand_parent: OTA Bootloader Applications
parent: OTA Bootloader - Dual Bank
title: Building and Running on Curiosity PIC32MZ EF 2.0 Development Board
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# Building and Running the OTA Bootloader applications

## Downloading and building the application

To clone or download this application from Github,go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_ota) and then click Clone button to clone this repo or download as zip file. This content can also be download using content manager by following [these instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki)

Path of the application within the repository is **apps/ble/dual_bank/**

To build the application, refer to the following table and open the project using its IDE.

### OTA Bootloader

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| ota_bootloader/firmware/pic32mz_ef_curiosity.X    | MPLABX Project for [Curiosity PIC32MZ EF 2.0 Development Board](https://www.microchip.com/developmenttools/ProductDetails/DM320209)|


### OTA application

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| ota_app/firmware/pic32mz_ef_curiosity.X    | MPLABX Project for [Curiosity PIC32MZ EF 2.0 Development Board](https://www.microchip.com/developmenttools/ProductDetails/DM320209)|

## Setting up [Curiosity PIC32MZ EF 2.0 Development Board](https://www.microchip.com/developmenttools/ProductDetails/DM320209)

- To run the demo, the following additional hardware are required:
  - [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a)

- Short J2-1 and J2-2 using jumper in the [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a)
- Insert [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a) board into MIKROBUS Connector 1.
- For programming and UART Console, connect a micro USB cable to the USB Debug port J700

## Running the Application

1. Open the OTA bootloader project *ota_bootloader/firmware/pic32mz_ef_curiosity.X* in the IDE

2. Make sure that the *ota_app/firmware/pic32mz_ef_curiosity.X* is added as a loadable project to OTA bootloader project
    - As the Dual Bank Flash memory may not have any valid binary **required by OTA bootloader** for the first time, Adding the **ota_app as loadable** allows MPLAB X to create a **unified hex file** and program both these applications in their respective memory locations based on their linker script configurations

    ![mplab_loadable_pic32mz_ef_curiosity](images/mplab_loadable_pic32mz_ef_curiosity.png)

3. Open the Terminal application (Ex.:Tera Term) on the computer
4. Connect to the Serial COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None

5. Build and Program the **OTA bootloader application** using the IDE

6. Once programming is done OTA bootloader starts execution and directly jumps to application space to **run the OTA application**
    - **LED1** starts blinking indicating that the **OTA application** is running
	- Observe the below message on console

    ![console_output1](images/pic32m_output1.png)

7. Go to the **\<harmony3_path\>/bootloader_apps_ota/apps/ble/dual_bank/ota_app/firmware/pic32mz_ef_curiosity.X/dist/pic32mz_ef_curiosity/production/** directory and open the command prompt to run the below command

        python <harmony3_path>/bootloader_apps_ota/tools/ota_host_mcu_header.py -p 0x9D100000 -j 0x9D000200 -f pic32mz_ef_curiosity.X.production.bin

    - This command adds OTA Host MCU Header to the application binary. If command executed successful then "image.bin is generated successfully" message display on the command prompt.

8. Run the below command

        python <harmony3_path>/bootloader_apps_ota/tools/ota_rnbd_header.py -f image.bin

    - This command adds OTA RNBD Header to the image.bin and generates **RNBD_image.bin** application binary. If command executed successful then "RNBD_image.bin is generated successfully" message display on the command prompt.

9. Refer [Send application binary using MBD App](../../docs/readme_ota_app.md) to program **RNBD_image.bin** application binary to the target.

10. Once Firmware Update is successful
    - **LED1** still blinking indicating that the **OTA application** is running
	- Observe the below message on console

    ![console_output2](images/pic32m_output2.png)

11. Repeat Step 9-10.
    - Observe **LED1** still blinking
	- Observe message printed on console **####### Application running from Program Flash BANK 1 #######** or **####### Application running from Program Flash BANK 2  #######** alternatively.
