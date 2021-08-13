/*
 *	IMP projekt: 		TEPLOMER
 *	VEDUCI PROJEKTU: 	MICHAL BIDLO (bidlom@fit.vut.cz)
 *	VYPRACOVAL PROJEKT: BRANISLAV MATEAS (XMATEA00@stud.fit.vutbr.cz)
 *	90% ( Funkcia MCUInit je z DemoIMP, funkcie pre pracu s UART prevzate z cv1 IMP a MODIFIKOVANE pre UART5 fitkit3)
 *	Last-update: 22-12-2019
 */

/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdio.h>
#include <math.h>

#define PIN28  0x10000000

/* Initialize the MCU - basic clock settings, turning the watchdog off */
/* SOURCE: DEMO IMP FITKIT3 */
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

/* PORT INITIALIZATION */
void PortsInit(void)
{
    /* Turn on port clocks */
    SIM->SCGC5 = SIM_SCGC5_PORTE_MASK;
    SIM->SCGC1 = SIM_SCGC1_UART5_MASK;

    /* UART5 INIT */
    PORTE->PCR[8]  = PORT_PCR_MUX(0x03);
    PORTE->PCR[9]  = PORT_PCR_MUX(0x03);

    /* PORT INIT */
    PORTE->PCR[28] = ( PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x00));
}

/* SEND CHAR TO UART
 * SOURCE: IMP CVICENIE 1 */
void SendCh(char c)
{
    while( !(UART5->S1 & UART_S1_TDRE_MASK) && !(UART5->S1 & UART_S1_TC_MASK) );
    UART5->D = c;
}

/* SEND STRING TO UART
 * SOURCE: IMP CVICENIE 1 */
void SendStr(char* s) {
    unsigned int i = 0;
    while (s[i] != '\0') {
        SendCh(s[i++]);
    }
}

/* UART5 INIT */
void UART5Init()
{
    UART5->C2  &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
    UART5->BDH =  0x00;
    UART5->BDL =  0x1A; // Baud rate 115 200 Bd, 1 stop bit
    UART5->C4  =  0x0F; // Oversampling ratio 16, match address mode disabled
    UART5->C1  =  0x00; // 8 data bitu, bez parity
    UART5->C3  =  0x00;
    UART5->MA1 =  0x00; // no match address (mode disabled in C4)
    UART5->MA2 =  0x00; // no match address (mode disabled in C4)
    UART5->S2  |= 0xC0;
    UART5->C2  |= ( UART_C2_TE_MASK | UART_C2_RE_MASK ); // Zapnout vysilac i prijimac
}

/* RECEIVE CHAR FROM UART5
 * SOURCE: IMP CVICENIE 1 */
char ReceiveCh(void) {
	while(!(UART5->S1 & UART_S1_RDRF_MASK));
	return UART5->D;
}


/* MAIN */
int main(void)
{
	float hi = 0.0, li = 0.0, try = 0.0;
	char itoa[100];
	char flag;
 	MCUInit();
    PortsInit();
    UART5Init();
    SendStr("=====================INIT======================\r\n");
    while (1) {
    	if(GPIOE_PDIR & PIN28)
    	{
    		/* ZACINA SLEDOVANIE STAVU PINU A POCITANIE DLZKY v log. 1 a v log. 0, na zaklade dokumentacie sa tento proces zopakuje este niekolko krat*/
    		while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	while(GPIOE_PDIR & PIN28){ hi = hi + 1;};
        	while(!(GPIOE_PDIR & PIN28)){li = li + 1;};

        	/* VYPOCET TEPLOTY ZO ZISKANEHO DUTY CYCLE */
        	try = hi / (hi + li);
        	try = 212.77 * try - 68.085;
        	gcvt(try, 8, itoa);
        	SendStr("TEPLOTA: ");
        	SendStr(itoa);
        	SendStr("\r\n");
        	hi = 0.0; li = 0.0;

        	/* OVLADANIE APLIKACIE JEDNODUCHYM PRIKAZOM */
        	SendStr("Continue? [Y/N]\r\n");
        	flag = ReceiveCh();
        	if(flag != 'Y')
        	{
        		SendStr("=============EXITING PROGRAM===============\r\n");
        		while(1);
        	}
    	}
    }
    return 0;
}
