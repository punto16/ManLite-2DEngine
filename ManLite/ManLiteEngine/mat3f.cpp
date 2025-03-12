#include "mat3f.h"
#include <cmath>

mat3f::mat3f() : m{ 1,0,0, 0,1,0, 0,0,1 } {}

mat3f::mat3f(const float* matrix)
{
    for (int i = 0; i < 9; ++i) m[i] = matrix[i];
}

mat3f mat3f::CreateTransformMatrix(vec2f position, float rotation_radians, vec2f scale)
{
    float c = cos(rotation_radians);
    float s = sin(rotation_radians);

    return mat3f(new float[9] {
        c* scale.x, s* scale.x, 0,
            -s * scale.y, c* scale.y, 0,
            position.x, position.y, 1
        });
}

mat3f mat3f::operator*(const mat3f& other) const
{
    float result[9] = { 0 };

    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            for (int i = 0; i < 3; ++i)
                result[row + col * 3] += m[row + i * 3] * other.m[i + col * 3];

    return mat3f(result);
}

vec2f mat3f::operator*(const vec2f& point) const
{
    return TransformPoint(point);
}

mat3f mat3f::Inverted() const
{
    float det = m[0] * m[4] - m[1] * m[3];

    return mat3f(new float[9] {
        m[4] / det, -m[3] / det, 0,
            -m[1] / det, m[0] / det, 0,
            (m[1] * m[7] - m[4] * m[6]) / det,
            (m[3] * m[6] - m[0] * m[7]) / det,
            1
        });
}

vec2f mat3f::GetTranslation() const
{
    return vec2f(m[6], m[7]);
}

float mat3f::GetRotation() const
{
    return atan2(m[1], m[0]);
}

vec2f mat3f::GetScale() const
{
    return vec2f(
        sqrt(m[0] * m[0] + m[1] * m[1]),
        sqrt(m[3] * m[3] + m[4] * m[4])
    );
}

vec2f mat3f::TransformPoint(vec2f point) const
{
    return vec2f(
        m[0] * point.x + m[3] * point.y + m[6],
        m[1] * point.x + m[4] * point.y + m[7]
    );
}