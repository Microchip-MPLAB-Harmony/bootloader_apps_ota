![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# Microchip MPLAB® Harmony 3 Release Notes

## OTA Bootloader Applications Release v3.0.0

### New Features

- This release includes support of
    - OTA Bootloader Applications with Dual Bank Flash for SAM D5x/E5x, PIC32MK GPK/MCM and PIC32MZ EF family of 32-bit microcontrollers.
    - OTA Bootloader Applications with External Memory for SAM D21/DA1, SAM D5x/E5x and SAM E70/S70/V70/V71 family of 32-bit microcontrollers.

### Development kit and demo application support
- The following table provides bootloader demo applications available for different development kits.

    | Product Family                 | Development Kits                                    | Dual Bank Flash           | External Memory            |
    | ------------------------------ | --------------------------------------------------- | ---------------- | ------------------------- |
    | SAM D21/DA1                    | [SAM D21 Xplained Pro Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=ATSAMD21-XPRO)   | NA              | Yes                        |
    | SAM D5x/E5x                    | [SAM E54 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/ATSAME54-XPRO)               | Yes              |                        |
    | SAM D5x/E5x                    | [SAM E54 Curiosity Ultra Development Board](https://www.microchip.com/en-us/development-tool/dm320210)               | Yes              | Yes                      |
    | SAM E70/S70/V70/V71            | [SAM E70 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=ATSAME70-XULT)   | NA              | Yes                        |
    | PIC32MK GPK/MCM                | [PIC32MK MCM Curiosity Pro Development Board](https://www.microchip.com/Developmenttools/ProductDetails/EV31E34A)                    | Yes              |                        |
    | PIC32MZ EF                     | [PIC32MZ EF Curiosity 2.0 Development Kit](https://www.microchip.com/en-us/development-tool/DM320209)                    | Yes              |                        |

- **NA:** Lack of product capability

### Known Issues

- N/A

### Development Tools

- [MPLAB® X IDE v6.15](https://www.microchip.com/mplab/mplab-x-ide)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator (MCC) v5.4.1
- [MPLAB® XC32 C/C++ Compiler v4.35](https://www.microchip.com/mplab/compilers)
