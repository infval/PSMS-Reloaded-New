#ifndef PS2FONT_H_
#define PS2FONT_H_

// Prototypes
int loadFont(char *path_arg);
void drawChar(unsigned int c, int x, int y, int z, u64 colour);
int printXY(const unsigned char *s, int x, int y, int z, u64 colour, int draw, int space);

#endif /*PS2FONT_H_*/
