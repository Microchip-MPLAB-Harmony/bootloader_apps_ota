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
#include "configuration.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#define DATA_SIZE                        PAGE_SIZE
#define BUFFER_SIZE(x, y)                ((y) > (x)? ((((y) % (x)) != 0U)?((((y) / (x)) + 1U) * (x)) : (((y) / (x)) * (x))) : (x))
#define OTA_CONTROL_BLOCK_SIZE           sizeof(OTA_CONTROL_BLOCK)

#define OTA_CONTROL_BLOCK_PAGE_SIZE      DRV_AT25_EEPROM_PAGE_SIZE

#define OTA_CONTROL_BLOCK_BUFFER_SIZE    BUFFER_SIZE(OTA_CONTROL_BLOCK_PAGE_SIZE, OTA_CONTROL_BLOCK_SIZE)

typedef enum
{
    /* Transfer being processed */
    BTL_MEM_TRANSFER_BUSY,

    /* Transfer is successfully completed */
    BTL_MEM_TRANSFER_COMPLETED,

    /* Transfer had error*/
    BTL_MEM_TRANSFER_ERROR_UNKNOWN

} BTL_MEM_TRANSFER_STATUS;

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
    uint32_t read_blockSize;
    uint32_t read_numBlocks;
    uint32_t numReadRegions;

    uint32_t write_blockSize;
    uint32_t write_numBlocks;
    uint32_t numWriteRegions;

    uint32_t erase_blockSize;
    uint32_t erase_numBlocks;
    uint32_t numEraseRegions;

    uint32_t blockStartAddress;
} BTL_MEM_GEOMETRY;

typedef struct
{
    BTL_STATES state;

    uintptr_t handle;

    BTL_MEM_GEOMETRY geometry;

    uint32_t read_index;

    uint32_t write_index;

    uint32_t appJumpAddress;

    OTA_CONTROL_BLOCK *controlBlock;
} BTL_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

static uint8_t CACHE_ALIGN controlBlockBuffer[OTA_CONTROL_BLOCK_BUFFER_SIZE];
static uint8_t CACHE_ALIGN flash_data[DATA_SIZE];
static uint32_t ctrlBlkSize = OTA_CONTROL_BLOCK_BUFFER_SIZE;

static BTL_DATA btlData =
{
    .state              = BTL_STATE_INIT,
    .appJumpAddress     = APP_START_ADDRESS,
    .controlBlock       = (OTA_CONTROL_BLOCK *)controlBlockBuffer,
};

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Local Functions
// *****************************************************************************
// *****************************************************************************

static bool bootloader_OTA_WaitForXferComplete(void)
{
    bool status = false;

    BTL_MEM_TRANSFER_STATUS transferStatus = BTL_MEM_TRANSFER_ERROR_UNKNOWN;

    do
    {
        transferStatus = (BTL_MEM_TRANSFER_STATUS)DRV_AT25_TransferStatusGet(btlData.handle);

    } while (transferStatus == BTL_MEM_TRANSFER_BUSY);

    if(transferStatus == BTL_MEM_TRANSFER_COMPLETED)
    {
        status = true;
    }

    return status;
}

static void bootloader_OTA_FlashWrite(void *data, uint32_t address)
{
    uint32_t *buffer = (uint32_t *)data;

    if (0U == (address % ERASE_BLOCK_SIZE))
    {
        // Lock region size is always bigger than the row size
        NVMCTRL_RegionUnlock(address);

        while(NVMCTRL_IsBusy() == true)
        {

        }

        /* Erase the Current sector */
        (void)NVMCTRL_RowErase(address);

        while(NVMCTRL_IsBusy() == true)
        {

        }
    }

    (void)NVMCTRL_PageWrite(buffer, address);

    while(NVMCTRL_IsBusy() == true)
    {

    }
}

static bool bootloader_OTA_CtrlBlkRead(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t count;
    uint32_t blockSize;
    uint32_t otaMemoryStart;
    uint32_t otaMemorySize;
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        otaMemoryStart    = btlData.geometry.blockStartAddress;

        otaMemorySize     = (btlData.geometry.read_blockSize * btlData.geometry.read_numBlocks);

        blockSize = btlData.geometry.write_blockSize;

        appMetaDataAddress  = ((otaMemoryStart + otaMemorySize) - BUFFER_SIZE(blockSize, ctrlBlkSize));

        for (count = 0; count < length; count += OTA_CONTROL_BLOCK_PAGE_SIZE)
        {
            if (DRV_AT25_Read(btlData.handle, ptrBuffer, OTA_CONTROL_BLOCK_PAGE_SIZE, appMetaDataAddress) != true)
            {
                status = false;
                break;
            }

            status = bootloader_OTA_WaitForXferComplete();
            appMetaDataAddress += OTA_CONTROL_BLOCK_PAGE_SIZE;
            ptrBuffer += OTA_CONTROL_BLOCK_PAGE_SIZE;
        }
    }

    return status;
}

static bool bootloader_OTA_CtrlBlkWrite(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t count;
    uint32_t blockSize;
    uint32_t otaMemoryStart;
    uint32_t otaMemorySize;
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        otaMemoryStart    = btlData.geometry.blockStartAddress;

        otaMemorySize     = (btlData.geometry.read_blockSize * btlData.geometry.read_numBlocks);

        blockSize = btlData.geometry.write_blockSize;

        appMetaDataAddress  = ((otaMemoryStart + otaMemorySize) - BUFFER_SIZE(blockSize, ctrlBlkSize));


        for (count = 0; count < length; count += OTA_CONTROL_BLOCK_PAGE_SIZE)
        {
            if (DRV_AT25_PageWrite(btlData.handle, ptrBuffer, appMetaDataAddress) != true)
            {
                status = false;
                break;
            }

            status = bootloader_OTA_WaitForXferComplete();
            appMetaDataAddress += OTA_CONTROL_BLOCK_PAGE_SIZE;
            ptrBuffer += OTA_CONTROL_BLOCK_PAGE_SIZE;
        }
    }

    return status;
}


static bool bootloader_OTA_CheckForUpdate(OTA_CONTROL_BLOCK *controlBlock)
{
    bool status = false;

    if ((controlBlock->versionNum != 0xFU) && (controlBlock->ActiveImageNum < 2U))
    {
        if (controlBlock->appImageInfo[controlBlock->ActiveImageNum].status == OTA_CB_STATUS_VALID)
        {
            btlData.appJumpAddress  = controlBlock->appImageInfo[controlBlock->ActiveImageNum].jumpAddress;
        }

        if (controlBlock->blockUpdated == 1U)
        {
            status = true;
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
            if (DRV_AT25_Status(DRV_AT25_INDEX) == SYS_STATUS_READY)
            {
                btlData.handle = DRV_AT25_Open(DRV_AT25_INDEX, DRV_IO_INTENT_READWRITE);

                if (btlData.handle != DRV_HANDLE_INVALID)
                {
                    if (DRV_AT25_GeometryGet(btlData.handle, (void *)&btlData.geometry) != true)
                    {
                        btlData.state = BTL_STATE_ERROR;
                        break;
                    }
                    btlData.state = BTL_STATE_GET_METADATA;
                }
            }
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
                btlData.state = BTL_STATE_READ_APP_IMAGE;
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

        case BTL_STATE_READ_APP_IMAGE:
        {
            (void)memset((void *)flash_data, 0xFF, DATA_SIZE);

            if (DRV_AT25_Read(btlData.handle, &flash_data[0], DATA_SIZE, (btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].loadAddress + btlData.read_index)) == false)
            {
                btlData.state = BTL_STATE_ERROR;
                break;
            }

            if (bootloader_OTA_WaitForXferComplete() == true)
            {
                btlData.state = BTL_STATE_PROGRAM_FLASH;
            }
            else
            {
                btlData.state = BTL_STATE_ERROR;
            }

            break;
        }

        case BTL_STATE_PROGRAM_FLASH:
        {
            bootloader_OTA_FlashWrite(&flash_data[0], (btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].programAddress + btlData.write_index));

            btlData.write_index += DATA_SIZE;
            btlData.read_index += DATA_SIZE;

            if (btlData.read_index >= btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].imageSize)
            {
                btlData.state = BTL_STATE_VERIFY_SIGNATURE;
            }
            else
            {
                btlData.state = BTL_STATE_READ_APP_IMAGE;
            }

            break;
        }

        case BTL_STATE_VERIFY_SIGNATURE:
        {
            uint32_t crc32;
            uint32_t *received_crc32 = (void *)btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].signature;
            crc32 = bootloader_CRCGenerate(btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].programAddress, btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].imageSize);

            if (crc32 == *received_crc32)
            {
                btlData.state = BTL_STATE_UPDATE_META_DATA;
            }
            else
            {
                btlData.state = BTL_STATE_ERROR;
            }

            break;
        }
        case BTL_STATE_UPDATE_META_DATA:
        {
            if (bootloader_OTA_CtrlBlkRead(btlData.controlBlock, ctrlBlkSize) == true)
            {
                btlData.controlBlock->blockUpdated = 0U;
                btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].imageType = OTA_CB_IMAGETYPE_ACTIVE;
                btlData.controlBlock->appImageInfo[btlData.controlBlock->ActiveImageNum].status = OTA_CB_STATUS_VALID;

                (void)bootloader_OTA_CtrlBlkWrite(btlData.controlBlock, ctrlBlkSize);
            }

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
