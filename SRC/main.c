#include "alwrap.h"
#include "game.h"
#include <stdio.h>
#include <random.h>
#include <time.h>
#include "log.h"

int main(void) {
    log_clear();
    srand(time(NULL));

    allegro_init();
    install_keyboard();
    set_uformat(U_ASCII);

    if (install_mouse() < 0) {
        log_debug("No mouse detected (is a DOS mouse driver loaded?)");
        return 1;
    }

    if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) {
        log_debug("Failed to initialize sound: %s", allegro_error);
        return 1;
    }

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0) != 0) {
        log_debug("Couldn't initialize video");
        return 1;
    }
    set_palette(desktop_palette);
    set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS);

    run_game();

    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

    return 0;
}
END_OF_MAIN();
