/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_same54.c

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

#include "app_same54.h"
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
    This structure should be initialized by the APP_SAME54_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_SAME54_DATA app_same54Data;

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


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

static void CLOCK_DeInitialize(void)
{
    // Enable DFLL
    OSCCTRL_REGS->OSCCTRL_DFLLCTRLA = OSCCTRL_DFLLCTRLA_ENABLE_Msk ;
    // Configure CPU to run from DFLL Clock
    MCLK_REGS->MCLK_CPUDIV = MCLK_CPUDIV_DIV(0x01);
    while((MCLK_REGS->MCLK_INTFLAG & MCLK_INTFLAG_CKRDY_Msk) != MCLK_INTFLAG_CKRDY_Msk)
    {
        /* Wait for the Main Clock to be Ready */
    }
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC(6) | GCLK_GENCTRL_GENEN_Msk;
    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL_GCLK0) == GCLK_SYNCBUSY_GENCTRL_GCLK0)
    {
        /* wait for the Generator 0 synchronization */
    }
    /* Disable FDPLL0 */
    OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLA &= (~OSCCTRL_DPLLCTRLA_ENABLE_Msk)   ;
    while((OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSYNCBUSY & OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk) == OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk )
    {
        /* Waiting for the DPLL enable synchronization */
    }
    /* Disable FDPLL1 */
    OSCCTRL_REGS->DPLL[1].OSCCTRL_DPLLCTRLA &= (~OSCCTRL_DPLLCTRLA_ENABLE_Msk)   ;
    while((OSCCTRL_REGS->DPLL[1].OSCCTRL_DPLLSYNCBUSY & OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk) == OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk )
    {
        /* Waiting for the DPLL enable synchronization */
    }
}

/* Fill this function to release or De-Initialize all the resources used 
 * It will be called by bootloader before jumping into application
 */
void SYS_DeInitialize(void *data)
{
    /* Reset CLOCK as it will Re-Initialized in application space */
    CLOCK_DeInitialize();
}

/*******************************************************************************
  Function:
    void APP_SAME54_Initialize ( void )

  Remarks:
    See prototype in app_same54.h.
 */

void APP_SAME54_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_same54Data.state = APP_SAME54_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_SAME54_Tasks ( void )

  Remarks:
    See prototype in app_same54.h.
 */

void APP_SAME54_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_same54Data.state )
    {
        /* Application's initial state. */
        case APP_SAME54_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                app_same54Data.state = APP_SAME54_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_SAME54_STATE_SERVICE_TASKS:
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
