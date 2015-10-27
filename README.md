//main.c
//Cody Blades
//19 Oct 2015
//based on main.c by Robert Bloomfield (18 Oct 2015)
 
//Senior Design
//Watchdog / WALL-E
 
#include <stdio.h>
#include <stdlib.h>
#include <p33EP256MU806.h>
#include <xc.h>
 
#include "ioconfig.c"
//#include "I2CLib.c"
#include "delay.h"
 
 
// DSPIC33EP256MU806 Configuration Bit Settings
 
// 'C' source line config statements
 
// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GSS = OFF                // General Segment Code-Protect bit (General Segment Code protect is disabled)
#pragma config GSSK = OFF               // General Segment Key bits (General Segment Write Protection and Code Protection is Disabled)
 
// FOSCSEL
#pragma config FNOSC = FRCDIVN          // Initial Oscillator Source Selection bits (Internal Fast RC (FRC) Oscillator with postscaler)
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)
 
// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Mode Select bits (Primary Oscillator disabled)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = ON             // Peripheral pin select configuration (Allow only one reconfiguration)
#pragma config FCKSM = CSECME           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)
 
// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON          // PLL Lock Wait Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = ON              // Watchdog Timer Enable bit (Watchdog timer always enabled)
 
// FPOR
#pragma config FPWRT = PWR128           // Power-on Reset Timer Value Select bits (128ms)
#pragma config BOREN = ON               // Brown-out Reset (BOR) Detection Enable bit (BOR is enabled)
#pragma config ALTI2C1 = OFF            // Alternate I2C pins for I2C1 (SDA1/SCK1 pins are selected as the I/O pins for I2C1)
 
// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config RSTPRI = PF              // Reset Target Vector Select bit (Device will obtain reset instruction from Primary flash)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)
 
// FAS
#pragma config AWRP = OFF               // Auxiliary Segment Write-protect bit (Aux Flash may be written)
#pragma config APL = OFF                // Auxiliary Segment Code-protect bit (Aux Flash Code protect is disabled)
#pragma config APLK = OFF               // Auxiliary Segment Key bits (Aux Flash Write Protection and Code Protection is Disabled)
 
 
//global variables
int mode;
int health;
 
void configureClock() 
{
    PLLFBD = 38; /* M = 44 */
     
    CLKDIVbits.PLLPOST = 0; /* N1 = 2 */
    CLKDIVbits.PLLPRE = 0; /* N2 = 2 */
    OSCTUN = 21;
 
    /* Initiate Clock Switch to Primary
     * Oscillator with PLL (NOSC= 0x3)*/
 
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);
}
 
void setupIORegs()
{
    TRISB = 0x00; //make port b all outputs
    TRISE = 0xFF; //make port e all inputs
     
    //Set up digital input pins by disabling analog function
    ANSELGbits.ANSG6 = 0;
    ANSELEbits.ANSE7 = 0;
    ANSELEbits.ANSE6 = 0;
    ANSELEbits.ANSE5 = 0;
    ANSELEbits.ANSE4 = 0;
    ANSELEbits.ANSE1 = 0;
    ANSELEbits.ANSE2 = 0;
    ANSELEbits.ANSE3 = 0;
    ANSELEbits.ANSE0 = 0;
     
    //latch b
    LATB = 0x00;
}
 
void initOutputs()
{
    health = 100;
    mode = 1;
     
    followerMode_LED = OFF;
    watchdogMode_LED = ON;
 
    //sound pins are active-LOW, ON by default
    soundPin1 = ON;
    soundPin2 = ON;
    soundPin3 = ON;
    soundPin4 = ON;
  
    //updateDisplay();
}
 
void toggleMode()
{
    if(mode == 0)       //follower mode
    {
        mode = 1;       //switch to watchdog
        followerMode_LED = OFF;
        watchdogMode_LED = ON;
    }
    else //watchdog mode
    {
        mode = 0;       //switch to follower
        followerMode_LED = ON;
        watchdogMode_LED = OFF;
    }
}
 
void increaseHealth()
{
    health = health + 1;
}
 
void decreaseHealth()
{
    health = health - 1;
}
 
void playSound(int n)
{
    if(n == 1)
    {
        soundPin1 = 0;
        DELAY_MS(200);
        soundPin1 = 1;
    }
    if(n == 2)
    {
        soundPin2 = 0;
        DELAY_MS(200);
        soundPin2 = 1;
    }
    if(n == 3)
    {
        soundPin3 = 0;
        DELAY_MS(200);
        soundPin3 = 1;
    }
}
 
void updateDisplay()
{
    int i = 1;
    for(i; i<256; i++)
    {
        I2C_start();
        I2C_write(sevenSeg1);
        I2C_write(i);
        I2C_stop();
    }
}
 
int main(int argc, char** argv)
{
    configureClock();
    setupIORegs();
    I2C_init();
    initOutputs();
     
    while(1)
    {
        if(mode_button = OFF)       //button happens to be wired normally on
        {
            toggleMode();
            DELAY_MS(2000);
        }
 
        if(increaseHealth_button = OFF)
        {
            //increaseHealth();
            //updateDisplay();
            soundPin1 = OFF;
            DELAY_MS(2000);
            soundPin1 = ON;
        }
 
        if(decreaseHealth_button = OFF)
        {
            decreaseHealth();
            updateDisplay();
            DELAY_MS(2000);
        }
    }
     
    /*
    playSound(1);
    playSound(2);
    playSound(3);
    */
     
    return (EXIT_SUCCESS);
}
