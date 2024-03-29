/*******************************************************************************
  OTA service File Handler Source File

  File Name:
    ota_service_file_handler.c

  Summary:
    This file contains source code necessary to handle the image and it's metadata.

  Description:
    This file contains source code necessary to handle the image and it's metadata.
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

#include "ota_service_file_handler.h"
#include "ota_service.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

static uint8_t CACHE_ALIGN controlBlockBuffer[OTA_CONTROL_BLOCK_BUFFER_SIZE];
static uint8_t CACHE_ALIGN flash_data[DATA_SIZE];
static OTA_FILE_HANDLER_CONTEXT otaFileHandlerCtx;
static uint32_t ctrlBlkSize = OTA_CONTROL_BLOCK_BUFFER_SIZE;
static OTA_FILE_HANDLER_DATA otaFileHandlerData =
{
    .state = OTA_SERVICE_FH_STATE_INIT,
    .controlBlock = (OTA_CONTROL_BLOCK *)controlBlockBuffer
};

// *****************************************************************************
// *****************************************************************************
// Section: File Handler Local Functions
// *****************************************************************************
// *****************************************************************************

/* Function to Generate CRC using the device service unit peripheral on programmed data */
static uint32_t OTA_SERVICE_FH_CRCGenerate(uint32_t start_addr, uint32_t size)
{
    uint32_t crc  = 0xFFFFFFFFU;

    PAC_PeripheralProtectSetup (PAC_PERIPHERAL_DSU, PAC_PROTECTION_CLEAR);

    (void)DSU_CRCCalculate (start_addr, size, crc, &crc);

    PAC_PeripheralProtectSetup (PAC_PERIPHERAL_DSU, PAC_PROTECTION_SET);

    return crc;
}


// *****************************************************************************
// *****************************************************************************
// Section: File Handler Global Functions
// *****************************************************************************
// *****************************************************************************

/* Trigger a reset */
void OTA_SERVICE_FH_TriggerReset(void)
{
    NVIC_SystemReset();
}

bool OTA_SERVICE_FH_CtrlBlkRead(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        appMetaDataAddress  = (FLASH_END_ADDRESS - ctrlBlkSize);
        (void)NVMCTRL_Read((void *)ptrBuffer, ctrlBlkSize, appMetaDataAddress);
        status = true;
    }

    return status;
}

bool OTA_SERVICE_FH_CtrlBlkWrite(OTA_CONTROL_BLOCK *controlBlock, uint32_t length)
{
    uint32_t count;
    uint32_t appMetaDataAddress;
    bool     status = false;
    uint8_t  *ptrBuffer = (uint8_t *)controlBlock;

    if ((ptrBuffer != NULL) && (length == ctrlBlkSize))
    {
        appMetaDataAddress  = (FLASH_END_ADDRESS - ctrlBlkSize);

        // Lock region size is always bigger than the row size
        NVMCTRL_RegionUnlock(appMetaDataAddress);

        while(NVMCTRL_IsBusy() == true)
        {

        }

        /* Erase the Current sector */
        (void)NVMCTRL_BlockErase(appMetaDataAddress);

        while(NVMCTRL_IsBusy() == true)
        {

        }

        for (count = 0; count < length; count += OTA_CONTROL_BLOCK_PAGE_SIZE)
        {
            (void)NVMCTRL_PageWrite((void *)ptrBuffer, appMetaDataAddress);

            while(NVMCTRL_IsBusy() == true)
            {

            }
            appMetaDataAddress += OTA_CONTROL_BLOCK_PAGE_SIZE;
            ptrBuffer += OTA_CONTROL_BLOCK_PAGE_SIZE;
        }

        status = true;
    }

    return status;
}

OTA_SERVICE_FH_STATE OTA_SERVICE_FH_StateGet(void)
{
    return otaFileHandlerData.state;
}

void OTA_SERVICE_FH_Tasks(void)
{
    /* Check the application's current state. */
    switch (otaFileHandlerData.state)
    {
        case OTA_SERVICE_FH_STATE_INIT:
        {
            otaFileHandlerData.nFlashBytesWritten = 0U;
            otaFileHandlerData.nFlashBytesFreeSpace = DATA_SIZE;
            otaFileHandlerData.totalBytesWritten = 0U;
            otaFileHandlerData.controlBlock->ActiveImageNum = 0U;
            otaFileHandlerCtx.fileHeader = &otaFileHandlerData.fileHeader;
            otaFileHandlerData.state = OTA_SERVICE_FH_STATE_MSG_RECV;
            break;
        }

        case OTA_SERVICE_FH_STATE_MSG_RECV:
        {
            uint32_t receivedBytes;

            otaFileHandlerCtx.buffer = &flash_data[otaFileHandlerData.nFlashBytesWritten];
            otaFileHandlerCtx.size = otaFileHandlerData.nFlashBytesFreeSpace;
            receivedBytes = OTA_SERVICE_Transport_FHMsgReceive(&otaFileHandlerCtx);

            if (receivedBytes != 0U)
            {
                otaFileHandlerData.nFlashBytesWritten += receivedBytes;
                otaFileHandlerData.nFlashBytesFreeSpace -= receivedBytes;

                if ((otaFileHandlerData.nFlashBytesWritten >= DATA_SIZE)
                ||  ((otaFileHandlerData.initData == true) && ((otaFileHandlerData.nFlashBytesWritten + otaFileHandlerData.totalBytesWritten) >= otaFileHandlerData.fileHeader.imageSize)))
                {
                    if (otaFileHandlerData.initData == false)
                    {
                        otaFileHandlerData.initData = true;
                        otaFileHandlerData.memoryAddress = otaFileHandlerData.fileHeader.programAddress;
                    }

                    if (0U == (otaFileHandlerData.memoryAddress % ERASE_BLOCK_SIZE))
                    {
                        otaFileHandlerData.state = OTA_SERVICE_FH_STATE_ERASE;
                    }
                    else
                    {
                        otaFileHandlerData.state = OTA_SERVICE_FH_STATE_WRITE;
                    }
                }
            }
            break;
        }

        case OTA_SERVICE_FH_STATE_ERASE:
        {
            // Lock region size is always bigger than the row size
            NVMCTRL_RegionUnlock(otaFileHandlerData.memoryAddress);

            while(NVMCTRL_IsBusy() == true)
            {

            }

            /* Erase the Current sector */
            (void)NVMCTRL_BlockErase(otaFileHandlerData.memoryAddress);

            otaFileHandlerData.state = OTA_SERVICE_FH_STATE_ERASE_WAIT;

            break;
        }

        case OTA_SERVICE_FH_STATE_WRITE:
        {
            (void)NVMCTRL_PageWrite((void *)&flash_data[0], otaFileHandlerData.memoryAddress);
            otaFileHandlerData.nFlashBytesWritten = 0U;
            otaFileHandlerData.nFlashBytesFreeSpace = DATA_SIZE;
            otaFileHandlerData.memoryAddress += DATA_SIZE;
            otaFileHandlerData.totalBytesWritten += DATA_SIZE;
            otaFileHandlerData.state = OTA_SERVICE_FH_STATE_WRITE_WAIT;
            break;
        }

        case OTA_SERVICE_FH_STATE_ERASE_WAIT:
        case OTA_SERVICE_FH_STATE_WRITE_WAIT:
        {
            if (NVMCTRL_IsBusy() == false)
            {
                if (otaFileHandlerData.state == OTA_SERVICE_FH_STATE_ERASE_WAIT)
                {
                    otaFileHandlerData.state = OTA_SERVICE_FH_STATE_WRITE;
                }
                else
                {
                    if (otaFileHandlerData.totalBytesWritten >= otaFileHandlerData.fileHeader.imageSize)
                    {
                        otaFileHandlerData.state = OTA_SERVICE_FH_STATE_VERIFY;
                    }
                    else
                    {
                        otaFileHandlerData.state = OTA_SERVICE_FH_STATE_MSG_RECV;
                    }
                }
            }
            break;
        }

        case OTA_SERVICE_FH_STATE_VERIFY:
        {
            uint32_t crc32;
            uint32_t *received_crc32 = (void *)otaFileHandlerData.fileHeader.signature;

            crc32 = OTA_SERVICE_FH_CRCGenerate(otaFileHandlerData.fileHeader.programAddress, otaFileHandlerData.fileHeader.imageSize);

            if (crc32 == *received_crc32)
            {
                otaFileHandlerData.state = OTA_SERVICE_FH_STATE_CB_WRITE;
            }
            else
            {
                otaFileHandlerData.state = OTA_SERVICE_FH_STATE_IDLE;
            }
            break;
        }

        case OTA_SERVICE_FH_STATE_CB_WRITE:
        {
            otaFileHandlerData.controlBlock->ActiveImageNum = 0U;
            otaFileHandlerData.controlBlock->versionNum = 0U;
            otaFileHandlerData.controlBlock->blockUpdated = 1U;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].imageType = otaFileHandlerData.fileHeader.imageType;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].status = otaFileHandlerData.fileHeader.status;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].programAddress = otaFileHandlerData.fileHeader.programAddress;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].jumpAddress = otaFileHandlerData.fileHeader.jumpAddress;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].loadAddress = otaFileHandlerData.fileHeader.loadAddress;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].imageSize = otaFileHandlerData.fileHeader.imageSize;
            otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].signaturePresent = 1U;
            (void)memcpy(otaFileHandlerData.controlBlock->appImageInfo[otaFileHandlerData.controlBlock->ActiveImageNum].signature, otaFileHandlerData.fileHeader.signature, sizeof(otaFileHandlerData.fileHeader.signature));

            if (OTA_SERVICE_FH_CtrlBlkWrite(otaFileHandlerData.controlBlock, ctrlBlkSize) == false)
            {
                otaFileHandlerData.state = OTA_SERVICE_FH_STATE_ERROR;
            }
            else
            {
                otaFileHandlerData.state = OTA_SERVICE_FH_STATE_IDLE;
            }
            break;
        }

        case OTA_SERVICE_FH_STATE_IDLE:
        {
            break;
        }

        case OTA_SERVICE_FH_STATE_ERROR:
        default:
        {
            // Do nothing
            break;
        }
    }
}
