#pragma once

#include "NkMat4x4.h"

template<class T> class NkMat4x4
{
    // ============================================================
    // Transformations 3D en coordonnees homogenes (methodes statiques)
    // ============================================================

    // Translation (deplace un objet de tx, ty, tz)
    //
    //  | 1  0  0  tx |
    //  | 0  1  0  ty |
    //  | 0  0  1  tz |
    //  | 0  0  0  1  |
    static NkMat4x4 Translation(T tx, T ty, T tz)
    {
        NkMat4x4 result;
        result.m[0][3] = tx;
        result.m[1][3] = ty;
        result.m[2][3] = tz;
        return result;
    }

    // Scale (mise a l'echelle sur chaque axe)
    //
    //  | sx  0   0   0 |
    //  | 0   sy  0   0 |
    //  | 0   0   sz  0 |
    //  | 0   0   0   1 |
    static NkMat4x4 Scale(T sx, T sy, T sz)
    {
        NkMat4x4 result;
        result.m[0][0] = sx;
        result.m[1][1] = sy;
        result.m[2][2] = sz;
        return result;
    }

    // Rotation autour de l'axe X (en radians)
    //
    //  | 1   0    0   0 |
    //  | 0   cos -sin 0 |
    //  | 0   sin  cos 0 |
    //  | 0   0    0   1 |
    static NkMat4x4 RotationX(float angleRad)
    {
        T c = T(std::cos(angleRad));
        T s = T(std::sin(angleRad));
        NkMat4x4 result;
        result.m[1][1] =  c;
        result.m[1][2] = -s;
        result.m[2][1] =  s;
        result.m[2][2] =  c;
        return result;
    }

    // Rotation autour de l'axe Y (en radians)
    //
    //  |  cos  0  sin  0 |
    //  |  0    1  0    0 |
    //  | -sin  0  cos  0 |
    //  |  0    0  0    1 |
    static NkMat4x4 RotationY(float angleRad)
    {
        T c = T(std::cos(angleRad));
        T s = T(std::sin(angleRad));
        NkMat4x4 result;
        result.m[0][0] =  c;
        result.m[0][2] =  s;
        result.m[2][0] = -s;
        result.m[2][2] =  c;
        return result;
    }

    // Rotation autour de l'axe Z (en radians)
    //
    //  | cos -sin  0  0 |
    //  | sin  cos  0  0 |
    //  | 0    0    1  0 |
    //  | 0    0    0  1 |
    static NkMat4x4 RotationZ(float angleRad)
    {
        T c = T(std::cos(angleRad));
        T s = T(std::sin(angleRad));
        NkMat4x4 result;
        result.m[0][0] =  c;
        result.m[0][1] = -s;
        result.m[1][0] =  s;
        result.m[1][1] =  c;
        return result;
    }

    // Rotation autour d'un axe arbitraire (formule de Rodrigues)
    // axis doit etre un vecteur normalise
    static NkMat4x4 RotationAxis(float ax, float ay, float az, float angleRad)
    {
        T c  = T(std::cos(angleRad));
        T s  = T(std::sin(angleRad));
        T t  = T(1) - c;

        NkMat4x4 result;
        result.m[0][0] = t*ax*ax + c;
        result.m[0][1] = t*ax*ay - s*az;
        result.m[0][2] = t*ax*az + s*ay;

        result.m[1][0] = t*ax*ay + s*az;
        result.m[1][1] = t*ay*ay + c;
        result.m[1][2] = t*ay*az - s*ax;

        result.m[2][0] = t*ax*az - s*ay;
        result.m[2][1] = t*ay*az + s*ax;
        result.m[2][2] = t*az*az + c;
        return result;
    }

    // ============================================================
    // Interpolations entre deux matrices (element par element)
    // Note : interpoler des matrices de transformation directement
    //        fonctionne bien pour la translation et le scale.
    //        Pour la rotation, il est preferable d'utiliser des
    //        quaternions (non couverts ici).
    // ============================================================

    // Interpolation Lineaire (Lerp)
    // Calcule une matrice intermediaire en interpolant chaque element.
    // t=0 -> matrice 'a'  |  t=1 -> matrice 'b'
    static NkMat4x4 Lerp(const NkMat4x4& a, const NkMat4x4& b, float t)
    {
        NkMat4x4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = a.m[i][j] + (b.m[i][j] - a.m[i][j]) * T(t);
        return result;
    }

    // Interpolation Cubique de Hermite
    // Interpole entre les matrices 'a' et 'b' avec des matrices de
    // tangentes 'ta' et 'tb' qui controlent l'acceleration en debut
    // et en fin de transition (ease-in / ease-out).
    // Polynomes de base :
    //   h00 =  2t3 - 3t2 + 1   h10 =  t3 - 2t2 + t
    //   h01 = -2t3 + 3t2       h11 =  t3 -  t2
    static NkMat4x4 CubicHermite(const NkMat4x4& a, const NkMat4x4& ta,
                                   const NkMat4x4& b, const NkMat4x4& tb,
                                   float t)
    {
        float t2 = t * t;
        float t3 = t2 * t;

        float h00 =  2.0f*t3 - 3.0f*t2 + 1.0f;
        float h10 =       t3 - 2.0f*t2 + t;
        float h01 = -2.0f*t3 + 3.0f*t2;
        float h11 =       t3 -      t2;

        NkMat4x4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = T(h00)*a.m[i][j] + T(h10)*ta.m[i][j]
                                + T(h01)*b.m[i][j] + T(h11)*tb.m[i][j];
        return result;
    }

    // Interpolation Cubique de Bezier
    // Interpole entre p0 et p3 en passant pres de p1 et p2 (sans
    // necessairement les traverser) selon les polynomes de Bernstein.
    // Tres utilise pour les animations de camera et les chemins d'objets.
    // Polynomes de Bernstein :
    //   B0 = (1-t)3   B1 = 3(1-t)2*t   B2 = 3(1-t)*t2   B3 = t3
    static NkMat4x4 CubicBezier(const NkMat4x4& p0, const NkMat4x4& p1,
                                  const NkMat4x4& p2, const NkMat4x4& p3,
                                  float t)
    {
        float u  = 1.0f - t;
        float u2 = u * u;
        float u3 = u2 * u;
        float t2 = t * t;
        float t3 = t2 * t;

        float b0 =      u3;
        float b1 = 3.0f*u2*t;
        float b2 = 3.0f*u *t2;
        float b3 =      t3;

        NkMat4x4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = T(b0)*p0.m[i][j] + T(b1)*p1.m[i][j]
                                + T(b2)*p2.m[i][j] + T(b3)*p3.m[i][j];
        return result;
    }
};