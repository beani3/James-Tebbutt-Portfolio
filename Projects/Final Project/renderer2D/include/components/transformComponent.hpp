#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
* @struct TransformComponent
* @brief A 2D transform component
*/
struct TransformComponent {

    glm::mat3 transform{ glm::mat3(1.f) };      //!< Overall transform
    glm::vec2 translation{ glm::vec2(0.f) };    //!< Position
    glm::vec2 scale{ glm::vec2(1.f) };          //!< Scale
    float rotation{ 0.f };                      //!< Rotation in radians

    TransformComponent() = default;

    /** Constructor with values */
    TransformComponent(const glm::vec2& t, const float& r, const glm::vec2& s) :
        translation(t),
        rotation(r),
        scale(s)
    {
        recalc();
    }

    /** Recalculate local transform */
    void recalc()
    {
        //glm::mat3 t = glm::mat3(glm::translate(glm::mat4(1.f), glm::vec3(translation, 1.f)));
        glm::mat3 t = glm::mat3(
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            translation.x, translation.y, 1.f
        );

		glm::mat3 r = glm::mat3(
            cosf(rotation), -sinf(rotation), 0.f,
            sinf(rotation), cosf(rotation), 0.f,
			0.f, 0.f, 1.f
        );

        glm::mat3 s = glm::mat3(glm::scale(glm::mat4(1.f), glm::vec3(scale, 1.f)));

        transform = t * r * s;
    }

    /** Recalculate relative to parent transform */
    void recalc(const glm::mat3& parent)
    {
        glm::mat3 t = glm::mat3(
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            translation.x, translation.y, 1.f
        );

        glm::mat3 r = glm::mat3(
            cos(rotation), sin(rotation), 0.f,
            -sin(rotation), cos(rotation), 0.f,
            0.f, 0.f, 1.f
        );

        glm::mat3 s = glm::mat3(glm::scale(glm::mat4(1.f), glm::vec3(scale, 1.f)));

        transform = parent * t * r * s;
    }
};