#pragma once

#include <math.h>
#include <exception>
#include <algorithm>

class Vector2 {
public:
    Vector2(float x = 0.0f, float y = 0.0f): X(x), Y(y) {}
    Vector2(const Vector2& mE)            = default;
    Vector2(Vector2&& mE)                 = default;
    Vector2& operator=(const Vector2& mE) = default;
    Vector2& operator=(Vector2&& mE)      = default;
    Vector2 operator +(const Vector2& other) const {
        Vector2 result;
        result.X = X + other.X;
        result.Y = Y + other.Y;
        return result;
    }
    Vector2 operator -(const Vector2& other) const {
        Vector2 result;
        result.X = X - other.X;
        result.Y = Y - other.Y;
        return result;
    }
    Vector2 operator *(float value) const {
        Vector2 result = *this;
        result.X *= value;
        result.Y *= value;
        return result;
    }
    Vector2 operator /(float value) const {
        Vector2 result = *this;
        result.X /= value;
        result.Y /= value;
        return result;
    }
    void operator +=(const Vector2& other) {
        X += other.X;
        Y += other.Y;
    }
    void operator -=(const Vector2& other) {
        X -= other.X;
        Y -= other.Y;
    }
    void operator *=(float value) {
        X *= value;
        Y *= value;
    }
    void operator /=(float value) {
        X /= value;
        Y /= value;
    }
    Vector2 Normalized() const {
        Vector2 result = *this;
        float len = Magnitude();
        result /= len;
        return result;
    }
    float Magnitude() const {
        return sqrt(Dot(*this));
    }
    float SqrMagnitude() const {
        return Dot(*this);
    }
    float Dot(const Vector2& other) const {
        return X * other.X + Y * other.Y;
    }
public:
    float X;
    float Y;
};

class Vector3 {
public:
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f): X(x), Y(y), Z(z) {}
    Vector3(const Vector3& mE)            = default;
    Vector3(Vector3&& mE)                 = default;
    Vector3& operator=(const Vector3& mE) = default;
    Vector3& operator=(Vector3&& mE)      = default;
    Vector3 operator +(const Vector3& other) const {
        Vector3 result;
        result.X = X + other.X;
        result.Y = Y + other.Y;
        result.Z = Z + other.Z;
        return result;
    }
    Vector3 operator -(const Vector3& other) const {
        Vector3 result;
        result.X = X - other.X;
        result.Y = Y - other.Y;
        result.Z = Z - other.Z;
        return result;
    }
    Vector3 operator *(float value) const {
        Vector3 result = *this;
        result.X *= value;
        result.Y *= value;
        result.Z *= value;
        return result;
    }
    Vector3 operator /(float value) const {
        Vector3 result = *this;
        result.X /= value;
        result.Y /= value;
        result.Z /= value;
        return result;
    }
    void operator +=(const Vector3& other) {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
    }
    void operator -=(const Vector3& other) {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
    }
    void operator *=(float value) {
        X *= value;
        Y *= value;
        Z *= value;
    }
    void operator /=(float value) {
        X /= value;
        Y /= value;
        Z /= value;
    }
    Vector3 Normalized() const {
        Vector3 result = *this;
        float len = Magnitude();
        result /= len;
        return result;
    }
    float Magnitude() const {
        return sqrt(Dot(*this));
    }
    float SqrMagnitude() const {
        return Dot(*this);
    }
    float Dot(const Vector3& other) const {
        return X * other.X + Y * other.Y + Z * other.Z;
    }
    Vector3 Reflect(const Vector3& normal) const {
        return (normal * normal.Dot(*this) * 2.0f - *this).Normalized();
    }
    Vector3 Refract(const Vector3& normal, float cf) const {
        float c = -std::max(-1.0f, std::min(1.0f, Dot(normal)));
        float firstCF = 1;
        float secondCF = cf;
        Vector3 n = normal;
        if (c < 0) {
            c = -c;
            std::swap(firstCF, secondCF);
        }
        cf = firstCF / secondCF;
        float k = 1 - firstCF * firstCF * (1 - c * c);
        if (k < 0) {
            return {};
        }
        return *this * cf + normal * (cf * c - sqrtf(k));
    }
public:
    float X;
    float Y;
    float Z;
};

//void RunLinmathTests() {
//    auto a = Vector2(10, 10);
//    auto b = a.Normalized();
//    if (fabs(b.Magnitude()-1) > 0.00001f) {
//        throw std::exception();
//    }
//}