#include <at89x051.h>

#define POS1 P3_0
#define POS2 P3_1
#define POS3 P3_2
#define POS4 P3_7

//#define test 1

#ifdef test
#define POS_ON 1
#define POS_OFF 0
#define pauza ;
#else
#define POS_ON 0
#define POS_OFF 1
#define pauza while (--j != 0) {;}
#endif


typedef unsigned char byte;

byte j; //univerzalni zpozdovac

/*
 AAAA
F    B  
F    B
F    B
 GGGG
E    C
E    C
E    C
 DDDD

P1 = 0b A F B E D C G dp
*/

//znaky na &seg LED 0-9, na pozicich 10-15 je jednoduchy efekt

byte table[16] = {
	0b11111100, //0
	0b00100100, //1
	0b10111010,
	0b10101110,
	0b01100110, //4
	0b11001110,
	0b11011110,
	0b10100100,
	0b11111110,
	0b11101110, //9
	0b10000000, //efekt
	0b00100001,
	0b00000100,
	0b00001001,
	0b00010000,
	0b01000001
};

byte dp = 0; //dvojtecka

volatile byte num[4]; //pozice

volatile byte intflag;    // Pocitadlo 50ms ticku

void Timer0_ISR(void) __interrupt (1) {
    intflag--;        // ticky
	TL0 = 15536 & 0xff;
	TH0 = (15536) >> 8;
}



void show1(byte a) {
	P1 = a; //01010101  - seg. 
	POS1 = POS_ON;
	pauza
	POS1 = POS_OFF;
}
void show2(byte a) {
	P1 = a | dp; //01010101  - seg. 
	POS2 = POS_ON;
	pauza
	POS2 = POS_OFF;
}
void show3(byte a) {
	P1 = a; //01010101  - seg. 
	POS3 = POS_ON;
	pauza
	POS3 = POS_OFF;
}
void show4(byte a) {
	P1 = a; //01010101  - seg. 
	POS4 = POS_ON;
	pauza
	POS4 = POS_OFF;
}

void display() {
	show1(table[num[0]]);
	show2(table[num[1]]);
	show3(table[num[2]]);
	show4(table[num[3]]);
}

byte cita = 0;
byte alarm = 0;

//odectu 1 sekundu. 1, pokud uz jsme na nule, 0 pokud se cita dal
byte dec1() {
	if (num[3]==1 && num[2]==0 && num[1]==0 && num[0] == 0) {num[3]=0;return 1;}
	if (--num[3]<9) return 0;
	num[3]=9;
	if (--num[2]==255) {
	  num[2]=5;
	  if (--num[1]<9) return 0;
	  num[1]=9;
	  if(--num[0]==255) return 1;
	  
	}
	return 0;
}

// zvysim minuty o 1. Az k 99 - to je schvalne
void plusmin() { 
     num[1]++;
     if (num[1]==10) {
     	num[1]=0;
     	num[0]++;
     	if (num[0]==10) {
     	    num[0] = 0;
     	}
     }
}

byte k1,k2;
byte longk2;
byte efnum;

//neustale dokola zobrazuju na displeji znaky 10-15
void efekt() {
	byte acc;
	efnum--;
	if (efnum == 0) {
		efnum = 50;
		acc = num[0]+1;
		if (acc==16) acc=10;
		num[0] = acc;
		num[1] = acc;
		num[2] = acc;
		num[3] = acc;
	}
}

////MAIN/////////////////////////////////
void main() {
	//vypnout displej
	POS1 = POS_OFF;
	POS2 = POS_OFF;
	POS3 = POS_OFF;
	POS4 = POS_OFF;

	//klavesy jsou vstup
	P3_4 = 1;
	P3_5 = 1;

	//nuluju displej
	num[0] = 0;
	num[1] = 0;
	num[2] = 0;
	num[3] = 0;



	//interrupt, pri 12MHz je vyvolan kazdych 50ms
	TMOD = 0x1;
	TL0 = 15536 & 0xff;
	TH0 = (15536) >> 8;
	TR0 = 1;
	ET0 = 1;
	EA = 0;
	intflag = 20; // 50ms * 20 = 1000ms, neboli 1 sec, neboli nase stastne cislo!

	//na zacatku se necita
	cita = 0;

while(1){
	//test klavesy RESET
	if (P3_4 == 1) {k1=10;} //osetreni zakmitu
	if (P3_4 == 0) {if (--k1 == 0) {
		//Stisknuta? Rusime alarm, citani, casovac a vsechny cislice na 0
			alarm=0;
			cita = 0;
			EA = 0;
			num[0] = 0;
			num[1] = 0;
			num[2] = 0;
			num[3] = 0;
			}
	}
	// test klavesy SET
	if (P3_5 == 1) {k2=30;longk2 = 0;if(cita==1)EA = 1;} // zakmity a pocitadlo "dlouheho drzeni". Pokud je citani=1, spousti timer
	if (P3_5 == 0) {if (--k2 == 0) {
				k2 = 100; //autorepeat
				longk2++;
				if (longk2>10) k2 = 50; //pokud drzim tl. delsi dobu, zrychlim autorepeat
				alarm=0;
				cita = 255;
				//citani je 255. Po nejakem case dobehne k 1 a tim se spusti odpocet
				plusmin(); //minuty++
				num[2] = 0; //sekundy = 00
				num[3] = 0;

			}
	}

	if(cita>1) cita--; //Po pusteni tlacitka SET je potreba chvilka pauzy pred spustenim citani

	display(); //Ukaz cislice na displeji
	
	if (alarm==1) { //spusteny alarm, tzn. zapni bzucak
		P3_3 = 0;
		efekt(); //efekt na displeji
	} else {
		P3_3 = 1; //bzucak vypnout
	}

	if (intflag == 10) {dp = 0;} //po pul sekunde vypnu dvojtecku

	if (intflag == 0) {
		//uplynula sekunda
		intflag = 20; //reset konstanty
		if (cita==1) {
			dp = 1; //zapnu dvojtecku
			//dekrement sekund
			if (dec1()) {alarm = 1; //spustim alarm
				cita=0; //vypnu citani
				EA = 0; //vypnu interrupt
				efnum = 50; //casovani efektu
				num[0] = 10; //vsechny pozice displeje maji 1. znak efektu
				num[1] = 10;
				num[2] = 10;
				num[3] = 10;
				dp=0; //a pro jistotu vypnu dvojtecku
				}
		}
	}


	}
}