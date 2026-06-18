#pragma once
#include <cmath>
#include "NkVec3.h"
#include "NkMat4x4.h"

// ============================================================
// NkQuat<T>  — unit quaternion  q = (x, y, z, w)
//   w = scalar part, (x,y,z) = vector part
//   Convention matches Unity / GLM / XNA.
// ============================================================
template<class T>
class NkQuat
{
public:
    T x, y, z, w;

    // ---- Constructors ----------------------------------------

    // Identity quaternion (no rotation)
    NkQuat() : x(T(0)), y(T(0)), z(T(0)), w(T(1)) {}

    NkQuat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}

    // ---- Arithmetic -----------------------------------------

    NkQuat operator+(const NkQuat& r) const { return { x+r.x, y+r.y, z+r.z, w+r.w }; }
    NkQuat operator-(const NkQuat& r) const { return { x-r.x, y-r.y, z-r.z, w-r.w }; }
    NkQuat operator*(T s)             const { return { x*s,   y*s,   z*s,   w*s   }; }

    // Hamilton product  (this ⊗ r)  — r applied AFTER this
    NkQuat operator*(const NkQuat& r) const
    {
        return {
             w*r.x + x*r.w + y*r.z - z*r.y,
             w*r.y - x*r.z + y*r.w + z*r.x,
             w*r.z + x*r.y - y*r.x + z*r.w,
             w*r.w - x*r.x - y*r.y - z*r.z
        };
    }

    bool operator==(const NkQuat& r) const {
        return x==r.x && y==r.y && z==r.z && w==r.w;
    }

    // ---- Rotate a vector (Rodrigues, 15 muls) ----------------
    NkVec3<T> operator*(const NkVec3<T>& v) const
    {
        // v' = 2(q_v·v)q_v + (s²-|q_v|²)v + 2s(q_v×v)
        NkVec3<T> qv(x, y, z);
        float dot   = qv.Dot(v);
        float qvSq  = qv.Dot(qv);
        NkVec3<T> cross = qv.CrossProduct(v);

        return NkVec3<T>(
            T(2)*dot*qv[0] + (w*w - qvSq)*v[0] + T(2)*w*cross[0],
            T(2)*dot*qv[1] + (w*w - qvSq)*v[1] + T(2)*w*cross[1],
            T(2)*dot*qv[2] + (w*w - qvSq)*v[2] + T(2)*w*cross[2]
        );
    }

    // ---- Queries --------------------------------------------

    T Dot(const NkQuat& r) const { return x*r.x + y*r.y + z*r.z + w*r.w; }

    float LengthSquared() const { return float(x*x + y*y + z*z + w*w); }
    float Length()        const { return std::sqrt(LengthSquared()); }

    NkQuat Normalize() const
    {
        float len = Length();
        if (len < 0.0001f) return NkQuat();
        float inv = 1.0f / len;
        return { T(x*inv), T(y*inv), T(z*inv), T(w*inv) };
    }

    NkQuat Conjugate() const { return { -x, -y, -z, w }; }
    NkQuat Inverse()   const { return Conjugate().Normalize(); }

    // ---- Direction helpers ----------------------------------

    NkVec3<T> Forward() const { return (*this) * NkVec3<T>( T(0),  T(0),  T(1)); }
    NkVec3<T> Back()    const { return (*this) * NkVec3<T>( T(0),  T(0), T(-1)); }
    NkVec3<T> Up()      const { return (*this) * NkVec3<T>( T(0),  T(1),  T(0)); }
    NkVec3<T> Down()    const { return (*this) * NkVec3<T>( T(0), T(-1),  T(0)); }
    NkVec3<T> Right()   const { return (*this) * NkVec3<T>( T(1),  T(0),  T(0)); }
    NkVec3<T> Left()    const { return (*this) * NkVec3<T>(T(-1),  T(0),  T(0)); }

    // ---- Convert to 4x4 rotation matrix ---------------------
    NkMat4x4<T> ToMatrix() const
    {
        T xx = x*x, yy = y*y, zz = z*z;
        T xy = x*y, xz = x*z, yz = y*z;
        T wx = w*x, wy = w*y, wz = w*z;

        NkMat4x4<T> m;
        m(0,0) = T(1) - T(2)*(yy+zz);  m(0,1) = T(2)*(xy-wz);          m(0,2) = T(2)*(xz+wy);          m(0,3) = T(0);
        m(1,0) = T(2)*(xy+wz);          m(1,1) = T(1) - T(2)*(xx+zz);  m(1,2) = T(2)*(yz-wx);          m(1,3) = T(0);
        m(2,0) = T(2)*(xz-wy);          m(2,1) = T(2)*(yz+wx);          m(2,2) = T(1) - T(2)*(xx+yy);  m(2,3) = T(0);
        m(3,0) = T(0);                   m(3,1) = T(0);                   m(3,2) = T(0);                   m(3,3) = T(1);
        return m;
    }

    // ---- Interpolation --------------------------------------

    // NLerp — fast, approximate constant-speed slerp
    static NkQuat NLerp(const NkQuat& a, const NkQuat& b, float t)
    {
        // Guarantee shortest path
        float d = a.Dot(b);
        NkQuat end = (d < 0.0f) ? NkQuat(-b.x,-b.y,-b.z,-b.w) : b;
        NkQuat blend = a * T(1.0f-t) + end * T(t);
        return blend.Normalize();
    }

    // SLerp — Shoemake 1985
    static NkQuat SLerp(const NkQuat& a, const NkQuat& b, float t)
    {
        float d = a.Dot(b);
        NkQuat end = (d < 0.0f) ? NkQuat(-b.x,-b.y,-b.z,-b.w) : b;
        d = (d < 0.0f) ? -d : d;
        if (d > 0.9995f) return NLerp(a, end, t);
        float theta0 = std::acos(d);
        float theta  = theta0 * t;
        float s0 = std::cos(theta) - d * std::sin(theta) / std::sin(theta0);
        float s1 = std::sin(theta) / std::sin(theta0);
        return (a * T(s0) + end * T(s1)).Normalize();
    }

    // ---- Static factories -----------------------------------

    static NkQuat Identity() { return NkQuat(T(0), T(0), T(0), T(1)); }

    // Rotation around X axis (pitch) in radians
    static NkQuat RotateX(float rad)
    {
        float h = rad * 0.5f;
        return { T(std::sin(h)), T(0), T(0), T(std::cos(h)) };
    }

    // Rotation around Y axis (yaw) in radians
    static NkQuat RotateY(float rad)
    {
        float h = rad * 0.5f;
        return { T(0), T(std::sin(h)), T(0), T(std::cos(h)) };
    }

    // Rotation around Z axis (roll) in radians
    static NkQuat RotateZ(float rad)
    {
        float h = rad * 0.5f;
        return { T(0), T(0), T(std::sin(h)), T(std::cos(h)) };
    }

    // Rotation around arbitrary axis (Rodrigues) — axis must be normalised
    static NkQuat RotateAxis(float ax, float ay, float az, float rad)
    {
        float h = rad * 0.5f;
        float s = std::sin(h);
        return { T(ax*s), T(ay*s), T(az*s), T(std::cos(h)) };
    }

    // Minimum-arc rotation that brings 'from' to 'to'
    static NkQuat FromTo(const NkVec3<T>& from, const NkVec3<T>& to)
    {
        NkVec3<T> f = from.Normalize();
        NkVec3<T> t = to.Normalize();
        float d = f.Dot(t);
        if (d > 0.9999f) return Identity();
        if (d < -0.9999f) {
            // 180° — pick arbitrary perpendicular axis
            NkVec3<T> ortho(T(1), T(0), T(0));
            if (std::abs(float(f[0])) > 0.9f) ortho = NkVec3<T>(T(0), T(1), T(0));
            NkVec3<T> axis = f.CrossProduct(ortho).Normalize();
            return RotateAxis(axis[0], axis[1], axis[2], 3.14159265f);
        }
        NkVec3<T> axis = f.CrossProduct(t);
        float s = std::sqrt((1.0f + d) * 2.0f);
        float inv = 1.0f / s;
        return NkQuat(T(axis[0]*inv), T(axis[1]*inv), T(axis[2]*inv), T(s*0.5f)).Normalize();
    }

    // LookAt: returns quaternion that orients +Z toward 'direction'
    static NkQuat LookAt(const NkVec3<T>& direction, const NkVec3<T>& up = NkVec3<T>(T(0),T(1),T(0)))
    {
        NkVec3<T> forward = direction.Normalize();
        NkVec3<T> right   = up.CrossProduct(forward).Normalize();
        NkVec3<T> actualUp= forward.CrossProduct(right).Normalize();

        // Build rotation matrix, then extract quaternion
        float m00 = right[0],    m10 = right[1],    m20 = right[2];
        float m01 = actualUp[0], m11 = actualUp[1], m21 = actualUp[2];
        float m02 = forward[0],  m12 = forward[1],  m22 = forward[2];

        float trace = m00 + m11 + m22;
        if (trace > 0.0f) {
            float s = 0.5f / std::sqrt(trace + 1.0f);
            return NkQuat(T((m21-m12)*s), T((m02-m20)*s), T((m10-m01)*s), T(0.25f/s)).Normalize();
        } else if (m00 > m11 && m00 > m22) {
            float s = 2.0f * std::sqrt(1.0f + m00 - m11 - m22);
            return NkQuat(T(0.25f*s), T((m01+m10)/s), T((m02+m20)/s), T((m21-m12)/s)).Normalize();
        } else if (m11 > m22) {
            float s = 2.0f * std::sqrt(1.0f + m11 - m00 - m22);
            return NkQuat(T((m01+m10)/s), T(0.25f*s), T((m12+m21)/s), T((m02-m20)/s)).Normalize();
        } else {
            float s = 2.0f * std::sqrt(1.0f + m22 - m00 - m11);
            return NkQuat(T((m02+m20)/s), T((m12+m21)/s), T(0.25f*s), T((m10-m01)/s)).Normalize();
        }
    }
};

// Common aliases
using NkQuatf = NkQuat<float>;
using NkQuatd = NkQuat<double>;