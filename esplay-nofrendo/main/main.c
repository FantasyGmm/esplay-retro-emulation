#include "esp_system.h"
#include "../components/nofrendo/nofrendo.h"
#include "esp_partition.h"

#include "esp_err.h"
#include <esp_sleep.h>

#include "settings.h"
#include "power.h"
#include "sdcard.h"
#include "display.h"
#include "gamepad.h"
#include "audio.h"

const char* SD_BASE_PATH = "/sd";
extern bool forceConsoleReset;
int32_t scaleAlg;

int app_main(void)
{
    settings_init();

    esplay_system_init();

    audio_init(32000);

    char* fileName;

    char *romName = settings_load_str(SettingRomPath);

    if (romName)
    {
        fileName = system_util_GetFileName(romName);
        if (!fileName) abort();

        free(romName);
    }
    else
    {
        fileName = "nesemu-show3.nes";
    }


    int startHeap = esp_get_free_heap_size();
    printf("A HEAP:%d kb\n", startHeap/1024);

    // Joystick.
    gamepad_init();

    // display
    display_init();
//    display_prepare();

    // display brightness
    int brightness;
    settings_load(SettingBacklight, &brightness);
    set_display_brightness(brightness);

    // load alghoritm
    settings_load(SettingAlg, &scaleAlg);

    // battery
    battery_level_init();

    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_EXT0:
        {
            printf("app_main: ESP_SLEEP_WAKEUP_EXT0 deep sleep reset\n");
            break;
        }

        case ESP_SLEEP_WAKEUP_EXT1:
        case ESP_SLEEP_WAKEUP_TIMER:
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
        case ESP_SLEEP_WAKEUP_ULP:
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        {
            printf("app_main: Unexpected deep sleep reset\n");
            input_gamepad_state bootState = gamepad_input_read_raw();

            if (bootState.values[GAMEPAD_INPUT_MENU])
            {
                // Force return to menu to recover from
                // ROM loading crashes

                // Set menu application
                system_application_set(0);

                // Reset
                esp_restart();
            }

            if (bootState.values[GAMEPAD_INPUT_START])
            {
                // Reset emulator if button held at startup to
                // override save state
                forceConsoleReset = true; //emu_reset();
            }
        }
            break;

        default:
            printf("app_main: Not a deep sleep reset\n");
            break;
    }

    // Load ROM
    char *romPath = settings_load_str(SettingRomPath);
    if (!romPath)
    {
//        printf("osd_getromdata: Reading from flash.\n");
//
//        const esp_partition_t* part = esp_partition_find_first(0x40, 0, NULL);
//        if (part == 0)
//        {
//            printf("esp_partition_find_first failed.\n");
//            abort();
//        }
//
//        esp_err_t err = esp_partition_read(part, 0, (void*)ROM_DATA, 0x100000);
//        if (err != ESP_OK)
//        {
//            printf("esp_partition_read failed. size = %x (%d)\n", part->size, err);
//            abort();
//        }
    }
    else
    {
        printf("osd_getromdata: Reading from sdcard.\n");

        // copy from SD card
        esp_err_t r = sdcard_open(SD_BASE_PATH);
        if (r != ESP_OK)
        {
	        printf("osd_getromdata: Reading from sdcard fail.\n");
            abort();
        }

        display_clear(0);
        display_show_hourglass();
        size_t fileSize = sdcard_copy_file_to_memory(romPath);
        printf("app_main: fileSize=%d/1024 kb\n", fileSize);
        if (fileSize == 0)
        {
            abort();
        }

        r = sdcard_close();
        if (r != ESP_OK)
        {
            abort();
        }

        free(romPath);
    }

    char* args[1] = { fileName };
	printf("NoFrendo start!rom: %s\n",fileName);
    nofrendo_main(1, args);

    printf("NoFrendo died.\n");
    asm("break.n 1");
    return 0;
}