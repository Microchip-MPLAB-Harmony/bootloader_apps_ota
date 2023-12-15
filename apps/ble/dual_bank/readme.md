---
parent: OTA Bootloader Applications
title: OTA Bootloader - Dual Bank
has_children: false
has_toc: false
nav_order: 5
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

To clone or download these application from Github,go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_ota) and then click Clone button to clone this repo or download as zip file. This content can also be download using content manager by following [these instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki)

# OTA Bootloader - Dual Bank

This example application shows how to use the OTA Bootloader Library to bootload an application on device having dual bank flash using RNBD.

### OTA Bootloader and Applications Block Diagram

![ota_btl_block_dual_bank_diagram](docs/images/ota_btl_block_dual_bank_diagram.png)

### OTA Bootloader

- This is a OTA bootloader which resides from
    - The starting location of the flash memory region for SAM devices

    - The starting location of the Boot flash memory region for PIC32M devices
        - Uses a Custom linker script **btl.ld** to place it in Boot flash memory

**Performs Below Operation:**

1. Read the **control block** stored in the last sector of dual bank memory

2. Checks if **blockUpdated** field is set to 1, then it will update the control block
    - SAM devices - If **blockUpdated** field is set to 0 then it jumps to **Step 4**
    - PIC32M devices - If **blockUpdated** field is set to 0 then map Program Flash Bank with higher serial number to the lower region and then it jumps to **Step 4**

3. Set **blockUpdated** field to 0 and then update the **control block** in the dual bank flash memory. And perform the following operation.
    - SAM devices - Swap bank and trigger reset
	- PIC32M devices - Trigger reset

4. Run OTA application as below
    - Calls the SYS_Deinitialize() function which releases the resources used. **This Function is device specific and has to be implemented based on application requirement**

    - **Jumps to application space to run the OTA application**
### OTA Application

- This is a OTA application which resides from
    - The end of bootloader size in device flash memory for SAM devices

    - The start of Program Flash memory for PIC32M devices
        - Uses a Custom linker script **app_mz.ld**/**app_mk.ld** to place it in Program flash memory

- It blinks an LED and provides console output

- It calls the OTA_SERVICE_Tasks() function which receives the binary over OTA to be programmed into the inactive flash bank

- Once the binary is received and programmed into the inactive flash bank, it generates a CRC32 value over programmed inactive flash bank space and verifies it against the received CRC32

- If verification is successful then it updates the **control block** and trigger reset to run new application binary through OTA bootloader

## Development Kits
The following table provides links to documentation on how to build and run OTA bootloader and application on different development kits

| Development Kit |
|:---------|
|[PIC32MK MCM Curiosity Pro Board](docs/readme_pic32mk_mcm_curiosity_pro.md) |
|[Curiosity PIC32MZ EF 2.0 Development Board](docs/readme_pic32mz_ef_curiosity.md) |
|[SAM E54 Curiosity Ultra Development Board](docs/readme_sam_e54_cult.md) |
|[SAM E54 Xplained Pro Evaluation Kit](docs/readme_sam_e54_xpro.md) |