#ifndef BDRAW_H_
#define BDRAW_H_

// Prototypes
void menu_bgtexture(GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2, int z);
void DrawScreen(GSGLOBAL *gsGlobal);
void menu_background(float x1, float y1, float x2, float y2, int z);
void browser_primitive(char *title1, char *title2, GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2);
void normalize_screen(void);
void init_custom_screen(void);
void SetupGSKit(void);

#endif /*DRAW_H_*/
