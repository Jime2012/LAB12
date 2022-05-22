// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h> 

//Constantes
#define _XTAL_FREQ 1000000

//Variables
uint8_t valor = 0;
uint8_t address = 0;
uint8_t bandera = 1;


/*
 * PROTOTIPOS DE FUNCIÓN
 */
void setup(void);
uint8_t read_EEPROM(uint8_t address);
void write_EEPROM(uint8_t address, uint8_t data);

/*
 * INTERRUPCIONES
 */
void __interrupt() isr (void){
    if(PIR1bits.ADIF){
        if(ADCON0bits.CHS == 3){
           PORTC = ADRESH;
        }
        PIR1bits.ADIF = 0; 
     }
    if (INTCONbits.RBIF){ // INTERRUPCION DEL CAMBIO EN EL PORTB
        //PORTB = PORTB; // SE HACE UNA LECTURA DEL PUERTO B
        if(!PORTBbits.RB0){
        SLEEP();
        }
        if(!PORTBbits.RB1){
        write_EEPROM(address, PORTC);
        }
        INTCONbits.RBIF = 0;
    }
    return;
}

/*
 * ENTRADAS Y SALIDAS
 */
void setup(void){
    address = 0;
    ANSEL = 0b00001000;       //AN5
    ANSELH = 0;               // I/O digitales
    
    TRISAbits.TRISA5 = 1;     //RA5 entrada Potenciometro
    TRISC = 0;                //PORTA salida
    TRISD = 0;                //PORTA salida
    TRISE = 0; 
   
    
    TRISBbits.TRISB0 = 1;     //RB0 entrada Push-Button
    TRISBbits.TRISB1 = 1;     //RB0 entrada Push-Button
    OPTION_REGbits.nRBPU = 0; //Habilitación de PORTB pull-ups
    WPUBbits.WPUB0 = 1;       //Habilitamos Pull-Up en RB0
    WPUBbits.WPUB1 = 1;       //Habilitamos Pull-Up en RB0
    IOCBbits.IOCB0 = 1;// SE ACTIVA LA INTERRUPCION DEL CAMBIO EN PORTB
    IOCBbits.IOCB1 = 1;// SE ACTIVA LA INTERRUPCION DEL CAMBIO EN PORTB
    PORTB = 0;
    INTCONbits.RBIF = 0;// SE LIMPIA LA BANDERA
    
    PORTA = 0;                //Limpia PORTA
    PORTD = 0;                //Limpia PORTB
    PORTC = 0;                //Limpia PORTC
    
    //CONFIGURACIÓN DE OSCILADOR
    OSCCONbits.IRCF = 0b100;  //1MHz
    OSCCONbits.SCS = 1;       //Reloj interno

    //CONFIGURACIÓN DEL ADC
    ADCON1bits.ADFM = 0;      //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;     //VDD *Referencias internas
    ADCON1bits.VCFG1 = 0;     //VSS *Referencias internas
    
    ADCON0bits.ADCS = 0b01;   //FOSC/2
    ADCON0bits.CHS = 3;       //AN5/RE0 para potenciometro 1
    ADCON0bits.ADON = 1;      //Habilitamos Módulo ADC
    __delay_us(50);
    
    //CONFIGURACIÓN INTERUPCCIONES
    INTCONbits.GIE = 1;       //Se habilita las banderas globales
    INTCONbits.PEIE = 1;      //Habilitar interrupciones de periféricos
    PIR1bits.ADIF = 0;        //Limpiamos bandera de interrupción de ADC
    PIE1bits.ADIE = 1;        //Habilitamos interrupción de ADC
    INTCONbits.RBIE = 1;
}

/*
 * FUNCIONES EXTRAS
 */


/*
 * MAIN
 */
void main(void) {
    setup();
    while(1){
 
        PORTD = read_EEPROM(address);
        
        if(ADCON0bits.GO == 0){
            ADCON0bits.GO = 1;
        }
        __delay_ms(50);
        PORTEbits.RE0 = ~PORTEbits.RE0;
    }
    return;
}

uint8_t read_EEPROM(uint8_t address){
    EEADR = address;
    EECON1bits.EEPGD = 0;       // Lectura a la EEPROM
    EECON1bits.RD = 1;          // Obtenemos dato de la EEPROM
    return EEDAT;               // Regresamos dato 
}

void write_EEPROM(uint8_t address, uint8_t data){
    EEADR = address;
    EEDAT = data;
    EECON1bits.EEPGD = 0;       // Escritura a la EEPROM
    EECON1bits.WREN = 1;        // Habilitamos escritura en la EEPROM
    
    INTCONbits.GIE = 0;         // Deshabilitamos interrupciones
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;          // Iniciamos escritura
    
    EECON1bits.WREN = 0;        // Deshabilitamos escritura en la EEPROM
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
}