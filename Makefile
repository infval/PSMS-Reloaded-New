# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_BIN = psms.elf
EE_OBJS = sjpcm_rpc.o psms.o fmopl.o render.o sms.o sn76496.o system.o vdp.o ym2413.o z80.o browser/browser.o browser/cd.o browser/bdraw.o browser/font_uLE.o browser/init.o browser/pad.o browser/ps2font.o browser/cnfsettings.o browser/menu.o browser/Reboot_ELF.o irx/iomanX_irx.o irx/fileXio_irx.o irx/ps2dev9_irx.o irx/ps2atad_irx.o irx/ps2hdd_irx.o irx/ps2fs_irx.o irx/poweroff_irx.o irx/usbd_irx.o irx/cdvd_irx.o irx/usbhdfsd_irx.o irx/sjpcm_irx.o 
EE_CFLAGS  += -DALIGN_DWORD -DLSB_FIRST  
EE_LDFLAGS += -L$(PS2DEV)/gskit/lib -L$(PS2DEV)/ps2sdk/ports/lib -s
EE_LIBS += -lgskit -ldmakit -ljpeg -lpng -lz -lm -lfileXio -lhdd -lmc -lpadx -lpoweroff -lpatches -ldebug
EE_INCS += -I. -I./cpu -I./browser 
EE_INCS += -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include -I$(PS2DEV)/gsKit/include

EE_INCS += -Ilibcdvd/ee
EE_LDFLAGS += -Llibcdvd/lib
EE_LIBS += -lcdvdfs

IRX_DIR=$(PS2SDK)/iop/irx

#.SUFFIXES: .c .s .cc .dsm

all: $(EE_BIN)
	ee-strip $(EE_BIN)

./irx/iomanX_irx.c:
	bin2c $(IRX_DIR)/iomanX.irx ./irx/iomanX_irx.c iomanX_irx

./irx/fileXio_irx.c:
	bin2c $(IRX_DIR)/fileXio.irx ./irx/fileXio_irx.c fileXio_irx

./irx/ps2dev9_irx.c:
	bin2c $(IRX_DIR)/ps2dev9.irx ./irx/ps2dev9_irx.c ps2dev9_irx

./irx/ps2atad_irx.c:
	bin2c $(IRX_DIR)/ps2atad.irx ./irx/ps2atad_irx.c ps2atad_irx

./irx/ps2hdd_irx.c:
	bin2c $(IRX_DIR)/ps2hdd.irx ./irx/ps2hdd_irx.c ps2hdd_irx

./irx/ps2fs_irx.c:
	bin2c $(IRX_DIR)/ps2fs.irx ./irx/ps2fs_irx.c ps2fs_irx

./irx/poweroff_irx.c:
	bin2c $(IRX_DIR)/poweroff.irx ./irx/poweroff_irx.c poweroff_irx

./irx/usbd_irx.c:
	bin2c $(IRX_DIR)/usbd.irx ./irx/usbd_irx.c usbd_irx

./irx/usbhdfsd_irx.c:
	bin2c $(IRX_DIR)/usbhdfsd.irx ./irx/usbhdfsd_irx.c usbhdfsd_irx

./irx/cdvd_irx.c:
	bin2c cdvd.irx ./irx/cdvd_irx.c cdvd_irx
	
./irx/sjpcm_irx.c:
	bin2c sjpcm.irx ./irx/sjpcm_irx.c sjpcm_irx
	

clean:
	rm -f *.elf *.o *.a *.s browser/*.o ./irx/*.c ./irx/*.o

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

