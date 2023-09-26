/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "definitions.h"
#include "ota_service/ota_service.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;
static uint8_t led_toggle_count = 0;

/*pointer to register user callback function*/
static SYS_OTA_CALLBACK g_otaSrvcCallBack;

/*variable for ota service parameters*/
SYS_OTA_DATA sys_otaData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

bool SYS_OTA_Update_Check(void) {

    /*implement the logic here*/

    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section:  OTA service result 
// *****************************************************************************
// *****************************************************************************

static inline void SYS_OTA_SetOtaServicStatus(SYS_OTA_STATUS status) {
    sys_otaData.ota_srvc_status = status;
}
// *****************************************************************************
// *****************************************************************************
// Section: To check if update available
// *****************************************************************************
// *****************************************************************************

static inline bool SYS_OTA_IsUpdateCheckavailable(void) {
    return sys_otaData.otaUpdateCheckavailable;
}
// *****************************************************************************
// *****************************************************************************
// Section: To check system is connected to network or not
// *****************************************************************************
// *****************************************************************************

static inline bool SYS_OTA_ConnectedToNtwrk(void) {
    return sys_otaData.dev_cnctd;
}
// *****************************************************************************
// *****************************************************************************
// Section: To check if OTA process already in progress
// *****************************************************************************
// *****************************************************************************

static inline bool SYS_OTA_IsOtaInProgress(void) {
    return sys_otaData.otaFwInProgress;
}

// *****************************************************************************
// *****************************************************************************
// Section: To check if image download success
// *****************************************************************************
// *****************************************************************************

static inline bool SYS_OTA_IsDownloadSuccess(void) {
    return sys_otaData.download_success;
}

void SYS_OTA_Initialize(void) {

    memset(&sys_otaData, 0, sizeof (sys_otaData));
    sys_otaData.state = SYS_OTA_REGOTACALLBCK;
    sys_otaData.otaUpdateCheckavailable = true;

}


// *****************************************************************************
// *****************************************************************************
// Section: Call back function to OTA core
// *****************************************************************************
// *****************************************************************************

void OTACallback(uint32_t event, void * data, void *cookies) {

    switch (event) {

        case OTA_RESULT_IMAGE_DOWNLOAD_START:
        {

            SYS_OTA_SetOtaServicStatus(SYS_OTA_DOWNLOAD_START);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_IMAGE_DOWNLOAD_SUCCESS:
        {
            sys_otaData.download_success = true;
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            SYS_OTA_SetOtaServicStatus(SYS_OTA_DOWNLOAD_SUCCESS);
            break;
        }
        case OTA_RESULT_IMAGE_DOWNLOAD_FAILED:
        {
            sys_otaData.otaFwInProgress = false;
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            SYS_OTA_SetOtaServicStatus(SYS_OTA_DOWNLOAD_FAILED);
            break;
        }

        case OTA_RESULT_OTA_UPDATE_AVAILABLE:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_UPDATE_AVAILABLE);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_OTA_UPDATE_NOT_AVAILABLE:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_UPDATE_NOTAVAILABLE);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_SWITCH_FIRMWARE_SUCCESS:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_FIRMWARE_SWITCH_SUCCESS);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_SWITCH_FIRMWARE_FAILED:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_FIRMWARE_SWITCH_FAILED);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_ROLLBACK_SUCCESS:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_ROLLBACK_SUCCESS);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
        case OTA_RESULT_ROLLBACK_FAILED:
        {
            SYS_OTA_SetOtaServicStatus(SYS_OTA_ROLLBACK_FAILED);
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            break;
        }
    }
}

void app_ota_task(void) {

    switch (sys_otaData.state) {

        case SYS_OTA_REGOTACALLBCK:
        {
            /* Register OTA callback here */
            if (OTA_CallBackReg(OTACallback, sizeof (uint8_t *)) == SYS_STATUS_ERROR) {
                printf("SYS OTA : OTA callback register failed \r\n");
            } else {
                sys_otaData.state = SYS_OTA_UPDATE_CHECK;
            }
            break;
        }
        case SYS_OTA_UPDATE_CHECK:
        {
            SYS_STATUS status;
            /*implement a logic inside OTA_updateCheck() : button press  */

            /*To check if New update available*/
            if (SYS_OTA_Update_Check()) {
                /*provide callback to user about update availability*/               
                SYS_OTA_SetOtaServicStatus(SYS_OTA_UPDATE_AVAILABLE);
                sys_otaData.state = SYS_OTA_WAIT_FOR_OTA_COMPLETE;
                status = OTA_SERVICE_OTA_Start();
                sys_otaData.otaFwInProgress = true;
                if (status == SYS_STATUS_ERROR) {
                    printf("SYS OTA : Error starting OTA\r\n");
                    SYS_OTA_SetOtaServicStatus(SYS_OTA_TRIGGER_OTA_FAILED);
                    sys_otaData.state = SYS_OTA_UPDATE_USER;
                }

            }
            break;
        }
        case SYS_OTA_UPDATE_USER:
        {
            sys_otaData.state = SYS_OTA_STATE_IDLE;

            if ((sys_otaData.ota_srvc_status == SYS_OTA_DOWNLOAD_START) || (sys_otaData.ota_srvc_status == SYS_OTA_DOWNLOAD_SUCCESS))
                sys_otaData.state = SYS_OTA_WAIT_FOR_OTA_COMPLETE;

            if (sys_otaData.ota_srvc_status != SYS_OTA_NONE) {
                if (g_otaSrvcCallBack != NULL)
                    g_otaSrvcCallBack(sys_otaData.ota_srvc_status, NULL, NULL);
            }

            /*If image downloaded successfully*/
            if (SYS_OTA_IsDownloadSuccess() == true)
                sys_otaData.state = SYS_OTA_SYSTEM_RESET;

            break;
        }

        case SYS_OTA_ROLLBACK:
        {

            break;
        }
        case SYS_OTA_FIRMWARE_SWITCH:
        {

            break;
        }

        case SYS_OTA_STATE_IDLE:
        {
            /*OTA system in idle state*/
            SYS_OTA_SetOtaServicStatus(SYS_OTA_IDLE);
            break;
        }
        case SYS_OTA_WAIT_FOR_OTA_COMPLETE:
        {
            sys_otaData.state = SYS_OTA_UPDATE_USER;
            if (sys_otaData.download_success == true) {
                sys_otaData.state = SYS_OTA_SYSTEM_RESET;
            }
            /*wait for OTA complete*/
            break;
        }
        case SYS_OTA_SYSTEM_RESET:
        {
            if (SYS_OTA_IsDownloadSuccess() == true) {
                OTA_SERVICE_FH_TriggerReset();
            } else {
                printf("SYS OTA : OTA download Failed\n\r");
                sys_otaData.state = SYS_OTA_STATE_IDLE;
            }
            break;
        }
        default:
        {

            break;
        }
    }
}

void timeout_handler(uintptr_t context)
{
    led_toggle_count++;
    if (led_toggle_count > 5U)
    {
        led_toggle_count = 0;
	    LED_TOGGLE();
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;

            SYS_OTA_Initialize();
            
            SYSTICK_TimerCallbackSet(&timeout_handler, (uintptr_t) NULL);
            SYSTICK_TimerStart();
            
            printf("\n\r####### Application running #######\n\r");

            if (appInitialized)
            {

                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
			app_ota_task();
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */