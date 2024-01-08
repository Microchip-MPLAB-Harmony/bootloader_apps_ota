# SAM D21 Xplained Pro Evaluation Kit: Building and Running the OTA Bootloader applications

**Parent topic:**[OTA Bootloader - External Memory](GUID-BDF2405C-B905-4252-AEB3-07922B1A4748.md)

## Downloading and building the application

To clone or download this application from Github,go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_ota) and then click Clone button to clone this repo or download as zip file. This content can also be download using content manager by following [these instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki)

Path of the application within the repository is **apps/ble/external\_memory/**

To build the application, refer to the following table and open the project using its IDE.

### OTA Bootloader

|Project Name|Description|
|------------|-----------|
|ota\_bootloader/firmware/sam\_d21\_xpro.X|MPLABX Project for [SAM D21 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamd21-xpro)|

### OTA application

|Project Name|Description|
|------------|-----------|
|ota\_app/firmware/sam\_d21\_xpro.X|MPLABX Project for [SAM D21 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamd21-xpro)|

## Setting up [SAM D21 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamd21-xpro)

-   To run the demo, the following additional hardware are required:

    -   [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a)

    -   [EEPROM 4 click board](https://www.mikroe.com/eeprom-4-click)

    -   [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) - 2 boards

-   Short J2-1 and J2-2 using jumper in the [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a)

-   Install an [RNBD451 Add On Board](https://www.microchip.com/en-us/development-tool/ev25f14a) on to the [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) board 1

-   Connect [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) to the EXT1 header

-   Install an [EEPROM 4 click board](https://www.mikroe.com/eeprom-4-click) on to the [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) board 2

-   Connect [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) to the EXT2 header

-   Connect the Debug USB port on the board to the computer using a micro USB cable


**Setting up the host scripts**

-   Refer to [OTA Bootloader Host Script Help](GUID-ECC16920-EE69-48DB-9B32-F774CEB2D079.md) for settings up python scripts which are used in order to update the application binary image


## Running the Application

1.  Open the OTA bootloader project *ota\_bootloader/firmware/sam\_d21\_xpro.X* in the IDE

2.  Make sure that the *ota\_app/firmware/sam\_d21\_xpro.X* is added as a loadable project to OTA bootloader application

    -   As the external memory may not have any valid binary **required by OTA bootloader** for the first time, Adding the **ota\_app as loadable** allows MPLAB X to create a **unified hex file** and program both these applications in their respective memory locations based on their linker script configurations

    ![mplab_loadable_sam_d21_xpro](GUID-0062420A-A85D-4EB6-BEB4-5E2D653007CE-low.png)

3.  Open the Terminal application \(Ex.:Tera Term\) on the computer

4.  Connect to the Virtual COM port and configure the serial settings as follows:

    -   Baud : 115200

    -   Data : 8 Bits

    -   Parity : None

    -   Stop : 1 Bit

    -   Flow Control : None

5.  Build and program the **OTA bootloader application** using the IDE

6.  Once programming is done OTA bootloader starts execution and directly jumps to application space to **run the OTA application**

    -   **LED0** starts blinking indicating that the **OTA application** is running

    -   Observe the below message on console

    ![console_output1](GUID-9D178005-EEBB-4703-8040-C6A82CFB823B-low.png)

7.  Open the OTA application project ota\_app/firmware/sam\_d21\_xpro.X in the IDE

8.  Update ota\_app/firmware/src/app.c to update printf\(\) message from **\#\#\#\#\#\#\# Application running \#\#\#\#\#\#\#** to **\#\#\#\#\#\#\# New Application running \#\#\#\#\#\#\#** as below

    ![app_source](GUID-D687C0A5-8D2B-49B5-8265-524D6DADD565-low.png)

9.  Clean and Build the OTA application project to generate the binary \(Do not program the binary\)

10. Go to the **<harmony3\_path\>/bootloader\_apps\_ota/apps/ble/external\_memory/ota\_app/firmware/sam\_d21\_xpro.X/dist/sam\_d21\_xpro/production/** directory and open the command prompt to run the below command

    ```
    python <harmony3_path>/bootloader_apps_ota/tools/ota_host_mcu_header.py -p 0x2000 -j 0x2000 -l 0x0 -f sam_d21_xpro.X.production.bin
    ```

    -   This command adds OTA Host MCU Header to the application binary. If command executed successful then "image.bin is generated successfully" message display on the command prompt.

11. Run the below command

    ```
    python <harmony3_path>/bootloader_apps_ota/tools/ota_rnbd_header.py -f image.bin
    ```

    -   This command adds OTA RNBD Header to the image.bin and generates **RNBD\_image.bin** application binary. If command executed successful then "RNBD\_image.bin is generated successfully" message display on the command prompt.

12. Refer [Send application binary using MBD App](GUID-B3F7BD29-9883-431D-B682-F405F22C1BE0.md) to program **RNBD\_image.bin** application binary to the target.

13. Once Firmware Update is successful

    -   **LED0** still blinking indicating that the **OTA application** is running

    -   Observe the below message on console

    ![console_output2](GUID-369A9565-1D83-462A-AAE8-7F6B5D780CBE-low.png)

14. Repeat Step 8-13.

    -   Observe **LED0** still blinking

    -   Observe message printed on console


