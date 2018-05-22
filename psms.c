/*

	psms.c - PSMS Reloaded main source file. Contains PSMS core.

	                 (c) Nick Van Veen (aka Sjeep), 2002
	                 (c) Bruno Freitas (bootsector), 2008

	-------------------------------------------------------------------------

    This file is part of the PSMS.

    PSMS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PSMS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdlib.h>
#include <stdio.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <libmc.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <loadfile.h>
#include "shared.h"
#include "libpad.h"
#include "psms.h"
#include "sjpcm.h"
#include "browser.h"
#include "init.h"
#include "ps2font.h"
#include "pad.h"
#include "bdraw.h"
#include "cnfsettings.h"
#include "zlib.h"

void Ingame_Menu(void);

#define SND_RATE	48000

GSGLOBAL *gsGlobal;
extern skin FCEUSkin;
extern vars Settings;
GSTEXTURE SMSTEX;

u8 bitmap_data[256 * 256] __attribute__((aligned(128))) __attribute__ ((section (".bss"))); // SMS display is only 256x192, extra data is padding
//u16 clut[256] __attribute__((aligned(16))) __attribute__ ((section (".bss")));
u32 clut[256] __attribute__((aligned(128))) __attribute__ ((section (".bss")));

u8 base_rom[1048576] __attribute__((aligned(64))) __attribute__ ((section (".bss")));

extern char path[4096];
extern u32 old_pad[2];

u8 *state;
int state_saved = 0;

int whichdrawbuf = 0;
int endflag = 0;
int sound = 1;
int snd_sample = 0;

char rom_filename[1024];

int main()
{
	char *temp;

	init_machine();

	system_init(SND_RATE);
	
	setupSMSTexture();

	while(1) {

		strcpy(rom_filename,Browser(1,0,1));

		if(load_rom(rom_filename)) {
			continue; // if error loading file, start loop again (ie, go back to menu)
		}

		system_reset();
		
		setupSMSGS();
		
  		if(sound) {
			SjPCM_Clearbuff();
			SjPCM_Play();
		}

		while(1)
    	{
        	/* Get current input */
			update_input();

			if(endflag) break;

        	/* Run the SMS emulation for a frame */
        	sms_frame(0);

			if(sound) SjPCM_Enqueue(snd.buffer[0], snd.buffer[1], snd_sample, 1);

        	/* Update the display if desired */
			update_video();
    	}

		if(sound) SjPCM_Pause();

		psms_manage_sram(sms.sram, SRAM_SAVE);
		
		system_reset();
		endflag = 0;
		state_saved = 0;
		
		temp = strrchr(path,'/');
		temp++;
		*temp = 0;
	}
}


int init_machine()
{
	int i, sometime;

	InitPS2();
	setupPS2Pad();
	
    Default_Global_CNF();

    Load_Global_CNF("mc0:/PSMS/PSMS.CNF");
    
    if(Settings.display) // PAL
    	snd_sample = SND_RATE / 50;
    else // NTSC
    	snd_sample = SND_RATE / 60;

    for (i  = 0; i < 3; i++) {
        sometime = 0x10000;
        while(sometime--) asm("nop\nnop\nnop\nnop");
	}	
	
    SetupGSKit();

	gsKit_init_screen(gsGlobal); //initialize everything
	init_custom_screen(); //init user screen settings

	loadFont(0);
	
	Default_Skin_CNF();

	/* Initialize display bitmap */
    memset(&bitmap, 0, sizeof(t_bitmap));
    bitmap.width  = 256;
    bitmap.height = 192;
    bitmap.depth  = 8;
    bitmap.pitch  = (bitmap.width * (bitmap.depth >> 3));
    bitmap.data   = (unsigned char *)&bitmap_data[0];

	memset(&base_rom,0,1048576);

	state = malloc(sizeof(t_vdp) + sizeof(t_sms) + sizeof(Z80_Regs) + sizeof(int) + sizeof(t_SN76496) + 0x40);
	if(state == NULL) {
		display_error("Failed to allocate memory!", 1);
	}


	if(SjPCM_Init() < 0) display_error("SjPCM Bind failed!!", 1);

	
	return 0;
}

int load_rom(char* fname)
{
	char filename[1024];
	FILE *fd;
	long fd_size;
	
	memset(&base_rom,0,1048576);
	
	strcpy(filename, fname);

	fd = fopen(filename, "rb");
	if(fd == NULL) {
		display_error("Error opening file.",0);
		return 1;
	}

	fseek(fd,0,SEEK_END);
	fd_size = ftell(fd);
	fseek(fd,0,SEEK_SET);

	if(fd_size%32768) { // skip 512k header if necessary
		fseek(fd,512,SEEK_SET);
		fd_size -= 512;
	}

	if(fread(base_rom, fd_size, 1, fd) != 1) {
		display_error("Short file.",0);
		return 1;
	}

	fclose(fd);

	if(strstr(strupr(filename), ".GG") != NULL)
		cart.type = TYPE_GG;
	else
		cart.type = TYPE_SMS;

	cart.rom = (char *)&base_rom;
	cart.pages = (fd_size / 0x4000);

	return 0;

}

void setupSMSTexture(void) {
	SMSTEX.PSM = GS_PSM_T8;
	SMSTEX.ClutPSM = GS_PSM_CT32;
	SMSTEX.Clut = (u32 *)clut;
	SMSTEX.VramClut = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(16, 16, SMSTEX.ClutPSM), GSKIT_ALLOC_USERBUFFER);
	SMSTEX.Width = 256;
	SMSTEX.Height = 256;
	SMSTEX.TBW = 4; //256;
	SMSTEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(SMSTEX.Width, SMSTEX.Height, SMSTEX.PSM), GSKIT_ALLOC_USERBUFFER);
}

void setupSMSGS(void)
{
    gsGlobal->DrawOrder = GS_OS_PER;
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);
    gsKit_queue_reset(gsGlobal->Per_Queue);

    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00,0x00,0x00,0x80));

    if(Settings.filter)
    	SMSTEX.Filter = GS_FILTER_LINEAR;
    else
    	SMSTEX.Filter = GS_FILTER_NEAREST;

    gsKit_prim_sprite_texture( gsGlobal, &SMSTEX,
    					(cart.type == TYPE_SMS)?0.0f:15.0f, /* X1 */
    					(cart.type == TYPE_SMS)?20.0f:20.0f, /* Y1 */
    					(cart.type == TYPE_SMS)?0.0f:48.0f, /* U1 */
    					(cart.type == TYPE_SMS)?0.0f:24.0f, /* V1 */
                        gsGlobal->Width, /* X2 */ //stretch to screen width
						gsGlobal->Height, /* Y2 */ //stretch to screen height
						(cart.type == TYPE_SMS)?SMSTEX.Width:208, /* U2 */
						(cart.type == TYPE_SMS)?192:173, //SMSTEX.Height, /* V2*/
						2, /* Z */
						GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
						);
//    gsKit_prim_sprite_texture( gsGlobal, &SMSTEX,
//    					0.0f, /* X1 */
//    					0.0f, /* Y1 */
//    					0.0f, /* U1 */
//    					0.0f, /* V1 */
//                        gsGlobal->Width, /* X2 */ //stretch to screen width
//						gsGlobal->Height, /* Y2 */ //stretch to screen height
//						SMSTEX.Width, /* U2 */
//						SMSTEX.Height, /* V2*/
//						2, /* Z */
//						GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
//						);
}

void update_video()
{

	SMSTEX.Mem = (u32 *)bitmap_data;
	
    gsKit_texture_upload(gsGlobal, &SMSTEX);

    /* vsync and flip buffer */
    gsKit_sync_flip(gsGlobal);

    /* execute render queue */
    gsKit_queue_exec(gsGlobal);
}


//void system_load_sram() {}

void update_input()
{
	static struct padButtonStatus pad1; // just in case
	static struct padButtonStatus pad2;
	static int pad1_connected = 0, pad2_connected = 0;
	static int p1_1t=0,p1_2t=0,p2_1t=0,p2_2t=0;
	static u32 new_pad[2];
	int pad1_data = 0;
	int pad2_data = 0;

	memset(&input, 0, sizeof(t_input));

	if(pad1_connected) {
		padRead(0, 0, &pad1); // port, slot, buttons
		pad1_data = 0xffff ^ pad1.btns;
		new_pad[0] = pad1_data & ~old_pad[0];
		old_pad[0] = pad1_data;
		//pad1_data = 0xffff ^ ((pad1.btns[0] << 8) | pad1.btns[1]);

		if(pad1_data & PAD_L1) p1_1t ^= 1;
		else p1_1t = 0;
		if(pad1_data & PAD_R1) p1_2t ^= 1;
		else p1_2t = 0;
		if(pad1_data & PAD_LEFT)				input.pad[0] |= INPUT_LEFT;
		if(pad1_data & PAD_RIGHT)				input.pad[0] |= INPUT_RIGHT;
		if(pad1_data & PAD_UP)					input.pad[0] |= INPUT_UP;
		if(pad1_data & PAD_DOWN)				input.pad[0] |= INPUT_DOWN;
		if((pad1_data & PAD_CROSS)  || p1_2t)	input.pad[0] |= INPUT_BUTTON2;
		if((pad1_data & PAD_SQUARE) || p1_1t)	input.pad[0] |= INPUT_BUTTON1;
		if(pad1_data & PAD_START)				input.system |= (IS_GG) ? INPUT_START : INPUT_PAUSE;

		if((pad1.mode >> 4) == 0x07) {
			if(pad1.ljoy_h < 64) input.pad[0] |= INPUT_LEFT;
			else if(pad1.ljoy_h > 192) input.pad[0] |= INPUT_RIGHT;

			if(pad1.ljoy_v < 64) input.pad[0] |= INPUT_UP;
			else if(pad1.ljoy_v > 192) input.pad[0] |= INPUT_DOWN;
		}
	}

	if(pad2_connected) {
		padRead(1, 0, &pad2); // port, slot, buttons
		pad2_data = 0xffff ^ pad2.btns;
		new_pad[1] = pad1_data & ~old_pad[1];
		old_pad[1] = pad2_data;
		//pad2_data = 0xffff ^ ((pad2.btns[0] << 8) | pad2.btns[1]);

		if(pad2_data & PAD_L1) p2_1t ^= 1;
		else p2_1t = 0;
		if(pad2_data & PAD_R1) p2_2t ^= 1;
		else p2_2t = 0;
		if(pad2_data & PAD_LEFT)				input.pad[1] |= INPUT_LEFT;
		if(pad2_data & PAD_RIGHT)				input.pad[1] |= INPUT_RIGHT;
		if(pad2_data & PAD_UP)					input.pad[1] |= INPUT_UP;
		if(pad2_data & PAD_DOWN)				input.pad[1] |= INPUT_DOWN;
		if((pad2_data & PAD_CROSS ) || p2_2t)	input.pad[1] |= INPUT_BUTTON2;
		if((pad2_data & PAD_SQUARE) || p2_1t)	input.pad[1] |= INPUT_BUTTON1;
//		if(pad2_data & PAD_START)	input.system |= (IS_GG) ? INPUT_START : INPUT_PAUSE;

		if((pad2.mode >> 4) == 0x07) {
			if(pad2.ljoy_h < 64) input.pad[1] |= INPUT_LEFT;
			else if(pad2.ljoy_h > 192) input.pad[1] |= INPUT_RIGHT;

			if(pad2.ljoy_v < 64) input.pad[1] |= INPUT_UP;
			else if(pad2.ljoy_v > 192) input.pad[1] |= INPUT_DOWN;
		}
	}

	//check controller status
	if((padGetState(0, 0)) == PAD_STATE_STABLE) {
		if(pad1_connected == 0) {
			//WaitForNextVRstart(1);
		}
		pad1_connected = 1;
	} else pad1_connected = 0;

	if((padGetState(1, 0)) == PAD_STATE_STABLE) {
		if(pad2_connected == 0) {
			//WaitForNextVRstart(1);
		}
		pad2_connected = 1;
	} else pad2_connected = 0;

	if((new_pad[0] & PAD_TRIANGLE)) {
		if(sound) SjPCM_Pause();
		Ingame_Menu();
		if(sound) SjPCM_Play();
	}
}


void display_error(char* errmsg, int fatal) {
	printf("%s\n", errmsg);
}

char *FileBase(char *fname){
	static char fb[21];
	char *p;
	
	memset(fb, 0, sizeof(fb));
	strncpy(fb, strrchr(fname, '/')+1, 20);
	
	p = strrchr(fb, '.');
	
	if(p) *p = 0;
	
	return fb;
}

void psms_save_state(int slot)
{
	FILE *fd;
	char filename[1024];
	
	sprintf(filename, "%s%s%08X.sv%d", Settings.savepath, FileBase(rom_filename), (unsigned int)crc32(0, base_rom, 1048576), slot);
	
	fd = fopen(filename, "wb");
	
	if(fd == NULL)
		return;
	
	//int pos = 0;

    /* Save VDP context */
    fwrite(&vdp, sizeof(t_vdp), 1, fd);
	//memcpy(&state[pos],&vdp,sizeof(t_vdp));
	//pos += sizeof(t_vdp);

    /* Save SMS context */
    fwrite(&sms, sizeof(t_sms), 1, fd);
	//memcpy(&state[pos],&sms,sizeof(t_sms));
	//pos += sizeof(t_sms);

    /* Save Z80 context */
    fwrite(Z80_Context, sizeof(Z80_Regs), 1, fd);
    fwrite(&after_EI, sizeof(int), 1, fd);
	//memcpy(&state[pos],Z80_Context,sizeof(Z80_Regs));
	//pos += sizeof(Z80_Regs);
	//memcpy(&state[pos],&after_EI,sizeof(int));
	//pos += sizeof(int);

    /* Save YM2413 registers */
    fwrite(&ym2413[0].reg[0], 0x40, 1, fd);
	//memcpy(&state[pos],&ym2413[0].reg[0],0x40);
	//pos += 0x40;

    /* Save SN76489 context */
    fwrite(&sn[0], sizeof(t_SN76496), 1, fd);
	//memcpy(&state[pos],&sn[0],sizeof(t_SN76496));
	//pos += sizeof(t_SN76496);
    
    fclose(fd);
}

void psms_load_state(int slot)
{


	FILE *fd;
	char filename[1024];

	sprintf(filename, "%s%s%08X.sv%d", Settings.savepath, FileBase(rom_filename), (unsigned int)crc32(0, base_rom, 1048576), slot);

	fd = fopen(filename, "rb");

	if(fd == NULL)
		return;

	int i;
	byte reg[0x40];
	//int pos = 0;

	/* Initialize everything */
	cpu_reset();
	system_reset();

	/* Load VDP context */
	fread(&vdp, sizeof(t_vdp), 1, fd);
	//memcpy(&vdp,&state[pos],sizeof(t_vdp));
	//pos += sizeof(t_vdp);

	/* Load SMS context */
	fread(&sms, sizeof(t_sms), 1, fd);
	//memcpy(&sms,&state[pos],sizeof(t_sms));
	//pos += sizeof(t_sms);

	/* Load Z80 context */
	fread(Z80_Context, sizeof(Z80_Regs), 1, fd);
	fread(&after_EI, sizeof(int), 1, fd);
	//memcpy(Z80_Context,&state[pos],sizeof(Z80_Regs));
	//pos += sizeof(Z80_Regs);
	//memcpy(&after_EI,&state[pos],sizeof(int));
	//pos += sizeof(int);

	/* Load YM2413 registers */
	fread(reg, 0x40, 1, fd);
	//memcpy(reg,&state[pos],0x40);
	//pos += 0x40;

	/* Load SN76489 context */
	fread(&sn[0], sizeof(t_SN76496), 1, fd);
	//memcpy(&sn[0],&state[pos],sizeof(t_SN76496));
	//pos += sizeof(t_SN76496);
	
	fclose(fd);

	/* Restore callbacks */
	z80_set_irq_callback(sms_irq_callback);

	cpu_readmap[0] = cart.rom + 0x0000; /* 0000-3FFF */
	cpu_readmap[1] = cart.rom + 0x2000;
	cpu_readmap[2] = cart.rom + 0x4000; /* 4000-7FFF */
	cpu_readmap[3] = cart.rom + 0x6000;
	cpu_readmap[4] = cart.rom + 0x0000; /* 0000-3FFF */
	cpu_readmap[5] = cart.rom + 0x2000;
	cpu_readmap[6] = sms.ram;
	cpu_readmap[7] = sms.ram;

	cpu_writemap[0] = sms.dummy;
	cpu_writemap[1] = sms.dummy;
	cpu_writemap[2] = sms.dummy;
	cpu_writemap[3] = sms.dummy;
	cpu_writemap[4] = sms.dummy;
	cpu_writemap[5] = sms.dummy;
	cpu_writemap[6] = sms.ram;
	cpu_writemap[7] = sms.ram;

	sms_mapper_w(3, sms.fcr[3]);
	sms_mapper_w(2, sms.fcr[2]);
	sms_mapper_w(1, sms.fcr[1]);
	sms_mapper_w(0, sms.fcr[0]);

	/* Force full pattern cache update */
	is_vram_dirty = 1;
	memset(vram_dirty, 1, 0x200);

	/* Restore palette */
	for(i = 0; i < PALETTE_SIZE; i += 1)
		palette_sync(i);

	/* Restore sound state */
	if(snd.enabled)
	{
		/* Restore YM2413 emulation */
		OPLResetChip(ym3812);

		/* Clear YM2413 context */
		ym2413_reset(0);

		/* Restore rhythm enable first */
		ym2413_write(0, 0, 0x0E);
		ym2413_write(0, 1, reg[0x0E]);

		/* User instrument settings */
		for(i = 0x00; i <= 0x07; i += 1)
		{
			ym2413_write(0, 0, i);
			ym2413_write(0, 1, reg[i]);
		}

		/* Channel frequency */
		for(i = 0x10; i <= 0x18; i += 1)
		{
			ym2413_write(0, 0, i);
			ym2413_write(0, 1, reg[i]);
		}

		/* Channel frequency + ctrl. */
		for(i = 0x20; i <= 0x28; i += 1)
		{
			ym2413_write(0, 0, i);
			ym2413_write(0, 1, reg[i]);
		}

		/* Instrument and volume settings  */
		for(i = 0x30; i <= 0x38; i += 1)
		{
			ym2413_write(0, 0, i);
			ym2413_write(0, 1, reg[i]);
		}
	}
}

void psms_manage_sram(u8 *sram, int mode)
{
    char name[1024];
    FILE *fd;
    
	sprintf(name, "%s%s%08X.srm", Settings.savepath, FileBase(rom_filename), (unsigned int)crc32(0, base_rom, 1048576));

//    strcpy(name, game_name);
//    strcpy(strrchr(name, '.'), ".sav");

    switch(mode)
    {
        case SRAM_SAVE:
            if(sms.save)
            {
                fd = fopen(name, "wb");
                if(fd)
                {
                    fwrite(sram, 0x8000, 1, fd);
                    fclose(fd);
                }
            }
            break;

        case SRAM_LOAD:
            fd = fopen(name, "rb");
            if(fd)
            {
                sms.save = 1;
                fread(sram, 0x8000, 1, fd);
                fclose(fd);
            }
            else
            {
                /* No SRAM file, so initialize memory */
                memset(sram, 0x00, 0x8000);
            }
            break;
    }
}
