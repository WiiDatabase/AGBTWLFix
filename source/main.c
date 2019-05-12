#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#define VERSION 1.0

void exitServices() {
    ptmSysmExit();
    amExit();
    aptExit();
    gfxExit();
}

void exitApplication() {
	printf("\n\nPress START to exit");	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) { 
			exitServices();
			exit(0);
		}
	}
}

void initServices() {
    // Init console for text output
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    consoleClear();
    printf(CONSOLE_GREEN "3DS AGBTWLFix %.1f" CONSOLE_RESET "\n\n", VERSION);
    
    // Initialize services
    printf("Initializing services...\n");
	if (R_FAILED(aptInit())) {
		printf(CONSOLE_RED "Failed to initialize APT services" CONSOLE_RESET "\n");
		exitApplication();
	}
	if (R_FAILED(amInit())) {
		printf(CONSOLE_RED "Failed to initialize AM services" CONSOLE_RESET "\n");
		exitApplication();
	}
	if (R_FAILED(ptmSysmInit())) {
		printf(CONSOLE_RED "Failed to initialize PTM services" CONSOLE_RESET "\n");
		exitApplication();
    }
}

int main(int argc, char** argv) {
    initServices();
    bool isNew3DS = false;
    APT_CheckNew3DS(&isNew3DS);
    
    u64 titleList[5] = {
        0x0004800f484e4841, // Whitelist
        //0x0004800f484e4C41, // Version Data (doesn't get reinstalled)
        0x0004800542383841, // DS Internet
        0x00048005484E4441 // DS DLP
    };
	if (isNew3DS) {
		titleList[3] = 0x0004013820000102; // New3DS TWL FIRM
        titleList[4] = 0x0004013820000202; // New3DS AGB FIRM
	} else {
		titleList[3] = 0x0004013800000102; // Old3DS TWL FIRM
        titleList[4] = 0x0004013800000202; // Old3DS AGB FIRM
    }
    
    // Begin Code here
    printf("Press A to begin or START to exit.\n\n");
    
    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_START) {
            printf("Exiting...\n");
            exitServices();
            return 0;
        }
        
        if (kDown & KEY_A) {
            break;
        }
    }

    printf("Uninstalling...\n");
    for(int i = 0; i < sizeof(titleList) / sizeof(u64); i++) {
        printf("Uninstalling title %llx...\n", titleList[i]);
        if (R_FAILED(AM_DeleteTitle(MEDIATYPE_NAND, titleList[i]))) {
			printf(CONSOLE_RED "Failed!" CONSOLE_RESET "\n");
		} else {
			printf(CONSOLE_GREEN "Success!" CONSOLE_RESET "\n");
        }
    }
    
    printf("\nDone!\nReboot and then open System Update.\n\nPress START to reboot.\n");
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) { 
			break;
		}
        
        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
	}
    
    printf("Rebooting...\n");
    PTMSYSM_RebootAsync(0);
    exitServices();
    
    return 0;
}
