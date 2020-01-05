/*
 * A world simulation game by NG.
 * Created on 29-11-19.
 */

#include "global.h"
#include "Window.h"
#include "API.h"
#include "Visual.h"

#define GAME_FPS 30.0

// sleep function
#if defined(WIN32)
    #include <time.h>
    #include <windows.h>

    #define wait_for(time_ms) Sleep((time_ms))

#elif defined(LINUX)
    #include <unistd.h>
    #define wait_for(time_ms) usleep((time_ms) * 1000)

#else
    #define wait_for(time_ms) ; // don't wait at all
#endif

int main(int argc, char** argv) {
    LOG_INFO("Reading parameters...");
    // ...

    LOG_INFO("Starting Graphics engine...");
    Visualizer* visualizer = visualizer_create_window();

    LOG_INFO("Starting Simulation engine...");
    YADFEngine* simulator = yadf_init(visual_create);

    LOG_INFO("Setup done!");
    UpdateCycle game_time = 0;
    time_t loop_time = clock();

    while (!visualizer_is_closed(visualizer)) {
        time_t update_start = clock();
        yadf_trigger_loop_update(simulator);

        time_t render_start = clock();
        World* world = yadf_get_world(simulator);
        visualizer_draw_frame(visualizer, world);

        time_t callback_start = clock();
        visualizer_callbacks(visualizer);

        time_t end_time = clock();

        if ((end_time - loop_time) > 100){ // less than 10 FPS
            LOG_INFO_F(
                    "Frame drops: [update : %5ld ms][render : %5ld ms][callbacks : %5ld ms][total: %6ld ms]",
                    render_start - update_start, callback_start - render_start, end_time - callback_start,
                    end_time - loop_time
            );
        }

        long wait_time_ms = (long) ((1000.0 / GAME_FPS) - (double) (end_time - loop_time) - 1);

        if (wait_time_ms > 0) {
            wait_for(wait_time_ms);
            loop_time += (1000.0 / GAME_FPS);

        } else {
            loop_time = clock();
        }

        game_time++;
    }

    if (visualizer_has_error(visualizer)) {
        LOG_INFO("Closing due to an error in the visualizer...");

    } else {
        LOG_INFO("Closing...");
    }

    visualizer_free(visualizer);
    yadf_free(simulator);

    return EXIT_SUCCESS;
}