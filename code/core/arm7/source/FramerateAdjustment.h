#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initializes framerate adjustment.
void fps_initializeFramerateAdjustment(void);

/// @brief Starts adjusting the framerate such that the average number of cycles per frame
///        becomes identical to the GBA number of cycles per frame.
/// @warning This method will busy-wait until the right moment to start the adjustment.
void fps_startFramerateAdjustment(void);

/// @brief Stops the framerate adjustment.
void fps_stopFramerateAdjustment(void);

#ifdef __cplusplus
}
#endif
