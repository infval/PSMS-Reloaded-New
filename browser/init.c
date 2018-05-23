#include <kernel.h>
#include <stdio.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <smod.h>
#include <loadfile.h>
#include <libpwroff.h>
#include <sbv_patches.h>
#include <libmc.h>
#include "cdvd_rpc.h"
#include "cd.h"
#include <libpad.h>

extern void poweroff_irx;
extern int size_poweroff_irx;
extern void iomanX_irx;
extern int size_iomanX_irx;
extern void fileXio_irx;
extern int size_fileXio_irx;
extern void ps2dev9_irx;
extern int size_ps2dev9_irx;
extern void ps2atad_irx;
extern int size_ps2atad_irx;
extern void ps2hdd_irx;
extern int size_ps2hdd_irx;
extern void ps2fs_irx;
extern int size_ps2fs_irx;
extern void usbd_irx;
extern int size_usbd_irx;
extern void usbhdfsd_irx;
extern int size_usbhdfsd_irx;
extern void cdvd_irx;
extern int size_cdvd_irx;
extern void sjpcm_irx;
extern int size_sjpcm_irx;

void poweroffps2(int i)
{
    poweroffShutdown();
}

void LoadHDDModules(void)
{
    int ret;
    smod_mod_info_t	mod_t;

    if(!smod_get_mod_by_name("Poweroff_Handler", &mod_t))
        ret = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: POWEROFF.IRX");
    }
    poweroffInit();
    poweroffSetCallback((void *)poweroffps2, NULL);
    if(!smod_get_mod_by_name("IOX/File_Manager", &mod_t))
        ret = SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: IOMANX.IRX");
    }
    if(!smod_get_mod_by_name("IOX/File_Manager_Rpc", &mod_t))
        ret = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: IOMANX.IRX");
    }
    if(!smod_get_mod_by_name("dev9_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2DEV9.IRX");
    }
    if(!smod_get_mod_by_name("atad", &mod_t))
        ret = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2ATAD.IRX");
    }
    static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
    if(!smod_get_mod_by_name("hdd_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx,sizeof(hddarg), hddarg, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2HDD.IRX");
    }
    static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";
    if(!smod_get_mod_by_name("pfs_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx,sizeof(pfsarg), pfsarg, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2FS.IRX");
    }
}

int LoadBasicModules(void) {
	int ret = 0, old = 0;

	smod_mod_info_t mod_t;

	if (!smod_get_mod_by_name("sio2man", &mod_t)) {
		ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	}
	if (mod_t.version == 257)
		old = 1;
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
	}
	if (!smod_get_mod_by_name("mcman", &mod_t)) {
		ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
	}
	if (mod_t.version == 257)
		old = 1;
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
	}
	if (!smod_get_mod_by_name("mcserv", &mod_t)) {
		ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
	}
	if (mod_t.version == 257)
		old = 1;
	else
		mcReset();
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
	}
	if (!smod_get_mod_by_name("padman", &mod_t)) {
		ret = SifLoadModule("rom0:XPADMAN", 0, NULL);
	}
	if (mod_t.version == 276)
		old = 1;
	else
		padReset();
	if (ret < 0) {
		printf("Failed to load module: PADMAN");
	}

	return old;
}
void LoadExtraModules(void) {
	int i, ret, sometime;

	ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: LIBSD");
	}
	ret = SifExecModuleBuffer(&sjpcm_irx, size_sjpcm_irx,0, NULL, &ret);
	if (ret < 0) {
		printf("Failed to load module: SJPCM.IRX");
	}

	ret = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
	if (ret < 0) {
		printf("Failed to load module: USBD.IRX");
	}

	ret = SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, &ret);
	if (ret < 0) {
		printf("Failed to load module: CDVD.IRX");
	}

	ret = SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &ret);
	for (i = 0; i < 3; i++) { //taken from ulaunchelf
		sometime = 0x01000000;
		while (sometime--)
			asm("nop\nnop\nnop\nnop");
	}

	if (ret < 0) {
		printf("Failed to load module: USBHDFSD.IRX");
	}
}

void InitPS2(void) {
	SifInitRpc(0);

	//Reset IOP borrowed from uLaunchelf

	if (LoadBasicModules()) {
		SifIopReset("rom0:UDNL rom0:EELOADCNF", 0);
		while (!SifIopSync());
		fioExit();
		SifExitIopHeap();
		SifLoadFileExit();
		SifExitRpc();
		SifExitCmd();

		SifInitRpc(0);
		FlushCache(0);
		FlushCache(2);

		LoadBasicModules();
	}

	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	LoadExtraModules();
	LoadHDDModules();

	mcInit(MC_TYPE_XMC);
	cdInit(CDVD_INIT_INIT);
	CDVD_Init();

	padInit(0);
}
