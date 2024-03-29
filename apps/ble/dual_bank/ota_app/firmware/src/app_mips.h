/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
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

/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_mips.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_MIPS_Initialize" and "APP_MIPS_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_MIPS_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_MIPS_H
#define _APP_MIPS_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_MIPS_STATE_INIT=0,
    APP_MIPS_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_MIPS_STATES;

// *****************************************************************************

/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
 */

typedef enum {
    /* state machine's initial state. */
    SYS_OTA_REGOTACALLBCK = 0,

    /*Check OTA update available */
    SYS_OTA_UPDATE_CHECK,

    /*Wait for OTA complete*/
    SYS_OTA_WAIT_FOR_OTA_COMPLETE,

    /*To trigger rollback*/
    SYS_OTA_ROLLBACK,

    /*To trigger rollback*/
    SYS_OTA_FIRMWARE_SWITCH,

    /*to update user about current service state*/
    SYS_OTA_UPDATE_USER,

    /*For resetting system on user trigger*/
    SYS_OTA_SYSTEM_RESET,

    /*Service Idle state*/
    SYS_OTA_STATE_IDLE,

    /* TODO: Define states used by the application state machine. */

} SYS_OTA_STATES;

    // *****************************************************************************

/* System ota service status types

  Summary:
    provide status of ota system.

  Description:
    provide status of ota system.

  Remarks:

 */
typedef enum {
    /* To provide status of OTA system */

    /*system in idle state*/
    SYS_OTA_IDLE = 0,

    /*waiting for network connection*/        
    SYS_OTA_WAITING_FOR_NETWORK_CONNECTION,    

    /*waiting for OTA core to become IDLE*/        
    SYS_OTA_WAITING_FOR_OTACORE_IDLE,        

    /*waiting for user defined period*/        
    SYS_OTA_WAITING_FOR_USER_DEFINED_PERIOD,        

    /*update check with server start*/
    SYS_OTA_UPDATE_CHECK_START,

    /*update check with server failed*/        
    SYS_OTA_UPDATE_CHECK_FAILED,

    /*update available in server*/
    SYS_OTA_UPDATE_AVAILABLE,

    /*update not available*/
    SYS_OTA_UPDATE_NOTAVAILABLE,

    /*OTA trigger failed*/
    SYS_OTA_TRIGGER_OTA_FAILED,

    /*rollback success*/
    SYS_OTA_ROLLBACK_SUCCESS,

    /*rollback failed*/
    SYS_OTA_ROLLBACK_FAILED,

    /*download start*/
    SYS_OTA_DOWNLOAD_START,

    /*download success*/
    SYS_OTA_DOWNLOAD_SUCCESS,

    /*download failed*/
    SYS_OTA_DOWNLOAD_FAILED,

    /*firmware switch start*/        
    SYS_OTA_FIRMWARE_SWITCH_START,        

    /*firmware switch success*/        
    SYS_OTA_FIRMWARE_SWITCH_SUCCESS,

    /*firmware switch failed*/        
    SYS_OTA_FIRMWARE_SWITCH_FAILED,        

    /*not a defined(as mentioned above) status*/
    SYS_OTA_NONE
} SYS_OTA_STATUS;
// *****************************************************************************

/* system service Data

  Summary:
    system service data

  Description:
    This structure holds the system service's data.

  Remarks:

 */

typedef struct {
    /* service current state */
    SYS_OTA_STATES state;

    /*keep track if device connected to Transport layer*/
    bool dev_cnctd;

    /*to keep track if OTA process is in progress*/
    bool otaFwInProgress;

    /*to check if OTA update available or not*/
    bool otaUpdateCheckavailable;

    /*to keep track if image download success*/
    bool download_success;

    /*control interface result*/
    SYS_OTA_STATUS ota_srvc_status;

} SYS_OTA_DATA;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_MIPS_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_MIPS_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

typedef void (*SYS_OTA_CALLBACK)(uint32_t event, void * data, void *cookie);

void app_ota_task(void);

void SYS_OTA_Initialize(void) ;

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_MIPS_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_MIPS_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_MIPS_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_MIPS_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_MIPS_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_MIPS_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_MIPS_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_MIPS_H */

/*******************************************************************************
 End of File
 */

