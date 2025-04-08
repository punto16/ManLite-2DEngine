#include "mat3f.h"
#include <cmath>

mat3f::mat3f() : m{ 1,0,0, 0,1,0, 0,0,1 } {}

mat3f::mat3f(const float* matrix)
{
    for (int i = 0; i < 9; ++i) m[i] = matrix[i];
}

mat3f mat3f::CreateTransformMatrix(vec2f position, float rotation_radians, vec2f scale) {
    float c = cos(rotation_radians);
    float s = sin(rotation_radians);

    // column-major (3 columns)
    float matrix[9] = {
            c* scale.x, s* scale.x, 0,      // Column 0 ( X)
            -s * scale.y, c* scale.y, 0,    // Column 1 ( Y)
            position.x, position.y, 1       // Column 2 (traslación)
    };
    return mat3f(matrix);
}

mat3f mat3f::operator*(const mat3f& other) const {
    float result[9] = { 0 };

    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            float sum = 0;
            for (int k = 0; k < 3; ++k) {
                sum += m[k * 3 + row] * other.m[col * 3 + k]; // column-major
            }
            result[col * 3 + row] = sum;
        }
    }

    return mat3f(result);
}

vec2f mat3f::operator*(const vec2f& point) const
{
    return TransformPoint(point);
}

mat3f mat3f::Inverted() const {
    float det = m[0] * m[4] - m[1] * m[3];
    if (det == 0) return mat3f();

    return mat3f(new float[9] {
            m[4] / det, -m[1] / det, 0,   // Col0
            -m[3] / det, m[0] / det, 0,   // Col1
            (m[3] * m[7] - m[4] * m[6]) / det,  // tx
            (m[1] * m[6] - m[0] * m[7]) / det,  // ty
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