#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcm.h>

#include "hw_md.h"
#include "cdfh.h"
#include "module.h"


extern void setup();
extern void loop();

Mod_t module;
CDFileHandle_t *handle;

static uint16_t mvolume = 12; // 0 - 16 (MAX)
static uint16_t mindex = 0;

uint8_t modNotLoaded = 0;

#define NUM_MODS 1
static char *modName[NUM_MODS] = {
    "Track1.MOD"
};

static void start_song(int index)
{
    modNotLoaded = 1;
    StopMOD(&module);
    ExitMOD(&module);
    set_cwd("/MUSIC"); // set current directory
    handle = cd_handle_from_name(modName[index]);
    if (handle)
    {
        modNotLoaded = InitMOD(handle, &module);
        delete_cd_handle(handle);
        handle = NULL;
        VolumeMOD(mvolume);
        StartMOD(&module, 1); // start MOD with looping
    }
}

int main(void)
{
    uint16_t buttons, prev_buttons = 0;

    init_cd(); // init CD

    strcpy((char*)0x0C0000, "Initializing tables...");
    switch_banks();
    do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_WHITE, 9, 14);

    setup();

    strcpy((char*)0x0C0000, "   Loading music...   ");
    switch_banks();
    do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_WHITE, 9, 14);

    //start_song(mindex);

    strcpy((char*)0x0C0000, "                      ");
    switch_banks();
    do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_GREEN, 1, 24);

    memset((char*)0x0C0000, 0, 161*224*2); // clear the screen buffer
    switch_banks();
    memset((char*)0x0C0000, 0, 161*224*2); // clear the screen buffer

    // start direct color dma in narrow mode
    send_md_cmd2(MD_CMD_DMA_SCREEN, 0x200000, 0);

    while (1)
    {
        loop();

        buttons = GET_PAD(0) & SEGA_CTRL_BUTTONS;
        if ((buttons ^ prev_buttons) & SEGA_CTRL_MODE)
        {
            if (buttons & SEGA_CTRL_MODE)
            {
                // MODE pressed
                wait_md_cmd(); // stop direct color dma screen

                strcpy((char*)0x0C0000, "   Loading music...   ");
                switch_banks();
                do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_WHITE, 9, 14);

                mindex = mindex < (NUM_MODS - 1) ? mindex + 1 : 0;
                start_song(mindex);

                strcpy((char*)0x0C0000, "                      ");
                switch_banks();
                do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_GREEN, 1, 24);

                memset((char*)0x0C0000, 0, 161*224*2); // clear the screen buffer
                switch_banks();
                memset((char*)0x0C0000, 0, 161*224*2); // clear the screen buffer

                // start direct color dma in narrow mode
                send_md_cmd2(MD_CMD_DMA_SCREEN, 0x200000, 0);
            }
            prev_buttons = buttons;
        }
    }

    return 0;
}

