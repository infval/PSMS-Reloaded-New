#ifndef _PSMS_H_
#define _PSMS_H_


//#define DEVEL
//#define CD_BUILD


#define WIDTH		256
#define HEIGHT		240

/* VRAM layout:

0x000000 - FB 1
0x040000 - FB 2 (FB 1 + 256*256*4)
0x080000 - ZBuf (FB 2 * 2)
0x0A0000 - End of ZBuf. Star of TEX and CLUT area.

0x0B0000 - SMS Display Texture (0xC000 bytes long)
0x0BC000 - SMS Display Clut (0x200 bytes long)

0x0C0000 - PSMS logo image (0x8000 bytes long)
0x0C8000 - PSMS logo clut (0x200 bytes long)

0x0D0000 - PSMS font image (0x10000 bytes long)
0x0E0000 - PSMS font clut (0x200 bytes long)

*/

#define SMS_TEX		0x0B0000
#define SMS_CLUT	0x0BC000

#define LOGO_TEX	0x0C0000
#define LOGO_CLUT	0x0C8000

#define FONT_TEX	0x0D0000
#define FONT_CLUT	0x0E0000

#define VRAM_MAX	0x3E8000



//#define WAIT_PAD_READY(p, s) {while(padGetState((p),(s)) != PAD_STATE_STABLE) WaitForNextVRstart(1); }
#define WAIT_PAD_READY(p, s) {while(padGetState((p),(s)) != PAD_STATE_STABLE); }

// PSMS Logo
extern unsigned char __attribute__((aligned(16))) psms_image[];
extern unsigned char __attribute__((aligned(16))) psms_clut[];

#define psms_width 256
#define psms_height 128
#define psms_mode 1

// Menu font
extern unsigned char __attribute__((aligned(16))) vixar_image[];
extern unsigned char __attribute__((aligned(16))) vixar_clut[];
extern unsigned char vixarmet[];

#define vixar_width 256
#define vixar_height 256
#define vixar_mode 1

int load_rom(char* filename);
void update_video();
void update_input();
int init_machine();
void LoadModules();
void setupSMSGS(void);
void setupSMSTexture(void);
int InitPad(int port, int slot, char* buffer);
void test();
void TextOut(int x, int y, char *string, int z);
void TextOutC(int x_start, int x_end, int y, char *string, int z);
void IngameMenu();
void psms_save_state();
void psms_load_state();
void psms_manage_sram(u8 *sram, int mode);

void display_error(char* errmsg, int fatal);



#endif /* _PSMS_H_ */
