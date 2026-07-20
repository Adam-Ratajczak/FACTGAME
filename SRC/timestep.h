#ifndef TIMESTEP_H
#define TIMESTEP_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the elapsed time in microseconds since the application started.
 * Note: In DOS/DJGPP, this has millisecond precision using ftime().
 */
clock_t get_micros(void);

/**
 * Initialize the internal timer.
 */
void timestep_init(void);

/**
 * Timestep structure.
 */
typedef struct {
    clock_t current_time_us;
    float    accumulator;
    clock_t time_micros;
    unsigned int ticks;
    float    speed;
    float    alpha;
    float    delta;
    signed char  loopnum;
} Timestep;

/**
 * Create a new Timestep with the given ticks per second.
 * @param tps Ticks per second.
 * @return A new Timestep instance.
 */
Timestep timestep_new(float tps);

/**
 * Set the ticks per second.
 * @param timestep Pointer to the timestep structure.
 * @param tps New ticks per second.
 */
void timestep_set_tps(Timestep *timestep, float tps);

/**
 * Reset the timestep.
 * @param timestep Pointer to the timestep structure.
 */
void timestep_reset(Timestep *timestep);

/**
 * Process a tick.
 * @param timestep Pointer to the timestep structure.
 * @return 1 if a tick was processed, 0 otherwise.
 */
int timestep_on_tick(Timestep *timestep);

#ifdef __cplusplus
}
#endif

#endif /* TIMESTEP_H */
