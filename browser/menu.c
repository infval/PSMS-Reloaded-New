#include <stdio.h>
#include <fileio.h>
#include <io_common.h>
#include <sys/stat.h>
#include <libpad.h>
#include <dmaKit.h>
#include <gsKit.h>
#include <loadfile.h>
#include <libpwroff.h>
#include <kernel.h>
#include "browser.h"
#include "ps2font.h"
#include "bdraw.h"
#include "cnfsettings.h"
#include "sjpcm.h"
#include "shared.h"
#include "psms.h"


//Settings
extern vars Settings;
//Skin
extern skin FCEUSkin;
extern u8 menutex;
extern u8 bgtex;
//Input
extern char control_name[256];
extern char path[4096];
//extern char partitions[4][256];
extern u8 h;
extern int snd_sample;

#define SND_RATE	48000

int statenum = 0;
u8 power_off = 0;

/************************************/
/* gsKit Variables                  */
/************************************/
extern GSGLOBAL *gsGlobal;
extern GSTEXTURE BG_TEX;
extern GSTEXTURE MENU_TEX;

/************************************/
/* Pad Variables                    */
/************************************/
extern u32 old_pad[2];
struct padButtonStatus buttons[2];
int aorborab[2] = { 0, 0 };
//int rapidfire_a[2] = { 0, 0 };
//int rapidfire_b[2] = { 0, 0 };
//extern u8 fdsswap;

/************************************/
/* Browser and Emulator Variables   */
/************************************/
extern s8 selected;
extern int oldselect;
extern u8 selected_dir;
//extern u8 exitgame;
extern int FONT_HEIGHT;
extern int sound;
extern int endflag;


//void font_print(GSGLOBAL *gsGlobal, float X, float Y, int Z, unsigned long color, char *String);

static inline char* strzncpy(char *d, char *s, int l) {
	d[0] = 0;
	return strncat(d, s, l);
}

//void menu_background(float x1, float y1, float x2, float y2, int z) {
//	int thickness = 3;
//
//	//border
//	gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); //top
//	gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); //left
//	gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); //right
//	gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); //bottom
//
//	//background
//	gsKit_prim_quad_gouraud(gsGlobal, x1+thickness, y1+thickness,
//			x2-thickness, y1+thickness,
//			x1+thickness, y2-thickness,
//			x2-thickness, y2-thickness,
//			z+1,
//			FCEUSkin.bgColor1, FCEUSkin.bgColor2,
//			FCEUSkin.bgColor3, FCEUSkin.bgColor4);
//
//}

//void menu_bgtexture(GSTEXTURE *gsTexture, float x1, float y1, float x2,
//		float y2, int z) {
//	int thickness = 3;
//
//	//border
//	gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); //top
//	gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); //left
//	gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); //right
//	gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); //bottom
//
//	gsKit_prim_sprite_texture( gsGlobal, gsTexture,
//			x1+thickness, /* X1 */
//			y1+thickness, /* Y1 */
//			0.0f, /* U1 */
//			0.0f, /* V1 */
//			x2-thickness, /* X2 */
//			y2-thickness, /* Y2 */
//			gsTexture->Width, /* U2 */
//			gsTexture->Height, /* V2*/
//			z+1, /* Z */
//			GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
//	);
//}

void menu_primitive(char *title, GSTEXTURE *gsTexture, float x1, float y1,
		float x2, float y2) {

	if (!menutex || !bgtex) {
		menu_bgtexture(gsTexture, x1, y1, x2, y2, 1);
	} else {
		menu_background(x1, y1, x2, y2, 1);
	}
	menu_background(x2-(strlen(title)*12), y1, x2, y1+FONT_HEIGHT*2, 2);

	printXY(title, x2-(strlen(title)*10), y1+FONT_HEIGHT/2, 3,
			FCEUSkin.textcolor, 2, 0);
}

//void browser_primitive(char *title1, char *title2, GSTEXTURE *gsTexture,
//		float x1, float y1, float x2, float y2) {
//
//	if (!menutex || !bgtex) {
//		menu_bgtexture(gsTexture, x1, y1, x2, y2, 1);
//	} else {
//		menu_background(x1, y1, x2, y2, 1);
//	}
//	menu_background(x1, y1, x1+(strlen(title1)*9), y1+FONT_HEIGHT*2, 2);
//	menu_background(x2-(strlen(title2)*12), y1, x2, y1+FONT_HEIGHT*2, 2);
//
//	printXY(title1, x1+(strlen(title2)+4), y1+FONT_HEIGHT/2, 3,
//			FCEUSkin.textcolor, 2, 0);
//	printXY(title2, x2-(strlen(title2)*10), y1+FONT_HEIGHT/2, 3,
//			FCEUSkin.textcolor, 2, 0);
//}

int menu_input(int port, int center_screen) {
	int ret[2];
	u32 paddata[2];
	u32 new_pad[2];
	u16 slot = 0;

	int change = 0;

	//check to see if pads are disconnected
	ret[port]=padGetState(0, slot);
	if ((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
		if (ret[port]==PAD_STATE_DISCONN) {
			printf("Pad(%d, %d) is disconnected\n", 0, slot);
		}
		ret[port]=padGetState(0, slot);
	}
	ret[port] = padRead(0, slot, &buttons[port]); // port, slot, buttons
	if (ret[port] != 0) {
		paddata[port]= 0xffff ^ buttons[port].btns;
		new_pad[port] = paddata[port] & ~old_pad[port]; // buttons pressed AND NOT buttons previously pressed
		old_pad[port] = paddata[port];

		if (paddata[port] & PAD_LEFT && center_screen) {
			Settings.offset_x--;
			change = 1;
		}
		if (new_pad[port] & PAD_DOWN && !center_screen) {
			change = 1;
		}
		if (paddata[port] & PAD_DOWN && center_screen) {
			Settings.offset_y++;
			change = 1;
		}
		if (paddata[port] & PAD_RIGHT && center_screen) {
			Settings.offset_x++;
			change = 1;
		}
		if (new_pad[port] & PAD_UP && !center_screen) {
			change = -1;
		}
		if (paddata[port] & PAD_UP && center_screen) {
			Settings.offset_y--;
			change = 1;
		}
		if (new_pad[port] & PAD_START && center_screen) {
			change = 2;
		}
		if (new_pad[port] & PAD_SELECT && center_screen) {
			Settings.offset_x = 0;
			Settings.offset_y = 0;
			change = 1;
		}
		if (new_pad[port] & PAD_CIRCLE) {

		}
		if (new_pad[port] & PAD_CROSS) {
			selected = 1;
		}
		if ((new_pad[port] == PAD_TRIANGLE || (new_pad[port] == Settings.PlayerInput[port][9] && Settings.PlayerInput[port][9] != PAD_CROSS))
			&& !center_screen) {
			selected = 2;
		}
	}
	if ((center_screen && change) || (center_screen == 2)) {
		SetDisplayOffset();

		gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00,0x00,0x00,0x80,0x00));

		menu_primitive("Centering", &BG_TEX, 0, 0, gsGlobal->Width, gsGlobal->Height);

		DrawScreen(gsGlobal);

		center_screen = 1;
	}
	return change;
}

int Browser_Menu(void) {
	char cnfpath[2048];
	int i, selection = 0;
	oldselect = -1;
	int option_changed = 0;

	int menu_x1 = gsGlobal->Width  * 0.25;
	int menu_y1 = gsGlobal->Height * 0.25;
	int menu_x2 = gsGlobal->Width  * 0.75;
	int menu_y2 = gsGlobal->Height * 0.75 + FONT_HEIGHT;
	int text_line = menu_y1 + 40;

	char options[11][39] = {
		{ "Display: " },
		{ "Interlacing: " }, 
		{ "Center Screen" },
		{ "Configure Save Path: " }, 
		{ "" },
		{ "Configure ELF Path:  " },
		{ "" }, 
		{ "Save PSMS.CNF" },
		{ "Power Off" },
		{ "Exit to ELF" },
		{ "Exit Options Menu" }
	};
	char options_state[11][64] = { { 0 } };
	char options_buffer[39+64] = { 0 };

	//fill lines with values
	for (i=0; i<11; i++) {
		switch (i) {
		case 0:
			if (!Settings.display) {
				strcpy(options_state[i], "NTSC");
			} else {
				strcpy(options_state[i], "PAL");
			}
			break;
		case 1:
			if (Settings.interlace) {
				strcpy(options_state[i], "On");
			} else {
				strcpy(options_state[i], "Off");
			}
			break;
		case 4:
			strzncpy(options_state[4], Settings.savepath, 38);
			break;
		case 6:
			strzncpy(options_state[6], Settings.elfpath, 38);
			break;
		}
	}

	while (1) {
		selected = 0; //clear selected flag
		selection += menu_input(0, 0);

		if (selection > 10) {
			selection = 0;
		}
		if (selection < 0) {
			selection = 10;
		}
		if (selection == 4 && oldselect == 3) {
			selection++;
		} //5 is savepath
		if (selection == 4 && oldselect == 5) {
			selection--;
		}
		if (selection == 6 && oldselect == 5) {
			selection++;
		} //7 is elfpath
		if (selection == 6 && oldselect == 7) {
			selection--;
		}

		if ((oldselect != selection) || option_changed) {

			gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00,0x00,0x00,0x80,0x00));

			menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2,
					menu_y2);

			for (i=0; i<11; i++) {
				strcpy(options_buffer, options[i]);
				strcat(options_buffer, options_state[i]);
				if (selection == i) {
					//font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
					printXY(options_buffer, menu_x1+10, text_line+i*FONT_HEIGHT, 4,
							FCEUSkin.highlight, 1, 0);
				} else {
					//font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
					printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT,
							4, FCEUSkin.textcolor, 1, 0);
				}
			}

			DrawScreen(gsGlobal);

			if (power_off)
				option_changed = 1;
			power_off--;
			if (!power_off) {
				strcpy(cnfpath, "xyz:/imaginary/hypothetical/doesn't.exist");
				FILE *File;
				File = fopen(cnfpath, "r");
				if (File != NULL)
					fclose(File);
			}
		}

		oldselect = selection;
		option_changed = 0;

		if (selected) {
			if (selected == 2) {
				selection = 10;
			}
			i = selection;
			switch (i) {
			case 0: //Display PAL/NTSC
				Settings.display ^= 1;
				
				if (Settings.display) {
					gsGlobal->Mode = GS_MODE_PAL;
					gsGlobal->Height = 512;
					snd_sample = SND_RATE / 50;
					strcpy(options_state[i], "PAL");
				} else {
					gsGlobal->Mode = GS_MODE_NTSC;
					gsGlobal->Height = 448;
					snd_sample = SND_RATE / 60;
					strcpy(options_state[i], "NTSC");
				}

				init_custom_screen();

				menu_x1 = gsGlobal->Width  * 0.25;
				menu_y1 = gsGlobal->Height * 0.25;
				menu_x2 = gsGlobal->Width  * 0.75;
				menu_y2 = gsGlobal->Height * 0.75 + FONT_HEIGHT;
				text_line = menu_y1 + 40;
				option_changed = 1;
				break;
			case 1: //Interlacing Off/On
				Settings.interlace ^= 1;
				if (Settings.interlace) {
					gsGlobal->Interlace = GS_INTERLACED;
					gsGlobal->Field = GS_FIELD;
					strcpy(options_state[i], "On");
				} else {
					gsGlobal->Interlace = GS_NONINTERLACED;
					gsGlobal->Field = GS_FRAME;
					strcpy(options_state[i], "Off");
				}

				init_custom_screen();

				menu_x1 = gsGlobal->Width  * 0.25;
				menu_y1 = gsGlobal->Height * 0.25;
				menu_x2 = gsGlobal->Width  * 0.75;
				menu_y2 = gsGlobal->Height * 0.75 + FONT_HEIGHT;
				text_line = menu_y1 + 40;
				option_changed = 1;
				break;
			case 2: //Center Screen
				while (menu_input(0, 2) != 2) {
				}
				i = 0x10000;
				while (i--)
					asm("nop\nnop\nnop\nnop");
				option_changed = 1;
				break;
			case 3: //Configure Save Path
				h = 0; //reset browser
				selection = 0;
				oldselect = -1;
				selected = 0;
				strcpy(path, "path"); //end reset browser
				strcpy(Settings.savepath, Browser(0, 1, 0));
				printf("%s", Settings.savepath);
				strzncpy(options_state[4], Settings.savepath, 38);
				selected_dir = 0;
				h = 0;
				selection = 0;
				oldselect = -1;
				strcpy(path, "path");
				option_changed = 1;
				selected = 0;
				break;
			case 5: //Configure ELF Path
				h = 0;
				selection = 0;
				oldselect = -1;
				selected = 0;
				strcpy(path, "path");
				strcpy(Settings.elfpath, Browser(1, 2, 0));
				strzncpy(options_state[6], Settings.elfpath, 38);
				h = 0;
				selection = 0;
				oldselect = -1;
				strcpy(path, "path");
				option_changed = 1;
				selected = 0;
				break;
			case 7: //Save CNF
				fioMkdir("mc0:PSMS");
				Save_Global_CNF("mc0:/PSMS/PSMS.CNF");
				break;
			case 8: //Power Off
				poweroffShutdown();
				if (Settings.display)
					power_off = 50/4;
				else
					power_off = 60/4;
				option_changed = 1;
				break;
			case 9: //Exit to ELF
				return 2;
			case 10: //Exit Options Menu
				selected = 0;
				return 1;

			}
		}
	}
}

void setupSMSGS(void);

static void Ingame_Menu_Controls();

#define INGAME_MENU_N 10
#define INGAME_MENU_EXIT_I 8

void Ingame_Menu(void) {
	int i, selection = 0;
	oldselect = -1;
	char stateoption[16];
	strcpy(stateoption, "State number: ");

	int option_changed = 0;

	int menu_x1 = gsGlobal->Width  * 0.25;
	int menu_y1 = gsGlobal->Height * 0.25;
	int menu_x2 = gsGlobal->Width  * 0.75;
	int menu_y2 = gsGlobal->Height * 0.75 + FONT_HEIGHT;

	int text_line = menu_y1 + 4 + FONT_HEIGHT;

	char options[INGAME_MENU_N][23] = {
		{ "State number: " },
		{ "Save State" },
		{ "Load State" },
		{ "Filtering: " },
		{ "Sprite Limit: " },
		{ "Configure Input >" },
		{ "Region: " },
		{ "Reset Game" },
		{ "Exit Menu" },
		{ "Exit Game" }
	};
	char options_state[INGAME_MENU_N][32] = { { 0 } };
	char options_buffer[23+32] = { 0 };

	for (i=0; i<INGAME_MENU_N; i++) {
		switch (i) {
		case 0:
			sprintf(options_state[i], "%d", statenum);
			break;
		case 3:
			if (!Settings.filter)
				strcpy(options_state[i], "Off");
			else
				strcpy(options_state[i], "On");
			break;
		case 4:
			if (Settings.sprite_limit)
				strcpy(options_state[i], "On");
			else
				strcpy(options_state[i], "Off");
			break;
		case 6:
			if (sms.country == TYPE_OVERSEAS)
				strcpy(options_state[i], "US/EUR");
			else
				strcpy(options_state[i], "Japan");
			break;
		}
	}

	gsKit_mode_switch(gsGlobal, GS_ONESHOT);
	gsGlobal->DrawOrder = GS_PER_OS;

	while (1) {
		selected = 0; //clear selected flag
		selection += menu_input(0, 0);

		if (selection >= INGAME_MENU_N) { selection = 0; }
		if (selection < 0) { selection = INGAME_MENU_N - 1; }

		if (oldselect != selection || option_changed) {
			i = 0x10000;
			while (i--)
				asm("nop\nnop\nnop\nnop");
			gsKit_queue_reset(gsGlobal->Os_Queue);

			option_changed = 0;

			menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2,
					menu_y2);

			for (i=0; i<INGAME_MENU_N; i++) {
				strcpy(options_buffer, options[i]);
				strcat(options_buffer, options_state[i]);
				if (selection == i) {
					//font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
					printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT,
							4, FCEUSkin.highlight, 1, 0);
				} else {
					//font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
					printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT,
							4, FCEUSkin.textcolor, 1, 0);
				}
			}

			DrawScreen(gsGlobal);
		}

		oldselect = selection;

		if (selected) {
			if (selected == 2) { //menu combo pressed again
				selection = INGAME_MENU_EXIT_I;
			}
			i = selection;
			switch (i) {
			case 0: //State Number
				statenum++;
				if (statenum >= 10) {
					statenum = 0;
				}
				sprintf(options_state[i], "%d", statenum);
				//FCEUI_SelectState(statenum);
				option_changed = 1;
				break;
			case 1:
				psms_save_state(statenum);
				setupSMSGS();
				return;
			case 2:
				psms_load_state(statenum);
				setupSMSGS();
				return;
			case 3:
				Settings.filter ^= 1;
				if (Settings.filter) {
					strcpy(options_state[i], "On");
				} else {
					strcpy(options_state[i], "Off");
				}
				option_changed = 1;
				break;
			case 4:
				Settings.sprite_limit ^= 1;
				if (Settings.sprite_limit)
					strcpy(options_state[i], "On");
				else
					strcpy(options_state[i], "Off");
				vdp.limit = Settings.sprite_limit;
				option_changed = 1;
				break;
			case 5:
				Ingame_Menu_Controls();
				break;
			case 6:
				sms.country ^= 1;
				if (sms.country == TYPE_OVERSEAS) {
					strcpy(options_state[i], "US/EUR");
				} else {
					strcpy(options_state[i], "Japan");
				}
				option_changed = 1;
				break;
			case 7:
				system_reset();
				if(sound) {
					SjPCM_Clearbuff();
					SjPCM_Play();
				}
				setupSMSGS();
				return;
			case 8:
				setupSMSGS();
				return;
			case 9:
				statenum = 0;
				endflag = 1;
				selected = 0;
				return;
			}
		}
	}
}

static int menu_input_controls(int port, int is_changing_button, u32 *new_button, int *left_right)
{
	int ret[2];
	u32 paddata[2];
	u32 new_pad[2];
	u16 slot = 0;

	int change = 0;
	*left_right = 0;

	// Check to see if pads are disconnected
	ret[port] = padGetState(0, slot);
	if ((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
		if (ret[port] == PAD_STATE_DISCONN) {
			printf("Pad(%d, %d) is disconnected\n", 0, slot);
		}
		ret[port] = padGetState(0, slot);
	}
	ret[port] = padRead(0, slot, &buttons[port]); // port, slot, buttons
	if (ret[port] != 0) {
		paddata[port]= 0xffff ^ buttons[port].btns;
		new_pad[port] = paddata[port] & ~old_pad[port]; // Buttons pressed AND NOT buttons previously pressed
		old_pad[port] = paddata[port];

		if (new_pad[port]) {
			if (is_changing_button) {
				*new_button = new_pad[port];
				selected = 1;
			}
			else {
				if (new_pad[port] & PAD_UP) {
					change = -1;
				}
				if (new_pad[port] & PAD_DOWN) {
					change = 1;
				}
				if (new_pad[port] & PAD_LEFT) {
					*left_right = -1;
				}
				if (new_pad[port] & PAD_RIGHT) {
					*left_right = 1;
				}
				if (new_pad[port] & PAD_CROSS) {
					selected = 1;
				}
				if (new_pad[port] == PAD_TRIANGLE) {
					selected = 2;
				}
			}
		}
	}
	return change;
}

static void padbuttonToStr(u16 button, char button_name[9])
{
	if (button == 0) {
		strcpy(button_name, "---");
		return;
	}
	int i;
	for (i = 0; i < 16; i++) {
		if (button & (1 << i)) {
			break;
		}
	}
	char *buttons[16] = {
		"Select", "L3"   , "R3"  , "Start",
		"Up       \xFF""=",
		"Right    \xFF"":",
		"Down     \xFF"";",
		"Left     \xFF""<" ,
		"L2"    , "R2"   , "L1"  , "R1"   ,
		"Triangle \xFF""3",
		"Circle   \xFF""0",
		"Cross    \xFF""1",
		"Square   \xFF""2"
	};
	strcpy(button_name, buttons[i]);
}

#define CONTROLS_N 13
#define CONTROLS_BUTTON_N 10
#define CONTROLS_OFFSET (CONTROLS_N - CONTROLS_BUTTON_N)
#define CONTROLS_MENU_BUTTON_I 9

static void Ingame_Menu_Controls()
{
	int i, b, selection = 0;
	oldselect = -1;

	int option_changed = 0;

	int menu_x1 = gsGlobal->Width  * 0.25;
	int menu_y1 = gsGlobal->Height * 0.25;
	int menu_x2 = gsGlobal->Width  * 0.75;
	int menu_y2 = gsGlobal->Height * 0.75 + FONT_HEIGHT;

	int text_line = menu_y1 + 4 + FONT_HEIGHT;

	char options[CONTROLS_N][32] = {
		{ "< Back" },
		{ "Autofire Pattern: "},
		{ "Player: "},
		{ "  Pause | " },
		{ "     Up | " },
		{ "   Down | " },
		{ "   Left | " },
		{ "  Right | " },
		{ "Button1 | " },
		{ "Button2 | " },
		{ "Turbo 1 | " },
		{ "Turbo 2 | " },
		{ "   Menu | " }
	};
	char options_state[CONTROLS_N][16] = { { 0 } };
	char options_buffer[32+16] = { 0 };

	int player = 0;
	int is_changing_button = 0;
	u32 new_button = 0;
	int left_right = 0;

	//sprintf(options_state[1], "1 on, %d off", Settings.autofire_pattern + 1);
	sprintf(options_state[1], "%d on, %d off", (Settings.autofire_pattern>>3) + 1, (Settings.autofire_pattern&7) + 1);
	strcpy(options_state[2], "1");
	for (i = 0; i < CONTROLS_BUTTON_N; i++) {
		padbuttonToStr(Settings.PlayerInput[player][i], options_state[i + CONTROLS_OFFSET]);
	}

	while (1) {
		selected = 0; // Clear selected flag
		selection += menu_input_controls(0, is_changing_button, &new_button, &left_right);

		if (selection >= CONTROLS_N) { selection =  0; }
		if (selection < 0) { selection = CONTROLS_N - 1; }

		if (oldselect != selection || option_changed) {
			i = 0x10000;
			while (i--) asm("nop\nnop\nnop\nnop");
			gsKit_queue_reset(gsGlobal->Os_Queue);

			option_changed = 0;

			menu_primitive("Controls", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

			for (i = 0; i < CONTROLS_N; i++) {
				strcpy(options_buffer, options[i]);
				strcat(options_buffer, options_state[i]);
				if (selection == i) {
					printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
				}
				else {
					printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
				}
			}

			DrawScreen(gsGlobal);
		}

		oldselect = selection;

		if (selected || left_right) {
			if (selected == 2) {
				return;
			}
			i = selection;

			if (i == 0) {
				if (!left_right) {
					return;
				}
			}
			else if (i == 1) {
				if (left_right >= 0) {
					Settings.autofire_pattern++;
				}
				else {
					Settings.autofire_pattern--;
				}
				if (Settings.autofire_pattern < 0) {
					Settings.autofire_pattern = 64 - 1;
				}
				else if (Settings.autofire_pattern >= 64) {
					Settings.autofire_pattern = 0;
				}
				sprintf(options_state[1], "%d on, %d off", (Settings.autofire_pattern>>3) + 1, (Settings.autofire_pattern&7) + 1);
				option_changed = 1;
			}
			else if (i == 2) {
				player++;
				if (player >= 2) {
					player = 0;
				}
				sprintf(options_state[i], "%d", player + 1);
				for (b = 0; b < CONTROLS_BUTTON_N; b++) {
					padbuttonToStr(Settings.PlayerInput[player][b], options_state[b + CONTROLS_OFFSET]);
				}
				option_changed = 1;
			}
			else {
				if (!left_right) {
					if (!is_changing_button) {
						strcpy(options_state[i], "<Press Button>");
					}
					else {
						i -= CONTROLS_OFFSET;

						Settings.PlayerInput[player][i] = (u16)new_button;
						// Resolve conflict
						for (b = 0; b < CONTROLS_BUTTON_N; b++) {
							if (b != i && Settings.PlayerInput[player][i] == Settings.PlayerInput[player][b]) {
								Settings.PlayerInput[player][b] = 0;
								padbuttonToStr(0, options_state[b + CONTROLS_OFFSET]);
								break;
							}
						}
						// Menu
						if (Settings.PlayerInput[player][CONTROLS_MENU_BUTTON_I] == 0) {
							u16 used_buttons = 0;
							for (b = 0; b < CONTROLS_BUTTON_N; b++) {
								used_buttons |= Settings.PlayerInput[player][b];
							}
							for (b = 0; b < 16; b++) {
								if (!(used_buttons & (1 << b))) {
									Settings.PlayerInput[player][CONTROLS_MENU_BUTTON_I] = 1 << b;
									padbuttonToStr(Settings.PlayerInput[player][CONTROLS_MENU_BUTTON_I], options_state[CONTROLS_MENU_BUTTON_I + CONTROLS_OFFSET]);
									break;
								}
							}
						}
						padbuttonToStr(Settings.PlayerInput[player][i], options_state[i + CONTROLS_OFFSET]);
					}
					is_changing_button ^= 1;
					option_changed = 1;
				}
			}
		}
	}
}
