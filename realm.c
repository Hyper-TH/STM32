
/*
Copyright (C) 2014  Frank Duignan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdint.h>
#include <stm32l031xx.h>
#include "stm32l031lib.h"

// Find types: h(ealth),s(trength),m(agic),g(old),w(eapon)
static const char FindTypes[]={'h','s','m','g','w'};

// The following arrays define the bad guys and 
// their battle properies - ordering matters!
// Baddie types : O(gre),T(roll),D(ragon),H(ag)
static const char Baddies[]={'O','T','D','H'};
// The following is 4 sets of 4 damage types
static const byte WeaponDamage[]={10,10,5,25,10,10,5,25,10,15,5,15,5,5,2,10};
#define ICE_SPELL_COST 10
#define FIRE_SPELL_COST 20
#define LIGHTNING_SPELL_COST 30
static const byte FreezeSpellDamage[]={10,20,5,0};
static const byte FireSpellDamage[]={20,10,5,0};
static const byte LightningSpellDamage[]={15,10,25,0};
static const byte BadGuyDamage[]={10,10,15,5};
static int GameStarted = 0;
static tPlayer thePlayer;
static tRealm theRealm;
void randomize(void);
void ADCBegin(void);
uint16_t ADCRead(void);

/* START OF MUSIC CONFIG */
#define CPU_FREQ 16000000

// structure for music notes arrays
typedef struct {
	uint32_t frequency;
	uint32_t duration;
	uint32_t wait;
} note;

void playNote(note *n);
void playTune(const note *n, int notecount, int repeat);
void SysTick_Handler(void);
void delayms(uint32_t dly);
void stopSound(void);

static volatile uint32_t SoundDuration = 0;
static volatile uint32_t ms_counter = 0;
static const note *Tune;
static volatile int TuneNoteCount=0;
static volatile int TuneLoop = 0;
//static volatile uint32_t blink_period = 0;

/*START OF NOTES*/
#define B2 124

#define A3 220
#define As3 233
#define B3 247
#define C3 130
#define D3 147
#define E3 165
#define F3 175
#define Fs3 185
#define G3 196

#define A4 440
#define As4 466
#define B4 494
#define C4 262
#define Cs4 277
#define D4 294
#define Ds4 311
#define E4 329
#define F4 349
#define Fs4 370
#define G4 392
#define Gs4 415
#define A5 880

#define As5 932
#define B5 988
#define C5 523
#define Cs5 554
#define D5 587
#define Ds5 622
#define E5 659
#define F5 698
#define Fs5 740
#define G5 784
#define Gs5 831

#define C6 1047
#define Cs6 1109
#define D6 1175
#define Ds6 1245
#define E6 1319
#define F6 1397
#define Fs6 1480
#define G6 1568

#define C7 2093
#define C8 4186
/*END OF NOTES*/

/*START OF MUSIC ARRAYS*/
static const note testNote[] = { {C4,1,0} };

// Startup sound effect (GameBoy startup)
static const note startUp[] = {
	{As4,50,0}, {F5,50,0}, {A5,50,0}, {C6,50,0}, {D6,50,0}, {E6,50,0}, {G6,800,0},
	{C7,50,0}, {C8,800,0}
};

static const note rock[] = { {C4,50,0} };
// Lightning Effect (Originally composed)
static const note light[] = {
	{C5,77,0}, {E5,77,0}, {G5,77,0},
};

// Fire Effect (Originally composed)
static const note fire[] = {
	{A3,77,0}, {C4,77,0}, {E4,77,0},
};

// Ice Effect (Originally composed)
static const note ice[] = {
	{C6,77,0}, {E5,77,0}, {G5,77,0},
};

// Punch Effect (Originally composed)
static const note punch[] = {
	{C5,77,0}, {E5,77,0}, 
};

// Sword Effect (Originally composed)
static const note sword[] = {
	{C4,100,0}, {G4,77,0}, 
};

// BG music for baddies (Champion Lance/Trainer Red Pokemon GSC)
static const note baddies[] = { 
	//1st page
	//B1-7
	{Ds4,306,306}, {Ds4,306,306}, //1
	{Ds4,306,306}, {Ds4,306,0}, {D4,306,0}, //2
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,306}, //3
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, //4
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,306}, //5
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //6
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,306}, //7

	//B8-14
	{Ds4,153,10}, {Ds4,153,306}, {Ds4,153,10}, {Ds4,153,0}, {Cs5,306,0}, //8
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //9
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {D4,306,0}, //10
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //11
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {F4,306,0}, //12
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //13
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {D4,306,0}, //14

	//B15-19
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //15
	{Ds4,153,10}, {Ds4,153,0}, {C5,306,0}, {Ds4,153,10}, {Ds4,153,0}, {Cs5,306,0}, //16
	{Ds5,612,0}, {Ds4,612,0}, //17
	{E5,765,0}, {As4,77,0}, {B4,77,0}, {C5,77,0}, {Cs5,77,0}, //18
	{Ds5,459,0}, {As4,306,153}, {D5,153,0}, {Ds5,153,0}, //19

	//B20-23
	{Cs5,306,0}, {C5,306,0}, {B4,306,0}, {As4,306,0}, //20
	{B4,612,0}, {E5,612,0}, //21
	{Gs4,153,10}, {Gs4,153,0}, {B4,306,0}, {Gs4,153,10}, {Gs4,153,0}, {Cs5,306,0}, //22
	{Ds5,459,0}, {As4,306,153}, {D5,153,0}, {Ds5,153,0}, //23

	//B24-27
	{Cs5,306,0}, {C5,306,0}, {B4,306,0}, {As4,153,0}, {B4,153,0}, //24
	{E5,612,0}, {Gs5,612,0}, //25 
	{E5,612,0}, {B5,612,0}, //26
	{Ds5,153,10}, {Ds5,153,153}, {Ds5,153,153}, {Ds5,153,153}, {Ds5,153,153}, //27

	//2nd page
	//B28-32
	{Ds5,153,153}, {Ds5,153,0}, {E5,306,10}, {E5,306,0}, //28
	{Ds5,153,10}, {Ds5,153,153}, {Ds5,153,153}, {Ds5,153,153}, {Ds5,153,153}, //29
	{Ds5,153,153}, {Ds5,153,0}, {Fs5,306,10}, {Fs5,306,0}, //30
	{Ds5,612,0}, {Ds4,612,0}, //31
	{Cs5,612,0}, {Cs4,612,0}, //32

	//B33-36
	{Ds4,1224,10}, //33
	{Ds4,1224,0}, //34
	{F4,1224,0}, //35
	{Fs4,1224,0}, //36

	//B37-41
	{Ds6,306,0}, {D6,306,0}, {Cs6,306,0}, {C6,306,0}, //37
	{Ds6,153,10}, {D6,153,0}, {Cs6,153,10}, {C6,153,0}, {B5,306,10}, {B5,306,0}, //38
	{Ds6,306,0}, {D6,306,0}, {Cs6,306,0}, {C6,306,0}, //39
	{Ds6,153,10}, {D6,153,0}, {Cs6,153,10}, {C6,153,0}, {B5,306,10}, {B5,306,0}, //40
	{Ds4,1224,0}, //41

	//B42-45
	{B4,1224,0}, //42
	{As4,1224,0}, //43
	{Gs4,1224,0}, //44
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //45

	//B46-49
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {A4,306,0}, //46
	{Ds4,153,10}, {Ds4,153,0}, {As4,306,0}, {Ds4,153,10}, {Ds4,153,0}, {B4,306,0}, //47
	{Ds4,153,10}, {Ds4,153,0}, {Cs5,306,0}, {Ds4,153,10}, {Ds4,153,0}, {D5,306,0}, //48
	{Ds5,306,0}, {As4,306,0}, {E5,306,0}, {B4,306,0}, //49

	//3rd page
	//B50-52
	{Fs5,306,0}, {Cs5,306,0}, {E5,306,0}, {B4,306,0}, //50
	{Ds5,306,0}, {As4,306,0}, {E5,306,0}, {B4,306,0}, //51
	{Fs5,306,0}, {Cs5,306,0}, {Gs5,306,0}, {D5,306,0} //52
};


// BG music in general (Viridian Forest Pokemon RGB)
static const note cave[] = {
	//1st page
	//B1-5
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250},//5

	//B6-9
	{As5,250,250}, {As5,250,250}, {As5,250,250}, {As5,250,250}, //6
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250}, //7
	{Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, //8
	{B4,250,250}, {B4,250,250}, {B4,250,250}, {B4,250,250}, //9

	//B10-12
	{Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, //10
	{G3,250,0}, {As3,250,250}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {As3,250,10}, //11
	{As3,250,0}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {A3,250,250}, {Fs3,250,0}, //12


	//START REPEAT
	//B1-5
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250},//5

	//B6-9
	{As5,250,250}, {As5,250,250}, {As5,250,250}, {As5,250,250}, //6
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250}, //7
	{Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, //8
	{B4,250,250}, {B4,250,250}, {B4,250,250}, {B4,250,250}, //9

	//B10-12
	{Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, //10
	{G3,250,0}, {As3,250,250}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {As3,250,10}, //11
	{As3,250,0}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {A3,250,250}, {Fs3,250,0}, //12
	//END REPEAT

	//B13-16
	{E4,250,0}, {G4,250,0}, {E4,250,0}, {Ds4,250,0}, {E4,250,10}, {E4,250,0}, {E6,250,250}, //13
	{Ds6,250,250}, {D6,250,250}, {Cs6,250,0}, {C6,250,0}, {E5,250,0}, {G5,250,0}, //14
	{As4,250,0}, {Cs4,250,0}, {As4,250,0}, {A4,250,0}, {As4,250,0}, {G4,250,0}, {G6,250,250}, //15
	{Fs6,250,250}, {F6,250,250}, {E6,250,0}, {Ds6,250,0}, {D6,250,0}, {Cs6,250,0}, //16

	//B17-21
	{E5,250,250}, {B5,250,250}, {As5,250,250}, {D6,250,250}, //17
	{Cs6,250,250}, {Gs5,250,250}, {G5,250,250}, {B5,250,250}, //18
	{As5,250,250}, {E5,250,250}, {Ds5,250,250}, {A5,250,250}, //19
	{Gs5,250,250}, {E5,250,250}, {Fs5,250,250}, {Ds5,250,250}, //20
	{E5,250,250}, {B5,250,250}, {As5,250,250}, {D6,250,250}, //21

	//2nd page
	//B22-26
	{Cs6,250,250}, {Gs5,250,250}, {G5,250,250}, {B5,250,250}, //22
	{As5,250,250}, {E5,250,250}, {Ds5,250,250}, {A5,250,250}, //23
	{Gs5,250,250}, {E5,250,250}, {Fs5,250,250}, {Ds5,250,250}, //24
	{E5,250,250}, {B5,250,250}, {As5,250,250}, {D6,250,250}, //25
	{Cs6,250,250}, {Gs5,250,250}, {G5,250,250}, {B5,250,250}, //26

	//B27-30
	{As5,250,250}, {E5,250,250}, {Ds5,250,250}, {A5,250,250}, //27
	{Gs5,250,250}, {E5,250,250}, {Fs5,250,250}, {Ds5,250,250}, //28
	{E4,500,0}, {B4,500,0}, {As4,500,0}, {D5,500,0}, //29
	{Cs5,500,0}, {Gs4,500,0}, {G4,500,0}, {B4,500,0}, //30

	//B31-34
	{As4,500,0}, {E4,500,0}, {Ds4,500,0}, {A4,500,0}, //31
	{Gs4,500,0}, {E4,500,0}, {Fs4,500,0}, {Ds4,500,0}, //32
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250},//33
	{As5,250,250}, {As5,250,250}, {As5,250,250}, {As5,250,250}, //34

	//B35-38
	{E5,250,250}, {E5,250,250}, {E5,250,250}, {E5,250,250}, //35
	{Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, {Cs6,250,250}, //36
	{B4,250,250}, {B4,250,250}, {B4,250,250}, {B4,250,250}, //37
	{Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, {Fs5,250,250}, //38

	//B39-42
	{G3,250,0}, {As3,250,250}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {As3,250,10}, //39
	{As3,250,0}, {G3,250,0}, {Cs4,250,250}, {G3,250,0}, {A3,250,250}, {Fs3,250,0}, //40
	{E4,250,0}, {G4,250,0}, {E4,250,0}, {Ds4,250,0}, {E4,250,10}, {E4,250,0}, {E6,250,250}, //41
	{Ds6,250,250}, {D6,250,250}, {Cs6,250,0}, {C6,250,0}, {E5,250,0}, {G5,250,0}, //42

	//B43-47
	{As4,250,0}, {Cs4,250,0}, {As4,250,0}, {A4,250,0}, {As4,250,0}, {G4,250,0}, {G6,250,250}, //43
	{Fs6,250,250}, {F6,250,250}, {E6,250,0}, {Ds6,250,0}, {D6,250,0}, {Cs6,250,0}, //44
	{E4,250,500}, {E3,250,250}, {E3,250,0}, {Fs4,500,0}, //45
	{E4,250,500}, {E3,250,250}, {E3,250,0}, {Fs4,500,0}, //46
	{E4,250,500}, {E3,250,250}, {E3,250,0}, {Fs4,500,0}, //47

	//3rd page
	//B48
	{E4,250,500,}, {E3,250,1000}
};
// Sound effect for picking up an item (Pokemon)
static const note item[] = { 
	{A5,306,136}, {A5,153,10}, {A5,153,10}, {A5,153,0}, {B5,306,10}, {B5,306,10}, {B5,306,0}, {A5,1224,10},
};


// Sound effect for encountering baddies (Wild Pokemon Encounter)
static const note encounter[] = { 
	//Mid B1-2
	{A5,90,0}, {Gs5,90,0}, {G5,90,0}, {Fs5,90,0}, {A5,90,0}, {F5,90,0}, {Fs5,90,0}, {F5,90,0}, //1
	{A5,90,0}, {E5,90,0}, {F5,90,0}, {E5,90,0}, {A5,90,0}, {Ds5,90,0}, {E5,90,0}, {Ds5,90,0}, //1.5
	{A5,90,0}, {D5,90,0}, {Ds5,90,0}, {D5,90,0}, {A5,90,0}, {Cs5,90,0}, {D5,90,0}, {C5,90,0}, //2
	{A5,90,0}, {C5,90,0}, {Cs5,90,0}, {C5,90,0}, {A5,90,0}, {B4,90,0}, {C5,90,0}, {B4,90,0} //2.5
};



// Generic sound effect (Pokemon A button)
static const note aButton[] = { 
	{B5,77,0}, {E6,77,0}
};

// Sound effect for picking up a weapon (Level up Pokemon)
static const note weapon[] = {
	{E5,231,10},{E5,231,10}, {E5,231,0}, {B5,693,10}
};

// Sound effect for dying (Originally composed)
static const note lost[] = {
	{C4,77,0}, {D4,77,0}, {B3,77,0}, {A3,77,0}, {G3,77,0},{F3,77,0}, {E3,77,0}, {D3,77,0}, {C3,77,0}, {B2,77,0}
};

// Sound effect for winning (Victory Fanfare FFVII)
static const note won[] = {
	{C5,231,10}, {C5,231,10}, {C5,231,10}, {C5,462,0},
	{Gs4,462,0}, {As4,462,0}, {C5,462,0}, {As4,231,0}, {C5,924,0},
};
/*END OF MUSIC ARRAYS*/

/*START OF NOTE COUNT ARRAYS*/
#define NOTE_COUNT_startUp (sizeof(startUp)/sizeof(note))
#define NOTE_COUNT_rock (sizeof(rock)/sizeof(note))
#define NOTE_COUNT_baddies (sizeof(baddies)/sizeof(note))
#define NOTE_COUNT_cave (sizeof(cave)/sizeof(note))
#define NOTE_COUNT_testNote (sizeof(testNote)/sizeof(note))
#define NOTE_COUNT_item (sizeof(item)/sizeof(note))
#define NOTE_COUNT_encounter (sizeof(encounter)/sizeof(note))
#define NOTE_COUNT_aButton (sizeof(aButton)/sizeof(note))
#define NOTE_COUNT_weapon (sizeof(weapon)/sizeof(note))
#define NOTE_COUNT_light (sizeof(light)/sizeof(note))
#define NOTE_COUNT_fire (sizeof(fire)/sizeof(note))
#define NOTE_COUNT_ice (sizeof(ice)/sizeof(note))
#define NOTE_COUNT_sword (sizeof(sword)/sizeof(note))
#define NOTE_COUNT_punch (sizeof(punch)/sizeof(note))
#define NOTE_COUNT_lost (sizeof(lost)/sizeof(note))
#define NOTE_COUNT_won (sizeof(won)/sizeof(note))
/*END OF NOTE COUNT ARRAYS*/

/* END OF MUSIC CONFIG */

__attribute__((noreturn)) void runGame(void)  
{
	// play this so that it won't beep 
	for (unsigned int i=0; i < NOTE_COUNT_testNote; i++)
	{
		playNote(&testNote[i]);
		GPIOA->ODR ^= 1; 
		GPIOA->ODR ^= 2;
	}
				
	char ch;
	randomize();
	eputs("MicroRealms on the STM32L031\r\n");	
	
	showHelp();		
	
	while(GameStarted == 0)
	{
		for (unsigned int i=0; i < NOTE_COUNT_startUp; i++)
		{
			playNote(&startUp[i]);
			GPIOA->ODR ^= 1; 
			GPIOA->ODR ^= 2;
		}
	
		showGameMessage("Press S to start a new game\r\n");
		ch = getUserInput();			
		
		if ( (ch == 'S') || (ch == 's') )
			GameStarted = 1;
		for (unsigned int i=0; i < NOTE_COUNT_aButton; i++)
		{
			playNote(&aButton[i]);
			GPIOA->ODR ^= 1; 
			GPIOA->ODR ^= 2;
		}
		
	}
	RCC->IOPENR |= (1 << 0); // enable GPIOA
	RCC->IOPENR |= (1 << 1); // enable GPIOB
	RCC->IOPENR |= (1 << 2); // enable GPIOC

	pinMode(GPIOA, 0,1); // Make GPIOA bit 0 an output
	pinMode(GPIOA, 1,1); // Make GPIOA bit 1 an output
	initRealm(&theRealm);	
	initPlayer(&thePlayer,&theRealm);
	showPlayer(&thePlayer);
	showRealm(&theRealm,&thePlayer);
	showGameMessage("Press H for help");
	
	while (1)
	{
		ch = getUserInput();
		ch = ch | 32; // enforce lower case
		
		//playTune(cave, NOTE_COUNT_cave, 1);
		/*
		for (unsigned int i=0; i < NOTE_COUNT_cave; i++)
		{
			playNote(&cave[i]);
		}
		*/
		
		switch (ch) 
		{
			case 'h' : 
			{
				for (unsigned int i=0; i < NOTE_COUNT_aButton; i++)
				{
					playNote(&aButton[i]);
					GPIOA->ODR ^= 1; 
					GPIOA->ODR ^= 2;
				}
				showHelp();
				
				break;
			} // end case h
			
			/* START OF DIRECTION INPUTS */
			case 'w' : 
			{
				showGameMessage("North");
				step('n',&thePlayer,&theRealm);
				
				break;
			} // end case w
			case 's' : 
			{
				showGameMessage("South");
				step('s',&thePlayer,&theRealm);
				break;

			} // end case s
			case 'd' : 
			{
				showGameMessage("East");
				step('e',&thePlayer,&theRealm);
				break;
			} // end case d
			
			case 'a' : 
			{
				showGameMessage("West");
				step('w',&thePlayer,&theRealm);
				break;
			} // end case a
			/* END OF DIRECTION INPUTS */
			
			// map
			case '#' : 
			{		
				for (unsigned int i=0; i < NOTE_COUNT_aButton; i++)
				{
					playNote(&aButton[i]);
					GPIOA->ODR ^= 1; 
					GPIOA->ODR ^= 2;
				}
				showRealm(&theRealm,&thePlayer);
				
				break;
			} // end case #
			
			case 'p' : 
			{		
				for (unsigned int i=0; i < NOTE_COUNT_aButton; i++)
				{
				playNote(&aButton[i]);
				GPIOA->ODR ^= 1; 
				GPIOA->ODR ^= 2;
				}
				showPlayer(&thePlayer);
				
				break;
			} // end case p
			
		} // end switch
		
	} // end while
	
} // end runGame

void step(char Direction,tPlayer *Player,tRealm *Realm)
{
	uint8_t new_x, new_y;
	new_x = Player->x;
	new_y = Player->y;
	byte AreaContents;
	
	// switch case for changing player position
	switch (Direction) 
	{
		case 'n' :
		{
			if (new_y > 0)
				new_y--;
			break;
		}
		case 's' :
		{
			if (new_y < MAP_HEIGHT-1)
				new_y++;
			break;
		}
		case 'e' :
		{
			if (new_x <  MAP_WIDTH-1)
				new_x++;
			break;
		}
		case 'w' :
		{
			if (new_x > 0)
				new_x--;
			break;
		}		
		
	} // end switch
	
	AreaContents = Realm->map[new_y][new_x];
	
	if ( AreaContents == '*')
	{
		for (unsigned int i=0; i < NOTE_COUNT_rock; i++)
		{
			playNote(&rock[i]);
			GPIOA->ODR ^= 1; 
			GPIOA->ODR ^= 2;
		}
	
		showGameMessage("A rock blocks your path.");
		return;
	}
	
	Player->x = new_x;
	Player->y = new_y;
	int Consumed = 0;
	
	switch (AreaContents)
	{
		/* START OF BADDIES */
		// const char Baddies[]={'O','T','B','H'};
		case 'O' :
		{
			for (unsigned int i=0; i < NOTE_COUNT_encounter; i++)
			{
				playNote(&encounter[i]);
				GPIOA->ODR ^= 1; 
				GPIOA->ODR ^= 2;
			}
			
			showGameMessage("A smelly green Ogre appears before you");
			Consumed = doChallenge(Player,0);
			
			break;
		} // end case o
		
		case 'T' :
		{
			for (unsigned int i=0; i < NOTE_COUNT_encounter; i++)
			{
				playNote(&encounter[i]);
				GPIOA->ODR ^= 1; 
				GPIOA->ODR ^= 2;
			}
			
			showGameMessage("An evil troll challenges you");
			Consumed = doChallenge(Player,1);
			
			break;
		} // end case t
		
		case 'D' :
		{
			for (unsigned int i=0; i < NOTE_COUNT_encounter; i++)
			{
				playNote(&encounter[i]);
				GPIOA->ODR ^= 1; 
				GPIOA->ODR ^= 2;
			}
			
			showGameMessage("A smouldering Dragon blocks your way !");
			Consumed = doChallenge(Player,2);
			
			break;
		} // end case d
		
		case 'H' :
		{
			for (unsigned int i=0; i < NOTE_COUNT_encounter; i++)
			{
				playNote(&encounter[i]);
				GPIOA->ODR ^= 1; 
				GPIOA->ODR ^= 2;
			}

			showGameMessage("A withered hag cackles at you wickedly");
			Consumed = doChallenge(Player,3);
			
			break;
		} // end case H
		/* END BADDIES */
		
		/* START OF ITEMS */
		case 'h' :
		{
			GPIOA->ODR |= (1 << 1); // turn led on
			
			for (unsigned int i=0; i < NOTE_COUNT_item; i++)
			{
				playNote(&item[i]);
			}
			
			showGameMessage("You find an elixir of health");
			setHealth(Player,Player->health+10);
			Consumed = 1;		
			
			delay(100000); // wait
			GPIOA->ODR &= ~(1u << 1); // turn led off
			
			break;
		} // end h
		
		case 's' :
		{
			GPIOA->ODR |= (1 << 1); // turn led on
			
			for (unsigned int i=0; i < NOTE_COUNT_item; i++)
			{
				playNote(&item[i]);
			}
			
			showGameMessage("You find a potion of strength");
			Consumed = 1;
			setStrength(Player,Player->strength+1);
			
			delay(100000); // wait
			GPIOA->ODR &= ~(1u << 1); // turn led off
			
			break;
		} // end case s
		
		case 'g' :
		{
			GPIOA->ODR |= (1 << 1); // turn led on
			
			for (unsigned int i=0; i < NOTE_COUNT_item; i++)
			{
				playNote(&item[i]);
			}
	
			showGameMessage("You find a shiny golden nugget");
			Player->wealth++;			
			Consumed = 1;
			
			delay(100000); // wait
			GPIOA->ODR &= ~(1u << 1); // turn led off
			
			break;
		} // end case g
		
		case 'm' :
		{
			GPIOA->ODR |= (1 << 1); // turn led on
			
			for (unsigned int i=0; i < NOTE_COUNT_item; i++)
			{
				playNote(&item[i]);
			}

			showGameMessage("You find a magic charm");
			Player->magic++;						
			Consumed = 1;
			
			delay(100000); // wait
			GPIOA->ODR &= ~(1u << 1); // turn led off
			
			break;
		} // end case m
		
		/* END OF ITEMS */
		case 'w' :
		{
			Consumed = addWeapon(Player,(int)random(MAX_WEAPONS-1)+1);
			showPlayer(Player);
			break;			
		} // end case w
		
		case 'X' : 
		{
			// Player landed on the exit
			eputs("A door! You exit into a new realm");
			setHealth(Player,100); // maximize health
			initRealm(&theRealm);
			showRealm(&theRealm,Player);
		} // end case X
	} // end switch
	if (Consumed)
		Realm->map[new_y][new_x] = '.'; // remove any item that was found
} // end voidstep()


int doChallenge(tPlayer *Player,int BadGuyIndex)
{
	char ch;
	char Damage;
	const byte *dmg;
	int BadGuyHealth = 100;
	
	eputs("Press F to fight");
	ch = getUserInput() | 32; // get user input and force lower case
	
	// player choses to fight
	if (ch == 'f')
	{
		playTune(cave, NOTE_COUNT_cave, 1);
		
		for (unsigned int i=0; i < NOTE_COUNT_baddies; i++)
		{
			playNote(&baddies[i]);
		}
		
		eputs("\r\nChoose action");
		
		while ( (Player->health > 0) && (BadGuyHealth > 0) )
		{
			eputs("\r\n");
			
			// Player takes turn first
			if (Player->magic > ICE_SPELL_COST)
				eputs("(I)CE spell");
			if (Player->magic > FIRE_SPELL_COST)
				eputs("(F)ire spell");
			if (Player->magic > LIGHTNING_SPELL_COST)
				eputs("(L)ightning spell");
			
			if (Player->Weapon1)
			{
				eputs("(1)Use ");
				eputs(getWeaponName(Player->Weapon1));
			}	
			
			if (Player->Weapon2)
			{
				eputs("(2)Use ");
				eputs(getWeaponName(Player->Weapon2));
			}
			
			eputs("(P)unch");
			ch = getUserInput();
			
			switch (ch)
			{
				
				case 'i':
				case 'I':
				{
					for (unsigned int i=0; i < NOTE_COUNT_ice; i++)
					{
						playNote(&ice[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					eputs("FREEZE!");
					Player->magic -= ICE_SPELL_COST;
					BadGuyHealth -= FreezeSpellDamage[BadGuyIndex]+random(10);
					zap();
					
					break;
				}
				
				case 'f':
				case 'F':
				{
					for (unsigned int i=0; i < NOTE_COUNT_fire; i++)
					{
						playNote(&fire[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					eputs("BURN!");
					Player->magic -= FIRE_SPELL_COST;
					BadGuyHealth -= FireSpellDamage[BadGuyIndex]+random(10);
					zap();
					break;
				}
				
				case 'l':
				case 'L':
				{
					for (unsigned int i=0; i < NOTE_COUNT_light; i++)
					{
						playNote(&light[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					eputs("ZAP!");
					Player->magic -= LIGHTNING_SPELL_COST;
					BadGuyHealth -= LightningSpellDamage[BadGuyIndex]+random(10);
					zap();
					
					break;
				}
				
				case '1':
				{
					for (unsigned int i=0; i < NOTE_COUNT_sword; i++)
					{
						playNote(&sword[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					dmg = WeaponDamage+(Player->Weapon1<<2)+BadGuyIndex;
					eputs("Take that!");
					BadGuyHealth -= *dmg + random(Player->strength);
					setStrength(Player,Player->strength-1);
					
					break;
				}
				
				case '2':
				{
					for (unsigned int i=0; i < NOTE_COUNT_sword; i++)
					{
						playNote(&sword[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					dmg = WeaponDamage+(Player->Weapon2<<2)+BadGuyIndex;
					eputs("Take that!");
					BadGuyHealth -= *dmg + random(Player->strength);
					setStrength(Player,Player->strength-1);
					
					break;
				} 
				
				case 'p':
				case 'P':
				{
					for (unsigned int i=0; i < NOTE_COUNT_punch; i++)
					{
						playNote(&punch[i]);
						GPIOA->ODR ^= 1; 
						GPIOA->ODR ^= 2;
					}
					
					eputs("Thump!");
					BadGuyHealth -= 1+random(Player->strength);
					setStrength(Player,Player->strength-1);
					break;
				}
				
				default: 
				{	
					eputs("You fumble. Uh oh");
				}
				
			} // end switch
			
			// Bad guy then gets a go 
			
			if (BadGuyHealth < 0)
				BadGuyHealth = 0;
			Damage = (uint8_t)(BadGuyDamage[BadGuyIndex]+(int)random(5));
			setHealth(Player,Player->health - Damage);
			eputs("Health: you "); printDecimal(Player->health);
			eputs(", them " );printDecimal((uint32_t)BadGuyHealth);
			eputs("\r\n");
			
		} // end while
		
		if (Player->health == 0)
		{ // You died
			playTune(0,0,0);
			
			for (unsigned int i=0; i < NOTE_COUNT_lost; i++)
			{
					playNote(&lost[i]);
					GPIOA->ODR ^= 1; 
					GPIOA->ODR ^= 2;
			}
				
			eputs("You are dead. Press Reset to restart");
			while(1);
			
		} // end if
		
		else
		{ // You won!
			playTune(0,0,0);
			for (unsigned int i=0; i < NOTE_COUNT_won; i++)
			{
					playNote(&won[i]);
					GPIOA->ODR ^= 1; 
					GPIOA->ODR ^= 2;
			}
			
			playTune(cave, NOTE_COUNT_cave, 1);
			for (unsigned int i=0; i < NOTE_COUNT_cave; i++)
			{
					playNote(&cave[i]);
			}
			
			Player->wealth = (uint8_t)(50 + random(50));			
			showGameMessage("You win! Their gold is yours");			
			
			return 1;
		} // end else
		
	} // end outer if
	
	else
	{
		showGameMessage("Our 'hero' chickens out");
		
		return 0;
	} // end else

} // end doChallenge()

int addWeapon(tPlayer *Player, int Weapon)
{
	char c;
	eputs("You stumble upon ");
	
	for (unsigned int i=0; i < NOTE_COUNT_weapon; i++)
	{
		playNote(&weapon[i]);
		GPIOA->ODR ^= 1; 
		GPIOA->ODR ^= 2;
	}
				
	switch (Weapon)
	{
		case 1:
		{	
			eputs("a mighty axe");
			break;
		}
		case 2:
		{	
			eputs("a sword with mystical runes");
			break;
		}
		case 3:
		{	
			eputs("a bloody flail");
			break;
		}		
		default:
			printDecimal((uint32_t)Weapon);
	}
	if ( (Player->Weapon1) && (Player->Weapon2) )
	{
		// The player has two weapons already.
		showPlayer(Player);
		eputs("You already have two weapons\r\n");		
		eputs("(1) drop Weapon1, (2) for Weapon2, (0) skip");
		c = getUserInput();
		eputchar(c);
		switch(c)
		{
			case '0':{
				return 0; // don't pick up
			}
			case '1':{
				Player->Weapon1 = (uint8_t)Weapon;
				break;
			}
			case '2':{
				Player->Weapon2 = (uint8_t)Weapon;
				break;
			}
		}
	}
	else
	{
		if (!Player->Weapon1)
		{
			Player->Weapon1 = (uint8_t)Weapon;	
		}
		else if (!Player->Weapon2)
		{
			Player->Weapon2 = (uint8_t)Weapon;
		}
	}	
	return 1;
} // end addWeapon

const char *getWeaponName(int index)
{
	switch (index)
	{
		case 0:return "Empty"; 
		case 1:return "Axe";
		case 2:return "Sword"; 
		case 3:return "Flail"; 
	}
	return "Unknown";
} // end getWeaponName
void setHealth(tPlayer *Player,int health)
{
	if (health > 100)
		health = 100;
	if (health < 0)
		health = 0;
	Player->health = (uint8_t)health;
} // end setHealth	

void setStrength(tPlayer *Player, byte strength)
{
	if (strength > 100)
		strength = 100;
	Player->strength = strength;
} // end setStrength

void initPlayer(tPlayer *Player,tRealm *Realm)
{
	// get the player name
	int index=0;
	byte x,y;
	char ch=0;
	// Initialize the player's attributes
	eputs("Enter the player's name: ");
	while ( (index < MAX_NAME_LEN) && (ch != '\n') && (ch != '\r'))
	{
		ch = getUserInput();
		if ( ch > '0' ) // strip conrol characters
		{
			
			Player->name[index++]=ch;
			eputchar(ch);
		}
	}
	Player->name[index]=0; // terminate the name
	setHealth(Player,100);
	Player->strength=(uint8_t)(50+random(50));
	Player->magic=(uint8_t)(50+random(50));	
	Player->wealth=(uint8_t)(10+random(10));
	Player->Weapon1 = 0;
	Player->Weapon2 = 0;
	// Initialize the player's location
	// Make sure the player does not land
	// on an occupied space to begin with
	do {
		x=(uint8_t)random(MAP_WIDTH);
		y=(uint8_t)random(MAP_HEIGHT);
		
	} while(Realm->map[y][x] != '.');
	Player->x=x;
	Player->y=y;
	
	for (unsigned int i=0; i < NOTE_COUNT_aButton; i++)
	{
		playNote(&aButton[i]);
		GPIOA->ODR ^= 1; 
		GPIOA->ODR ^= 2;
	}
				
}// end initPlayer

void showPlayer(tPlayer *Player)
{
	eputs("\r\nName: ");
	eputs(Player->name);
	eputs("health: ");
	printDecimal(Player->health);
	eputs("\r\nstrength: ");
	printDecimal(Player->strength);
	eputs("\r\nmagic: ");
	printDecimal(Player->magic);
	eputs("\r\nwealth: ");
	printDecimal(Player->wealth);	
	eputs("\r\nLocation : ");
	printDecimal(Player->x);
	eputs(" , ");
	printDecimal(Player->y);	
	eputs("\r\nWeapon1 : ");
	eputs(getWeaponName(Player->Weapon1));
	eputs(" Weapon2 : ");
	eputs(getWeaponName(Player->Weapon2));
} // end showPlayer

void initRealm(tRealm *Realm)
{
	unsigned int x,y;
	unsigned int Rnd;
	// clear the map to begin with
	for (y=0;y < MAP_HEIGHT; y++)
	{
		for (x=0; x < MAP_WIDTH; x++)
		{
			Rnd = random(100);
			
			if (Rnd >= 98) // put in some baddies
				Realm->map[y][x]=	Baddies[random(sizeof(Baddies))];
			else if (Rnd >= 95) // put in some good stuff
				Realm->map[y][x]=	FindTypes[random(sizeof(FindTypes))];
			else if (Rnd >= 90) // put in some rocks
				Realm->map[y][x]='*'; 
			else // put in empty space
				Realm->map[y][x] = '.';	
		}
	}
	
	// finally put the exit to the next level in
	x = random(MAP_WIDTH);
	y = random(MAP_HEIGHT);
	Realm->map[y][x]='X';
}	// end initRealm

void showRealm(tRealm *Realm,tPlayer *Player)
{
	int x,y;
	eputs("\r\nThe Realm:\r\n");	
	for (y=0;y<MAP_HEIGHT;y++)
	{
		for (x=0;x<MAP_WIDTH;x++)
		{
			
			if ( (x==Player->x) && (y==Player->y))
				eputchar('@');
			else
				eputchar(Realm->map[y][x]);
		}
		eputs("\r\n");
	}
	eputs("\r\nLegend\r\n");
	eputs("(T)roll, (O)gre, (D)ragon, (H)ag, e(X)it\r\n");
	eputs("(w)eapon, (g)old), (m)agic, (s)trength\r\n");
	eputs("@=You\r\n");
} // end showRealm

void showHelp()
{

	eputs("Help\r\n");
	eputs("N,S,E,W : go North, South, East, West\r\n");
	eputs("# : show map (cost: 1 gold piece)\r\n");
	eputs("(H)elp\r\n");
	eputs("(P)layer details\r\n");
	
} // end showHelp

void showGameMessage(char *Msg)
{
	eputs(Msg);
	eputs("\r\nReady\r\n");	
} // end showGameMessage

char getUserInput()
{
	char ch = 0;
	
	while (ch == 0)
		ch = egetchar();
	return ch;
} // end getUserInput

unsigned random(unsigned range)
{
	// Implementing my own version of modulus
	// as it is a lot smaller than the library version
	// To prevent very long subtract loops, the
	// size of the value returned from prbs has been
	// restricted to 8 bits.
	unsigned Rvalue = (prbs()&0xff);
	while (Rvalue >= range)
		Rvalue -= range; 
	return Rvalue;
} // end unsigned random

void zap()
{

} // end zap

void ADCBegin()
{
	RCC->APB2ENR |= (1u << 9); // Turn on ADC
	RCC->IOPENR |= 1; // enable GPIOA
	pinMode(GPIOA,4,3); // Make GPIOA_4 an analogue input
	ADC1->CR = 0; // disable ADC before making changes
	ADC1->CR |= (1u << 28); // turn on the voltage regulator
	ADC1->CR |= (1u << 31); // start calibration
	while ( (ADC1->CR & (1u << 31)) != 0); // Wait for calibration to complete.
	ADC1->CHSELR = (1 << 8); // select channel4  
	ADC1->CR |= 1; // enable the ADC
} // end ADCBegin

uint16_t ADCRead(void)
{
	ADC1->CR |= (1 << 2); // start a conversion
	while ( (ADC1->CR & (1 << 2)) != 0); // Wait for conversion to complete.
	return (uint16_t)ADC1->DR;
} // end ADCRead

static unsigned long shift_register=0;

void randomize()
{
	// To generate a "true" random seed for the prbs generator
	// Repeatedly read a floating analogue input (32 times)
	// and the least significant bit of that read to set each
	// bit of the prbs shift registers.
	// Must make sure that all of its buts are not zero when
	// done.
	RCC->IOPENR |= (1 << 1); // enable port B
	pinMode(GPIOB,0,3); // use PB0 as the source of noise.  This ADC channel 8
	ADCBegin();
	shift_register = 0;
	while(shift_register == 0)
	{
		for (int i=0; i < 32; i++)
		{
			shift_register = shift_register << 1;
			shift_register |= (ADCRead() & 1);
			delay(10000);
			printDecimal(shift_register);
			eputs("\r\n");
		}
	}
} // end randomize

uint32_t prbs()
{
	// This is an unverified 31 bit PRBS generator
	// It should be maximum length but this has not been verified

	unsigned long new_bit=0;

			new_bit = (shift_register >> 27) ^ (shift_register >> 30);
			new_bit = new_bit & 1;
			shift_register=shift_register << 1;
			shift_register=shift_register | (new_bit);

	return shift_register & 0x7fffffff; // return 31 LSB's
} // end prbs


void playTune(const note *n, int notecount, int repeat)
{
	TuneLoop = repeat;
	Tune = n;
	TuneNoteCount = notecount;
}

void playNote(note *n)
{
	uint32_t Period = 1000000 / n->frequency;
	TIM2->CR1 |= 1; // enable counter 
	TIM2->ARR = Period; // Set initial reload value of 1000 microseconds
	TIM2->CCR2 = TIM2->ARR/2; // 50% duty;
	SoundDuration = n->duration;
	while(SoundDuration != 0); // Wait
	TIM2->ARR = 1;
	TIM2->CR1 |= 0; // enable counter 
	delayms(n->wait);

} // end playNote

/* OLD SYSTICK_HANDLER
void SysTick_Handler(void)
{
	
	if (SoundDuration > 0)
	{
		SoundDuration --;
	}
	ms_counter ++;   // another millisecond has passed

} // end sysTick_Handler
*/

/* NEW SYSTICK HANDLER */
void SysTick_Handler(void)
{
	static int CurrentNote = 0;
	
	// if sound duration > 0
	if (SoundDuration > 0)
	{
		// decrement it
		SoundDuration --;
	}
	
	// if sound duration has reached 0
	else
	{
		
		CurrentNote++;
		
		// check if current note is NOT the last note of the array
		if (CurrentNote < TuneNoteCount)
		{
			// play tune at that current note
			playNote(&Tune[CurrentNote]);
		}

		// if end of tune array
		else
		{
			// is tune meant to loop?
			if (TuneLoop)
			{
				// reset back to the note	
				CurrentNote = 0;
			}
			else
			{
				// stop the sound
				stopSound();
			}
		}
	}

	ms_counter ++; // another millisecond has passed
} // end systick()

void stopSound()
{
	TIM2->CR1 = 0; // disable counter
}

void delayms(uint32_t dly)
{
	dly = dly+ms_counter;
	while(ms_counter < dly);
} // end delayMs
