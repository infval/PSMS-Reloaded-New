#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>
#include "browser.h"
#include "ps2font.h"

extern skin FCEUSkin;
extern vars Settings;
extern GSGLOBAL *gsGlobal;
extern u8 menutex;
extern u8 bgtex;

int defaultx;
int defaulty;

int FONT_HEIGHT = 16;

void menu_bgtexture(GSTEXTURE *gsTexture, float x1, float y1, float x2,
		float y2, int z) {
	int thickness = 3;

	//border
	gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); //top
	gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); //left
	gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); //right
	gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); //bottom

	gsKit_prim_sprite_texture( gsGlobal, gsTexture,
			x1+thickness, /* X1 */
			y1+thickness, /* Y1 */
			0.0f, /* U1 */
			0.0f, /* V1 */
			x2-thickness, /* X2 */
			y2-thickness, /* Y2 */
			gsTexture->Width, /* U2 */
			gsTexture->Height, /* V2*/
			z+1, /* Z */
			GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
	);
}

void DrawScreen(GSGLOBAL *gsGlobal)
{
    int i;

    i = 0x10000;
    while(i--) asm("nop\nnop\nnop\nnop");

    gsKit_sync_flip(gsGlobal);

    gsKit_queue_exec(gsGlobal);
}


void menu_background(float x1, float y1, float x2, float y2, int z) {
	int thickness = 3;

	//border
	gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); //top
	gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); //left
	gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); //right
	gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); //bottom

	//background
	gsKit_prim_quad_gouraud(gsGlobal, x1+thickness, y1+thickness,
			x2-thickness, y1+thickness,
			x1+thickness, y2-thickness,
			x2-thickness, y2-thickness,
			z+1,
			FCEUSkin.bgColor1, FCEUSkin.bgColor2,
			FCEUSkin.bgColor3, FCEUSkin.bgColor4);

}



void browser_primitive(char *title1, char *title2, GSTEXTURE *gsTexture,
		float x1, float y1, float x2, float y2) {

	if (!menutex || !bgtex) {
		menu_bgtexture(gsTexture, x1, y1, x2, y2, 1);
	} else {
		menu_background(x1, y1, x2, y2, 1);
	}
	menu_background(x1, y1, x1+(strlen(title1)*9), y1+FONT_HEIGHT*2, 2);
	menu_background(x2-(strlen(title2)*12), y1, x2, y1+FONT_HEIGHT*2, 2);

	printXY(title1, x1+(strlen(title2)+4), y1+FONT_HEIGHT/2, 3,
			FCEUSkin.textcolor, 2, 0);
	printXY(title2, x2-(strlen(title2)*10), y1+FONT_HEIGHT/2, 3,
			FCEUSkin.textcolor, 2, 0);
}



void normalize_screen(void) {
	GS_SET_DISPLAY1(gsGlobal->StartX, // X position in the display area (in VCK unit
			gsGlobal->StartY, // Y position in the display area (in Raster u
			gsGlobal->MagX, // Horizontal Magnification
			gsGlobal->MagY, // Vertical Magnification
			(gsGlobal->Width * 4) -1, // Display area width
			(gsGlobal->Height-1)); // Display area height

	GS_SET_DISPLAY2(gsGlobal->StartX, // X position in the display area (in VCK units)
			gsGlobal->StartY, // Y position in the display area (in Raster units)
			gsGlobal->MagX, // Horizontal Magnification
			gsGlobal->MagY, // Vertical Magnification
			(gsGlobal->Width * 4) -1, // Display area width
			(gsGlobal->Height-1)); // Display area height
}

void init_custom_screen(void) {
	
	if (Settings.display) {
		gsGlobal->Mode = GS_MODE_PAL;
		gsGlobal->Height = 512;
		defaulty = 72;
	} else {
		gsGlobal->Mode = GS_MODE_NTSC;
		gsGlobal->Height = 448; // 448;
		defaulty = 40;
	}
	
	gsGlobal->StartY = defaulty + Settings.offset_y;
	gsGlobal->StartX = defaultx + Settings.offset_x;

	if(Settings.interlace)
		gsGlobal->StartY = gsGlobal->StartY + 22;
	else
		gsGlobal->StartY = gsGlobal->StartY + 11;

	if(!Settings.interlace) {
		gsGlobal->Interlace = GS_NONINTERLACED;
		gsGlobal->Field = GS_FRAME;
		gsGlobal->StartY = gsGlobal->StartY/2 + 1;
	}

	normalize_screen();

	SetGsCrt(gsGlobal->Interlace, gsGlobal->Mode, gsGlobal->Field);
}


void SetupGSKit(void) {
	/* detect and set screentype */
	//gsGlobal = gsKit_init_global(GS_MODE_PAL);
	gsGlobal = gsKit_init_global_custom(GS_MODE_PAL,
			GS_RENDER_QUEUE_OS_POOLSIZE+GS_RENDER_QUEUE_OS_POOLSIZE/2,
			GS_RENDER_QUEUE_PER_POOLSIZE+GS_RENDER_QUEUE_PER_POOLSIZE/2);
	gsGlobal->Height = 512;

	defaultx = gsGlobal->StartX;
	defaulty = gsGlobal->StartY;

	/* initialize dmaKit */
	//dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8);
	dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
			D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

	dmaKit_chan_init(DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
	dmaKit_chan_init(DMA_CHANNEL_TOSPR);

	gsGlobal->DoubleBuffering = GS_SETTING_OFF;
	gsGlobal->ZBuffering = GS_SETTING_OFF;

	//640x448, ntsc, tv
	//640x512, pal, tv
	gsGlobal->Width = 640;

}

