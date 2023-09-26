/*******************************************************************************
  OTA Bootloader Source File

  File Name:
    bootloader_ota.c

  Summary:
    This file contains source code necessary to execute OTA bootloader.

  Description:
    This file contains source code necessary to execute OTA bootloader.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "ota_service_control_block.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#define DATA_SIZE                        PAGE_SIZE
#define BUFFER_SIZE(x, y)                ((y) > (x)? ((((y) % (x)) != 0U)?((((y) / (x)) + 1U) * (x)) : (((y) / (x)) * (x))) : (x))
#define OTA_CONTROL_BLOCK_SIZE           sizeof(OTA_CONTROL_BLOCK)

#define OTA_CONTROL_BLOCK_PAGE_SIZE      PAGE_SIZE

#define OTA_CONTROL_BLOCK_BUFFER_SIZE    BUFFER_SIZE(OTA_CONTROL_BLOCK_PAGE_SIZE, OTA_CONTROL_BLOCK_SIZE)


typedef enum
{
    BTL_STATE_INIT = 0,

    BTL_STATE_GET_METADATA,

    BTL_STATE_CHECK_META_DATA_UPDATE,

    BTL_STATE_READ_APP_IMAGE,

    BTL_STATE_PROGRAM_FLASH,

    BTL_STATE_VERIFY_SIGNATURE,

    BTL_STATE_UPDATE_META_DATA,

    BTL_STATE_RUN_APPLICATION,

    BTL_STATE_ERROR

} BTL_STATES;


typedef struct
{
    BTL_STATES state;

    uint32_t appJumpAddress;

    OTA_CONTROL_BLOCK *controlBlock;
} BTL_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

static uint8_t CACHE_ALIGN controlBlockBuffer[OTA_CONTROL_BLOCK_BUFFER_SIZE];
static uint32_t ctrlBlkSize = OTA_CONTROL_BLOCK_BUFFER_SIZE;

static BTL_DATA btlData =
{
    .state              = BTL_STATE_INIT,
    .appJumpAddress     = APP_JUMP_ADDRESS,
    .controlBlock       = (OTA_CONTROL_BLOCK *)controlBlockBuffer,
};

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Local Functions
// *****************************************************************************
// *****************************************************************************


static bool bootloader_OTA_CtrlBlkRead(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        appMetaDataAddress  = (FLASH_END_ADDRESS - ctrlBlkSize);
        (void)NVM_Read((void *)ptrBuffer, ctrlBlkSize, appMetaDataAddress);
        status = true;
    }

    return status;
}

static bool bootloader_OTA_CtrlBlkWrite(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t count;
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        appMetaDataAddress  = (FLASH_END_ADDRESS - ctrlBlkSize);


        /* Erase the Current sector */
        (void)NVM_PageErase(appMetaDataAddress);

        while(NVM_IsBusy() == true)
        {

        }

        for (count = 0; count < length; count += OTA_CONTROL_BLOCK_PAGE_SIZE)
        {
            (void)NVM_RowWrite((void *)ptrBuffer, appMetaDataAddress);

            while(NVM_IsBusy() == true)
            {

            }
            appMetaDataAddress += OTA_CONTROL_BLOCK_PAGE_SIZE;
            ptrBuffer += OTA_CONTROL_BLOCK_PAGE_SIZE;
        }

        status = true;
    }

    return status;
}

static void bootloader_OTA_ProgramFlashSwapBank(T_FLASH_BANK flash_bank)
{
    /* NVMOP can be written only when WREN is zero. So, clear WREN */
    NVMCONCLR = _NVMCON_WREN_MASK;

    /* Write the unlock key sequence */
    NVMKEY = 0x0U;
    NVMKEY = 0xAA996655U;
    NVMKEY = 0x556699AAU;

    if (flash_bank == PROGRAM_FLASH_BANK_1)
    {
        /* Map Program Flash Memory Bank 1 to lower region */
        NVMCONCLR = _NVMCON_PFSWAP_MASK;
    }
    else if (flash_bank == PROGRAM_FLASH_BANK_2)
    {
        /* Map Program Flash Memory Bank 2 to lower region */
        NVMCONSET = _NVMCON_PFSWAP_MASK;
    }
    else
    {
        /* Do nothing */
    }
}

static uint32_t bootloader_OTA_FlashSerialGet(T_FLASH_BANK flash_bank)
{
    uint32_t serialNum;
    uint32_t *ptrSerialNum;

    if (flash_bank == PROGRAM_FLASH_BANK_1)
    {
        ptrSerialNum = (uint32_t *)KVA0_TO_KVA1((INACTIVE_BANK_START - ctrlBlkSize));
    }
    else
    {
        ptrSerialNum = (uint32_t *)KVA0_TO_KVA1((FLASH_END_ADDRESS - ctrlBlkSize));
    }
    serialNum = *ptrSerialNum;

    if (serialNum == 0xFFFFFFFFU)
    {
        serialNum = 0U;
    }

    return serialNum;
}

static bool bootloader_OTA_CheckForUpdate(OTA_CONTROL_BLOCK *controlBlock)
{
    bool status = false;

    if ((controlBlock->versionNum != 0xFU) && (controlBlock->ActiveImageNum < 1U))
    {
        if (controlBlock->appImageInfo[controlBlock->ActiveImageNum].status == OTA_CB_STATUS_VALID)
        {
            btlData.appJumpAddress  = controlBlock->appImageInfo[controlBlock->ActiveImageNum].jumpAddress;
        }

        if (controlBlock->blockUpdated == 1U)
        {
            status = true;
        }
        else
        {
            uint32_t serialNum1;
            uint32_t serialNum2;

            /* Map Program Flash Bank 1 to lower region after a reset */
            bootloader_OTA_ProgramFlashSwapBank(PROGRAM_FLASH_BANK_1);

            serialNum1 = bootloader_OTA_FlashSerialGet(PROGRAM_FLASH_BANK_1);
            serialNum2 = bootloader_OTA_FlashSerialGet(PROGRAM_FLASH_BANK_2);

            if (serialNum2 > serialNum1)
            {
                /* Map Program Flash Bank 2 to lower region */
                bootloader_OTA_ProgramFlashSwapBank(PROGRAM_FLASH_BANK_2);
            }
        }
    }

    return status;
}

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Global Functions
// *****************************************************************************
// *****************************************************************************

void bootloader_OTA_Tasks (void)
{
    /* Check the application's current state. */
    switch (btlData.state)
    {
        case BTL_STATE_INIT:
        {
            btlData.state = BTL_STATE_GET_METADATA;
            break;
        }

        case BTL_STATE_GET_METADATA:
        {
            if (bootloader_OTA_CtrlBlkRead(btlData.controlBlock, ctrlBlkSize) == true)
            {
                btlData.state = BTL_STATE_CHECK_META_DATA_UPDATE;
            }
            else
            {
                btlData.state = BTL_STATE_RUN_APPLICATION;
            }

            break;
        }

        case BTL_STATE_CHECK_META_DATA_UPDATE:
        {
            if (bootloader_OTA_CheckForUpdate(btlData.controlBlock) == true)
            {
                btlData.state = BTL_STATE_UPDATE_META_DATA;
            }
            else
            {
                if (bootloader_Trigger() == true)
                {
                    btlData.state = BTL_STATE_GET_METADATA;
                }
                else
                {
                    btlData.state = BTL_STATE_RUN_APPLICATION;
                }
            }

            break;
        }

        case BTL_STATE_UPDATE_META_DATA:
        {
            if (bootloader_OTA_CtrlBlkRead(btlData.controlBlock, ctrlBlkSize) == true)
            {
                uint32_t serialNum = bootloader_OTA_FlashSerialGet(PROGRAM_FLASH_BANK_1);
                btlData.controlBlock->serialNum = (serialNum + 1U);
                btlData.controlBlock->blockUpdated = 0U;
                btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].imageType = OTA_CB_IMAGETYPE_ACTIVE;
                btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].status = OTA_CB_STATUS_VALID;

                (void)bootloader_OTA_CtrlBlkWrite(btlData.controlBlock, ctrlBlkSize);
            }

            /* Reset */
            bootloader_TriggerReset();

            break;
        }

        case BTL_STATE_RUN_APPLICATION:
        {
            run_Application(btlData.appJumpAddress);

            break;
        }

        case BTL_STATE_ERROR:
        default:
        {
            // Do nothing
            break;
        }
    }
}
