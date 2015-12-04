// PIC16F886 Configuration Bit Settings

// 'C' source line config statements

#define _XTAL_FREQ 8000000


#include <xc.h>
#include <pic16f886.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF       // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

volatile int currentMode;

volatile unsigned int t1;
volatile unsigned int t2;
volatile unsigned int t3;
volatile unsigned int state;
volatile unsigned int sampleCounter;
volatile int risingEdge;
volatile int counter;
volatile char rx_char;

//we will be using three mode states on the control rod.
// mode 0 is idle
// mode 1 is follower mode
// mode 2 is watchdog mode

void enableInterrupts()
{


}

void configureTimer0()
{




}

void configureUART()
{
    //set up UART

    TXSTAbits.TXEN = 0x1;
    TXSTAbits.SYNC = 0x0;
    RCSTAbits.SPEN = 0x1;
    TXSTAbits.BRGH = 0x0;

    //Transmit Status and Control (TXSTA)
    //Receive Status and Control (RCSTA)
    //Baud Rate Control (BAUDCTL)

    BAUDCTLbits.BRG16 = 0;

    SPBRG = 12;
    SPBRGH - 0;

    PIE1bits.RCIE = 1;
    PIR1bits.RCIF = 0;
}

void flipMode()
{

    switch (currentMode)
    {
    case 0: currentMode = 2;
        TXREG = 'f';
        __delay_ms(250);
        return;
    case 1: currentMode = 2;
        TXREG = 'f';
        __delay_ms(250);
        return;
    case 2: currentMode = 1;
        TXREG = 'w';
        __delay_ms(250);
        return;

    }
}

void sendPulsingFollowerIRBeacon()
{
    int x;
    x = 0;

    int y;
    y = 0;

    for (x = 0; x < 170; x++)
    {

        if (PORTAbits.RA1 == 0b1)
        {
            flipMode();
            return;
        }
        for (y = 0; y < 50; y++)
        {
            PORTAbits.RA4 = !PORTAbits.RA4;
            __delay_us(8);
            if (PORTAbits.RA1 == 0b1)
            {
                flipMode();
                return;
            }
        }

        //__delay_ms(11);
    }

}

void sendPulsingWatchdogIRBeacon()
{
    int x;
    x = 0;

    int y;
    y = 0;

    for (x = 0; x < 86; x++)
    {

        if (PORTAbits.RA1 == 0b1)
        {
            flipMode();
            return;
        }

        for (y = 0; y < 50; y++)
        {
            PORTAbits.RA4 = !PORTAbits.RA4;
            __delay_us(8);

            if (PORTAbits.RA1 == 0b1)
            {
                flipMode();
                return;
            }
        }

        //__delay_ms(11);
    }

}

void sendBeacon() //send 10 bursts of ten cycles
{
    INTCONbits.GIE = 0;

    int x;
    x = 0;
    for (x = 0; x < 50; x++)
    {


        PORTAbits.RA4 = !PORTAbits.RA4;
        __delay_us(1);
        
        

        
        
        
        
        
        
        if (PORTAbits.RA1 == 0b1)
        {
            flipMode();
            return;
        }

        
        //__delay_us(10);
    }

    INTCONbits.GIE = 1;
}

void configureCCPModule()
{
    // need to configure timer 1 to work with the ECCP module
    // counter mode
    T1CONbits.TMR1ON = 0; //timer off
    T1CONbits.TMR1GE = 0; //disable gate for timer 1
    T1CONbits.T1CKPS = 0b00; //set pre-scaler to 1:1
    T1CONbits.T1OSCEN = 0; //disable low power oscillator
    T1CONbits.TMR1CS = 0; //will disable external clock sync, and sets timer to Fosc / 4

    TMR1H = 0;
    TMR1L = 0;

    T1CONbits.TMR1ON = 1; //timer on

    //!!! CCP1 requires timer 1

    PIE1bits.CCP1IE = 0; //enable the interrupt for the capture module
    PIR1bits.CCP1IF = 0; //clear the ccp1 module flag
    PIE1bits.TMR1IE = 0;
    PIR1bits.TMR1IF = 0;



    CCP1CONbits.CCP1M = 0b0110; //capture mode on every 16th rising edge


    PIE1bits.CCP1IE = 1; //enable the interrupt for the capture module
    PIR1bits.CCP1IF = 0; //clear the ccp1 module flag
    PIE1bits.TMR1IE = 0; //timer interupt online?
    PIR1bits.TMR1IF = 0;

    //CCPR1H //high byte of the time
    //CCPR1L //low byte of the time
}

void main()
{
    //make sure clock is set correctly
    OSCCON = 0x71;
    OSCCONbits.SCS = 1; //internal oscillator
    //OSCTUNE = 0b01111; //  MAXIMUM SPEED!

    //configureTimer0();
    //OPTION_REGbits.T0CS = 0; //set timer 0 to timer mode
    //OPTION_REGbits.PSA = 1; //set the prescaler to the WDT
    //OPTION_REGbits.PS = 0b000; //select the bits for a 1:1 prescale

    //TMR0 = 240; //set the timer so it will run to 256 and interupt

    INTCONbits.PEIE = 1; //enable peripherial interrupts
    INTCONbits.GIE = 1; //global interupt on
    PIE1bits.CCP1IE = 1; //enable the CCP int

    //set up TRIS registers

    TRISA = 0b00000010;
    ANSEL = 0x00;
    PORTAbits.RA0 = 1;
    TRISCbits.TRISC2 = 1; //set C2 to input for CCP module

    PORTAbits.RA2 = 0;
    PORTAbits.RA3 = 0;
    //PORTAbits.RA4 = 1;
    PORTAbits.RA6 = 1; //enable the IR modulators
    PORTAbits.RA5 = 0;
    PORTAbits.RA7 = 0;


    configureUART();
    configureCCPModule();
    PIR1bits.CCP1IF = 0;

    currentMode = 0;
    sampleCounter = 0;
    t1 = 0;
    t2 = 0;
    t3 = 0;
    risingEdge = 1;
    counter = 0;

    while (1)
    {
        //read the button and see if we wanna write these characters
        if (PORTAbits.RA1 == 0b1)
        {
            //send mode change
            switch (currentMode)
            {
            case 0: currentMode = 2;
                TXREG = 'f';
                __delay_ms(250);
                break;
            case 1: currentMode = 2;
                TXREG = 'f';
                __delay_ms(250);
                break;
            case 2: currentMode = 1;
                TXREG = 'w';
                __delay_ms(250);
                break;

            }

        }


        if (currentMode == 0) //idle mode
        {
            PORTAbits.RA0 = !PORTAbits.RA0;
            //PORTAbits.RA4 = !PORTAbits.RA4;
            __delay_ms(150);
            PORTAbits.RA2 = 0;
            PORTAbits.RA3 = 0;
            //PORTAbits.RA5 = 1;
            //PORTAbits.RA6 = 1;
            //sendBeacon();
            PIE1bits.CCP1IE = 0; //disable the RSSI
        }
        if (currentMode == 1) //watchdog mode
        {
            PORTAbits.RA0 = 0;
            PORTAbits.RA2 = 1;
            PORTAbits.RA3 = 0;
            //PORTAbits.RA4 = 0;
            PIE1bits.CCP1IE = 0; //disable the RSSI
            //PORTAbits.RA7 = 1;
            //sendPulsingWatchdogIRBeacon();

        }
        if (currentMode == 2) //follower mode
        {
            PORTAbits.RA0 = 0;
            PORTAbits.RA2 = 0;
            PORTAbits.RA3 = 1;
            //PORTAbits.RA4 = 0;
            PIE1bits.CCP1IE = 1; //enable the RSSI
            //PORTAbits.RA7 = 0;
            sendBeacon();

            //TXREG = 'q';
            __delay_ms(10);
            PORTAbits.RA3 = !PORTAbits.RA3;
            __delay_ms(25);
            PORTAbits.RA3 = !PORTAbits.RA3;
            
            sendBeacon();
            __delay_ms(10);
            sendBeacon();

            //turn on the big ass LED array
        }

        //sendBeacon();


        __delay_ms(100);

    }
}

void interrupt int_handler(void)
{
    INTCONbits.GIE = 0;

    if (RCIF == 0x01)//we have received data on the USART module
    {
        rx_char = RCREG;
        PORTAbits.RA0 = 1;
        __delay_ms(100);
        PORTAbits.RA0 = 0;
        __delay_ms(100);
        
        INTCONbits.GIE = 1;
        PIR1bits.RCIF = 0;
        
    }

    if (CCP1IF == 0x01)
    {
        counter++;

        if (counter > 5000)
        {


            if (risingEdge == 0)//stage 1 get the timestamp of the rising signal
            {

                t1 = CCPR1L;
                CCP1CONbits.CCP1M = 0b0100;
                risingEdge = 1;

            }
            else if (risingEdge == 1) //get the timestamp of the falling signal
            {

                t2 = CCPR1L;
                CCP1CONbits.CCP1M = 0b0101;
                risingEdge = 2;

            }
            else
            {
                unsigned int diff;

                diff = t2 - t1;

                if (t2 > t1)
                {

                    if (diff < 0x80)
                    {
                        //send character for stop
                        TXREG = 'g';
                    }
                    else
                    {
                        TXREG = 's';
                    }
                }
                counter = 0;

                risingEdge = 0;
            }

            //TMR1H = 0;
            //TMR1L = 0;
            if (counter > 5150)
            {
                counter = 0;
            }
        }
        //TXREG = CCPR1L;
        PIR1bits.CCP1IF = 0;

    }
    INTCONbits.GIE = 1;

}

