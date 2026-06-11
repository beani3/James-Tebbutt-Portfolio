#pragma once
#include "glm/glm.hpp"
#include "rendering/renderer.hpp"
#include "components/renderComponent.hpp"
#include "entt/entt.hpp"

/**
 * @file animationComponent.hpp
 * @brief Simple animation state for entities using sprite sheets.
 *
 * AnimationComponent stores the state required to step through sprite-sheet
 * frames for an entity. It is intentionally lightweight and driven by the
 * FinalProject update loop.
 */

/** @brief Enumeration of supported animation states. */
enum animStates { Idle, Running, Jumping };

/**
 * @brief Component holding animation timing and frame indices.
 *
 * - `frames` is the total number of frames in the current animation.
 * - `currentFrame` is the active frame index.
 * - `frameTime` is the duration of each frame in seconds.
 */
struct AnimationComponent
{
    animStates animState{ animStates::Idle }; //!< Current animation state
    animStates previousState{ animStates::Idle }; //!< Previous state (for transitions)

    int frames{ 1 }; //!< Number of frames in current animation
    int currentFrame{ 0 }; //!< Current frame index

    float frameTime{ 0.1f }; //!< Target time per frame in seconds
    float timer{ 0.0f }; //!< Accumulated time since last frame advance
};