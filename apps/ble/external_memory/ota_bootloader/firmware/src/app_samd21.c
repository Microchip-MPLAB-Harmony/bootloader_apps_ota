/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_samd21.c

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

#include "app_samd21.h"
#include "definitions.h"
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
    This structure should be initialized by the APP_SAMD21_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_SAMD21_DATA app_samd21Data;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* Fill this function to release or De-Initialize all the resources used 
 * It will be called by bootloader before jumping into application
 */
void SYS_DeInitialize( void)
{
    /* Reset SERCOM SPI peripheral as it will Re-Initialized in application space */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_SWRST_Msk;
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_SAMD21_Initialize ( void )

  Remarks:
    See prototype in app_samd21.h.
 */

void APP_SAMD21_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_samd21Data.state = APP_SAMD21_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_SAMD21_Tasks ( void )

  Remarks:
    See prototype in app_samd21.h.
 */

void APP_SAMD21_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_samd21Data.state )
    {
        /* Application's initial state. */
        case APP_SAMD21_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                app_samd21Data.state = APP_SAMD21_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_SAMD21_STATE_SERVICE_TASKS:
        {

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
