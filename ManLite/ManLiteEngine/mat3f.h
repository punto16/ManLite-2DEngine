#pragma once
#include "Defs.h"

class mat3f {
public:
    float m[9];

    mat3f();
    mat3f(const float* matrix);

    static mat3f CreateTransformMatrix(vec2f position, float rotation_radians, vec2f scale);

    mat3f operator*(const mat3f& other) const;
    vec2f operator*(const vec2f& point) const;

    mat3f Inverted() const;

    vec2f GetTranslation() const;
    float GetRotation() const;
    vec2f GetScale() const;

    vec2f TransformPoint(vec2f point) const;
};