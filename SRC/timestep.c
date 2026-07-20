#include "timestep.h"
#include <time.h>
#include <sys/timeb.h> // For ftime() in DOS/DJGPP
#include <stdlib.h>

// Internal: start time in microseconds
static clock_t time_start_us = 0;
static int time_initialized = 0;

/**
 * Initialize the internal timer.
 */
void timestep_init(void) {
    if (!time_initialized) {
        struct timeb tb;
        ftime(&tb);
        // Convert to microseconds: tb.time is seconds, tb.millitm is milliseconds
        time_start_us = ((clock_t)tb.time * 1000000ULL) + ((clock_t)tb.millitm * 1000ULL);
        time_initialized = 1;
    }
}

/**
 * Get the elapsed time in microseconds since the application started.
 */
clock_t get_micros(void) {
    if (!time_initialized) {
        timestep_init();
    }

    struct timeb tb;
    ftime(&tb);
    clock_t now_us = ((clock_t)tb.time * 1000000ULL) + ((clock_t)tb.millitm * 1000ULL);

    // Calculate elapsed time
    if (now_us >= time_start_us) {
        return now_us - time_start_us;
    } else {
        // Handle wrap-around (unlikely with 64-bit, but safe)
        return (2147483647 - time_start_us) + now_us;
    }
}

/**
 * Clamp a float value between min and max.
 */
static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Create a new Timestep with the given ticks per second.
 */
Timestep timestep_new(float tps) {
    Timestep timestep = {0}; // Zero-initialize
    timestep.speed = 1.0f;

    timestep_reset(&timestep);
    timestep_set_tps(&timestep, tps);

    return timestep;
}

/**
 * Set the ticks per second.
 */
void timestep_set_tps(Timestep *timestep, float tps) {
    timestep->delta = 1000.0f / tps;
}

/**
 * Reset the timestep.
 */
void timestep_reset(Timestep *timestep) {
    timestep->current_time_us = get_micros();
    timestep->accumulator = 0.0f;
}
/**
 * Process a tick.
 * @return 1 if a tick was processed, 0 otherwise.
 */
int timestep_on_tick(Timestep *timestep) {
    clock_t newtime = get_micros();
    clock_t frametime = newtime - timestep->current_time_us;

    timestep->time_micros += frametime;
    timestep->current_time_us = newtime;

    // Accumulate time, scaled by speed and converted from microseconds to milliseconds (since delta is in ms)
    timestep->accumulator += (float)frametime * timestep->speed / 1000.0f;

    // Calculate alpha
    timestep->alpha = clamp(timestep->accumulator / timestep->delta, 0.0f, 1.0f);

    if (timestep->accumulator >= timestep->delta) {
        timestep->accumulator -= timestep->delta;
        timestep->loopnum += 1;
        timestep->ticks += 1;

        if (timestep->loopnum > 5) {
            // Cannot keep up!
            timestep->loopnum = 0;
            timestep->accumulator = 0.0f;
            return 0;
        }

        return 1;
    } else {
        timestep->loopnum = 0;
        return 0;
    }
}
