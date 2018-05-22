#ifndef _SJPCM_H
#define _SJPCM_H

#define	SJPCM_IRX		0xB0110C5
#define SJPCM_PUTS		0x01
#define	SJPCM_INIT		0x02
#define SJPCM_PLAY		0x03
#define SJPCM_PAUSE		0x04
#define SJPCM_SETVOL	0x05
#define SJPCM_ENQUEUE	0x06
#define SJPCM_CLEARBUFF	0x07

void SjPCM_Puts(char *format, ...);
int SjPCM_Init();
void SjPCM_Enqueue(short *left, short *right, int size, int wait);
void SjPCM_Play();
void SjPCM_Pause();
void SjPCM_Setvol(unsigned int volume);
void SjPCM_Clearbuff();

#endif // _SJPCM_H
