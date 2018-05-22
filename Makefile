# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_BIN = psms.elf
EE_OBJS = sjpcm_rpc.o psms.o fmopl.o render.o sms.o sn76496.o system.o vdp.o ym2413.o z80.o sjpcm_irx.o browser/browser.o browser/cd.o browser/bdraw.o browser/font_uLE.o browser/init.o browser/pad.o browser/ps2font.o browser/cnfsettings.o browser/menu.o browser/Reboot_ELF.o iomanX_irx.o fileXio_irx.o ps2dev9_irx.o ps2atad_irx.o ps2hdd_irx.o ps2fs_irx.o poweroff_irx.o usbd_irx.o cdvd_irx.o usbhdfsd_irx.o 
EE_CFLAGS  += -DALIGN_DWORD -DLSB_FIRST  
EE_LDFLAGS += -L$(PS2DEV)/gskit/lib -L$(PS2DEV)/ps2sdk/ports/lib -s
EE_LIBS += -lgskit -ldmakit -ljpg -lpng -lz -lm -lfileXio -lhdd -lmc -lpadx -lpoweroff -lpatches -ldebug -lcdvdfs
EE_INCS += -I. -I./cpu -I./browser 
EE_INCS += -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include -I$(PS2DEV)/gsKit/include

#.SUFFIXES: .c .s .cc .dsm

all: $(EE_BIN)
	ee-strip $(EE_BIN)

iomanX_irx.s:
	bin2s $(PS2SDK)/iop/irx/iomanX.irx iomanX_irx.s iomanX_irx

fileXio_irx.s:
	bin2s $(PS2SDK)/iop/irx/fileXio.irx fileXio_irx.s fileXio_irx

ps2dev9_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2dev9.irx ps2dev9_irx.s ps2dev9_irx

ps2atad_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2atad.irx ps2atad_irx.s ps2atad_irx

ps2hdd_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2hdd.irx ps2hdd_irx.s ps2hdd_irx

ps2fs_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2fs.irx ps2fs_irx.s ps2fs_irx

poweroff_irx.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx poweroff_irx.s poweroff_irx

usbd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx usbd_irx.s usbd_irx

usbhdfsd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbhdfsd.irx usbhdfsd_irx.s usbhdfsd_irx

cdvd_irx.s:
	bin2s $(PS2SDK)/iop/irx/cdvd.irx cdvd_irx.s cdvd_irx
	
sjpcm_irx.s:
	bin2s sjpcm.irx sjpcm_irx.s sjpcm_irx
	

clean:
	rm -f *.elf *.o *.a *.s browser/*.o

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

