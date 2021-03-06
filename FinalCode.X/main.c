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
#pragma config FWDTEN = OFF              // Watchdog Timer Enable bit (Watchdog timer always enabled)

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

#include <p33ep256mu806.h>
#include "IOConfig.c"

#define FOSC 68000000ULL
#define FCY (FOSC/2)

#define CYCLES_PER_MS ((unsigned long long )(FCY * 0.001))        //instruction cycles per millisecond
#define CYCLES_PER_US ((unsigned long long )(FCY * 0.000001))   //instruction cycles per microsecond

#define DELAY_MS(ms)  __delay32(CYCLES_PER_MS * ((unsigned long long) ms));   //__delay32 is provided by the compiler, delay some # of milliseconds
#define DELAY_US(us)  __delay32(CYCLES_PER_US * ((unsigned long long) us));    //delay some number of microseconds


#define ON 1
#define OFF 0

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>

volatile long timerCounter;
volatile long counter;

volatile int MIRPinProgress;
int health;
int mode;
int watchMode;
int huntScanTimer;
int tooClose;
int rodFound;
int locationCounter;
int oblitCounter;
int directionTurn;

char receivedchar =' ';

void configureClock()
{
    //Configure Oscillator to operate the device at 40Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 7.37M*43(2*2)=80Mhz for 7.37M input clock
     PLLFBD=41;     // M=43
     CLKDIVbits.PLLPOST=0;  // N2=2
     CLKDIVbits.PLLPRE=0;  // N1=2
     OSCTUN=0;     // Tune FRC oscillator, if FRC is used
    // Clock switch to incorporate PLL
     __builtin_write_OSCCONH(0x01);  // Initiate Clock Switch to FRC with PLL (NOSC=0b001)
     __builtin_write_OSCCONL(0x01);  // Start clock switching
     while (OSCCONbits.COSC != 0b001){ }      // Wait for Clock switch to occur

     //Wait for PLL to lock
     while(OSCCONbits.LOCK!=1) {};

}

void setupHardware(){

    //Motion LED
    TRISCbits.TRISC14 = 0;
    //Mode LED
    TRISBbits.TRISB8 = 0;
    TRISBbits.TRISB9 = 0;
    ANSELBbits.ANSB8 = 0;
    ANSELBbits.ANSB9 = 0;

    //Attack LED
    TRISCbits.TRISC13 = 0;


    //Collision Detection
    TRISBbits.TRISB10 = 1;
    ANSELBbits.ANSB10 = 0;
    //Motion Detection
    TRISEbits.TRISE6 = 1;
    ANSELEbits.ANSE6 = 0;
    CNPDEbits.CNPDE6 = 1;


    //MIRP Stuff
    //MIRP Enable
    TRISBbits.TRISB3 = 0;
    ANSELBbits.ANSB3 = 0;
    //MIRP Carrier
    TRISBbits.TRISB4 = 0;
    ANSELBbits.ANSB4 = 0;

    //MIRP Input
    TRISEbits.TRISE5 = 1;
    ANSELEbits.ANSE5  = 0;

    //Location in
    TRISBbits.TRISB2 = 1;
    ANSELBbits.ANSB2 = 0;
    LATBbits.LATB2 = 0;
    CNPUBbits.CNPUB2 = 1;

    //HBridge Setup
    TRISGbits.TRISG6 = 0;
    TRISGbits.TRISG7 = 0;
    TRISGbits.TRISG8 = 0;
    TRISGbits.TRISG9 = 0;
    LATGbits.LATG6 = 0;
    LATGbits.LATG7 = 0;
    LATGbits.LATG8 = 0;
    LATGbits.LATG9 = 0;

    //Health Display Setup
    TRISEbits.TRISE4 = 0;
    TRISEbits.TRISE3 = 0;
    TRISEbits.TRISE2 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE0 = 0;
    //Turn off analog on health display
    ANSELEbits.ANSE4  = 0;
    ANSELEbits.ANSE3  = 0;
    ANSELEbits.ANSE2  = 0;
    ANSELEbits.ANSE1  = 0;
    ANSELEbits.ANSE0  = 0;

    //Setup Sound Pins
    TRISBbits.TRISB12 = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB15 = 0;

    LATBbits.LATB12 = 1;
    LATBbits.LATB13 = 1;
    LATBbits.LATB14 = 1;
    LATBbits.LATB15 = 1;

}

void enableGlobalInterrupts()
{
    INTCON2bits.GIE = 1;
    IEC0bits.T1IE = 1;
    IFS0bits.T1IF = 0;


    IEC0bits.T2IE = 1;
    IFS0bits.T2IF = 0;

    IEC1bits.CNIE = 1;
    IFS1bits.CNIF = 0;

    //MIRP Interrupt
    CNENEbits.CNIEE5 = 1;

    //Collision Interrupt
    CNENBbits.CNIEB10 = 1;
}

void setupCarrierTimer()
{
    T1CONbits.TCKPS = 0;
    T1CONbits.TCS = 0;
    T1CONbits.TGATE = 0;
    PR1 = 353;
    T1CONbits.TON = 1;
}

int main()
{
    configureClock();
    setupHardware();
    enableGlobalInterrupts();
    setupCarrierTimer();
    health = 81;
    updateHealth();
    stop();
    mode = startup;
    InitUART2();

    locationCounter = 0;
    tooClose = 0;

   
    while (1){


        if(health > 0){
            if(mode == watchdog){

                CNENBbits.CNIEB10 = 0;

                if(watchMode == scan){
                    if(motionSensor == 1){
                        motionLED = 1;
                        playSound(motionSound);
                        DELAY_MS(1300);
                        playSound(obliterateSound);
                        motionLED = 0;
                        attackLED = 1;
                        obliterate();
                        DELAY_MS(300);
                        attackLED = 0;
                        huntScanTimer = 5000;
                    }
                    huntScanTimer+=3;
                } else {

                    startLeftTurnWatchdog();
                    huntScanTimer+=13;
                }

                DELAY_MS(1);

                if(huntScanTimer >= 5000){

                    stop();
                    huntScanTimer = 0;
                    watchMode = !watchMode;
                    DELAY_MS(1500);
                }


            } else if(mode == follower){

                CNENBbits.CNIEB10 = 1;
                if(tooClose == 0){

                    while(locationCounter <= 400){

                        if(locFinder == 0){

                            rodFound = 1;
                            break;
                        } else {

                            rodFound = 0;
                        }

                        locationCounter++;
                        DELAY_US(5);
                    }

                    locationCounter = 0;

                    if(rodFound == 1){

                        forward();
                        DELAY_MS(500);
                        directionTurn = !directionTurn;
                    } else {

                        if(directionTurn == 1){
                            stop();
                            startRightTurn();
                        }else {
                            stop();
                            startLeftTurn();
                        }
                    }

                }else{

                    stop();
                }
            }
        } else {

            stop();
            attackLED = !attackLED;
            DELAY_MS(100);
        }
    }
}



void updateHealth(){

    if(health > 80){
        LED1 = ON;
        LED2 = ON;
        LED3 = ON;
        LED4 = ON;
        LED5 = ON;
    } else if((health > 60) && (health <= 80)){
        LED1 = ON;
        LED2 = ON;
        LED3 = ON;
        LED4 = ON;
        LED5 = OFF;
    } else if((health > 40 ) && (health <= 60)){

        LED1 = ON;
        LED2 = ON;
        LED3 = ON;
        LED4 = OFF;
        LED5 = OFF;
    } else if((health > 20) &&  (health <= 40)){

        LED1 = ON;
        LED2 = ON;
        LED3 = OFF;
        LED4 = OFF;
        LED5 = OFF;
    } else if((health > 0) &&  (health <= 20)){
        LED1 = ON;
        LED2 = OFF;
        LED3 = OFF;
        LED4 = OFF;
        LED5 = OFF;
    } else if(health < 1){

        LED1 = OFF;
        LED2 = OFF;
        LED3 = OFF;
        LED4 = OFF;
        LED5 = OFF;
    }

}

void stop(){

    
    HB_In1 = 0;
    HB_In2 = 0;
    HB_In3 = 0;
    HB_In4 = 0;
}

void forward(){


    HB_In1 = 1;
    HB_In2 = 0;
    HB_In3 = 1;
    HB_In4 = 0;

}

void reverse(){

    HB_In1 = 0;
    HB_In2 = 1;
    HB_In3 = 0;
    HB_In4 = 1;

}

void startRightTurn(){

    if(mode == follower){
        HB_In1 = 0;
        HB_In2 = 1;
        HB_In3 = 1;
        HB_In4 = 0;
    } else {

        HB_In1 = 0;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 0;
    }

}

void startLeftTurn(){

    if(mode == follower){
        HB_In1 = 1;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 1;
    } else {

        HB_In1 = 0;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 0;
    }

}

void startLeftTurnWatchdog(){


        HB_In1 = 1;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 1;

    if(mode == watchdog){
        HB_In1 = 1;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 1;
    } else {

        HB_In1 = 0;
        HB_In2 = 0;
        HB_In3 = 0;
        HB_In4 = 0;
    }

}

void sendDamageBurst()
{
    //to send damage
    CNENEbits.CNIEE5 = 0;
    DELAY_US(100);
    //we need to hold the line high for 10 cycles, then low for 140
    LED_en = 1;
    DELAY_US(180);
    LED_en = 0;
    DELAY_US(2520);

    //hold the line high for 20 cycles then low for 130
    LED_en = 1;
    DELAY_US(360);
    LED_en = 0;
    DELAY_US(2340);

    //hold the line high for 20 cycles then low for 130
    LED_en = 1;
    DELAY_US(360);
    LED_en = 0;
    DELAY_US(2340);

    //hold the line low for 150
    LED_en = 0;
    DELAY_US(2700);
    LED_en = 0;

    DELAY_US(100);
    CNENEbits.CNIEE5 = 1;


}

void obliterate(){

    oblitCounter = 0;
    while (oblitCounter < 100){
        
        oblitCounter++;
        sendDamageBurst();
        DELAY_US(200);
    }
}

void playSound(int soundToPlay)
{

    if(soundToPlay == followerSound)
    {
        followerSoundPin = 0;
        DELAY_MS(150);
        followerSoundPin = 1;
    }
    if(soundToPlay == watchdogSound)
    {
        stop();
        watchdogSoundPin = 0;
        DELAY_MS(150);
        watchdogSoundPin = 1;
        DELAY_MS(1000);
    }
    if(soundToPlay == motionSound)
    {
        motionSoundPin = 0;
        DELAY_MS(150);
        motionSoundPin = 1;
    }
    if(soundToPlay == obliterateSound)
    {
        obliterateSoundPin = 0;
        DELAY_MS(150);
        obliterateSoundPin = 1;
    }
}

void avoidCollision(){

    stop();
    DELAY_MS(200);
    reverse();
    DELAY_MS(1000);
    stop();
    DELAY_MS(200);
    startRightTurn();
    DELAY_MS(1250);
    stop();
    DELAY_MS(200);
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{

    LED_ca = !LED_ca;
    _T1IF = 0;
    // Clear Timer 1 interrupt flag
}

void __attribute__((__interrupt__, __auto_psv__)) _CNInterrupt(void)
{
    
    INTCON2bits.GIE = 0;

    long pulseWidth1;
    long pulseWidth2;
    long pulseWidth3;
    long pulseWidth4;

    long overflowCount = 0;

    pulseWidth1 = 0;
    pulseWidth2 = 0;
    pulseWidth3 = 0;
    pulseWidth4 = 0;

    if(collisionSensor == 0){

        avoidCollision();
    }


    if(MIRPin == 0){
        //first pulse
        while (MIRPin == 0)
        {
            //tick away man
            pulseWidth1++;
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 1 || overflowCount > 3000)
            {
                break;
            }
        }
        overflowCount = 0;

        while (MIRPin == 1)
        {
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 0 || overflowCount > 3000)
            {
                break;
            }

            //twiddle thumb
            //DELAY_US(1);
        }


        overflowCount = 0;

        //second pulse
        while (MIRPin == 0)
        {
            //tick away man
            pulseWidth2++;
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 1 || overflowCount > 3000)
            {
                break;
            }
        }

        overflowCount = 0;
        while (MIRPin == 1)
        {
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 0 || overflowCount > 3000)
            {
                break;
            }
        }

        overflowCount = 0;

        //third pulse
        while (MIRPin == 0)
        {
            //tick away man
            pulseWidth3++;
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 1 || overflowCount > 3000)
            {
                break;
            }
        }

        overflowCount = 0;
        while (MIRPin == 1)
        {
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 0 || overflowCount > 3000)
            {
                break;
            }
        }

        overflowCount = 0;

        //fourth pulse
        while (MIRPin == 0)
        {
            //tick away man
            pulseWidth4++;
            overflowCount++;
            DELAY_US(1);
            if(MIRPin == 1 || overflowCount > 3000)
            {
                break;
            }
        }

        overflowCount = 0;

        if (pulseWidth1 < 350) //start
        {

            
            if (pulseWidth2 > 180 && pulseWidth2 < 280) //data
            {


                health = health - 1;
                    updateHealth();

                
            } else if (pulseWidth2 < 380){

                health = health + 1;
                updateHealth();
            }

        }
    }

    _CNIF = 0;
    INTCON2bits.GIE = 1;

}

void watchSwitch(){

    mode = watchdog;
    stop();
    oblitCounter = 100;
    followModeLED = 0;
    DELAY_MS(3000);
    huntScanTimer = 0;
    locationCounter = 0;
    watchMode = scan;
    stop();
    watchModeLED = 1;
    playSound(watchdogSound);
    DELAY_MS(6000);
    stop();
    
}

void followSwitch(){

    mode = follower;
    stop();
    oblitCounter = 100;
    watchModeLED = 0;
    huntScanTimer = 0;
    locationCounter = 0;
    followModeLED = 1;
    playSound(followerSound);
    DELAY_MS(3000);
    stop();
}

void __attribute__ ( (interrupt, no_auto_psv) ) _U1RXInterrupt( void )
{

    //watchModeLED = !watchModeLED;
    //followModeLED = !followModeLED;
    receivedchar = U1RXREG;
    if(receivedchar == 'w'||receivedchar == 'W'){
        //U1TXREG = 'W';
        watchSwitch();
        
    }else if(receivedchar == 'f'||receivedchar == 'F'){
        //U1TXREG  = 'F';
        followSwitch();
        
    }else if(receivedchar == 's'||receivedchar == 'S'){
        //U1TXREG = 'S';
    }else if(receivedchar == 'g'||receivedchar == 'G'){
        //U1TXREG = 'G';
    } else if(receivedchar == 'q'){
        //U1TXREG = 'q';

    }
    IFS0bits.U1RXIF = 0;
}

void __attribute__ ( (interrupt, no_auto_psv) ) _U1TXInterrupt( void )
{
    DELAY_US(10);
    IFS0bits.U1TXIF = 0;
}

void InitUART2( void )
{
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets
    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only
    // configure U1MODE
    U1MODEbits.UARTEN = 0;  // Bit15 TX, RX DISABLED, ENABLE at end of func

    //U1MODEbits.notimplemented;// Bit14
    U1MODEbits.USIDL = 0;   // Bit13 Continue in Idle
    U1MODEbits.IREN = 0;    // Bit12 No IR translation
    U1MODEbits.RTSMD = 0;   // Bit11 Simplex Mode

    //U1MODEbits.notimplemented;// Bit10
    U1MODEbits.UEN = 0;     // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0;    // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0;  // Bit6 No Loop Back
    U1MODEbits.ABAUD = 0;   // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.BRGH = 0;    // Bit3 16 clocks per bit period
    U1MODEbits.PDSEL = 0;   // Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0;   // Bit0 One Stop Bit

    // Load a value into Baud Rate Generator.  Example is for 9600.
    // See section 19.3.1 of datasheet.
    //  U1BRG = (Fcy/(16*BaudRate))-1
    //  U1BRG = (37M/(16*9600))-1
    //  U1BRG = 240
    int BaudRate = 9600;
    U1BRG = (FOSC/BaudRate)/26-1;            // 60Mhz osc, 9600 Baud

    // Load all values in for U1STA SFR
    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0;   //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15

    //U1STAbits.notimplemented = 0;//Bit12
    U1STAbits.UTXBRK = 0;   //Bit11 Disabled
    U1STAbits.UTXEN = 0;    //Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0;    //Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0;     //Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0;  //Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0;    //Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0;    //Bit4 *Read Only Bit*
    U1STAbits.PERR = 0;     //Bit3 *Read Only Bit*
    U1STAbits.FERR = 0;     //Bit2 *Read Only Bit*
    U1STAbits.OERR = 0;     //Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0;    //Bit0 *Read Only Bit*
    IPC7 = 0x4400;          // Mid Range Interrupt Priority level, no urgent reason
    IFS0bits.U1TXIF = 0;    // Clear the Transmit Interrupt Flag
    IEC0bits.U1TXIE = 1;    // Enable Transmit Interrupts
    IFS0bits.U1RXIF = 0;    // Clear the Recieve Interrupt Flag
    IEC0bits.U1RXIE = 1;    // Enable Recieve Interrupts
    RPOR1bits.RP66R = 1;    //RD2 as U1TX
    RPINR18bits.U1RXR = 73; //RD9 as U1RX
    U1MODEbits.UARTEN = 1;  // And turn the peripheral on
    U1STAbits.UTXEN = 1;
}