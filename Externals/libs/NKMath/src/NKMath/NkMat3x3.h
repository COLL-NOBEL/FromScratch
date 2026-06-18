#pragma once
#include <cmath>

// Matrice 3x3 stockee en row-major: m[ligne][colonne]
template<class T> class NkMat3x3
{
private:
    T m[3][3];

public:
    // ============================================================
    // Constructeurs: par defaut (identite), avec tableau plat
    // ============================================================

    // Matrice identite par defaut
    NkMat3x3()
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = (i == j) ? T(1) : T(0);
    }

    // Initialisation avec 9 valeurs (ligne par ligne)
    NkMat3x3(T a00, T a01, T a02,
             T a10, T a11, T a12,
             T a20, T a21, T a22)
    {
        m[0][0] = a00; m[0][1] = a01; m[0][2] = a02;
        m[1][0] = a10; m[1][1] = a11; m[1][2] = a12;
        m[2][0] = a20; m[2][1] = a21; m[2][2] = a22;
    }

    // Initialisation avec tableau plat de 9 elements (row-major)
    NkMat3x3(T data[9])
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = data[i * 3 + j];
    }

    // Acces par indice
    T& operator ()(int row, int col)             { return m[row][col]; }
    const T& operator ()(int row, int col) const { return m[row][col]; }

    // ============================================================
    // Operateurs: addition, soustraction, multiplication
    //             par un scalaire, multiplication matricielle
    // ============================================================

    NkMat3x3 operator +(const NkMat3x3& other) const
    {
        NkMat3x3 result;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                result.m[i][j] = m[i][j] + other.m[i][j];
        return result;
    }

    NkMat3x3 operator -(const NkMat3x3& other) const
    {
        NkMat3x3 result;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                result.m[i][j] = m[i][j] - other.m[i][j];
        return result;
    }

    NkMat3x3 operator *(T scalar) const
    {
        NkMat3x3 result;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                result.m[i][j] = m[i][j] * scalar;
        return result;
    }

    // Multiplication matricielle (3x3 * 3x3)
    NkMat3x3 operator *(const NkMat3x3& other) const
    {
        NkMat3x3 result;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
                result.m[i][j] = T(0);
                for (int k = 0; k < 3; k++)
                    result.m[i][j] += m[i][k] * other.m[k][j];
            }
        return result;
    }

    // ============================================================
    // Methodes: transpose, inverse, determinant, rotation, scale
    // ============================================================

    // Transposee: echange lignes et colonnes
    NkMat3x3 Transpose() const
    {
        NkMat3x3 result;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                result.m[i][j] = m[j][i];
        return result;
    }

    // Determinant de la matrice 3x3
    T Determinant() const
    {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
             - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
             + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    // Inverse (via matrice des cofacteurs / determinant)
    // Retourne la matrice identite si le determinant est nul
    NkMat3x3 Inverse() const
    {
        T det = Determinant();
        if (std::abs(float(det)) < 0.0001f) return NkMat3x3(); // matrice singuliere

        NkMat3x3 inv;
        inv.m[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) / det;
        inv.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) / det;
        inv.m[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det;
        inv.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) / det;
        inv.m[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det;
        inv.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) / det;
        inv.m[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det;
        inv.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) / det;
        inv.m[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det;
        return inv;
    }

    // Rotation 2D (en radians) — utile pour transformer des vecteurs 2D en homogene
    static NkMat3x3 Rotation(float angleRad)
    {
        T c = T(std::cos(angleRad));
        T s = T(std::sin(angleRad));
        return NkMat3x3(
             c, -s, T(0),
             s,  c, T(0),
            T(0), T(0), T(1)
        );
    }

    // Scale 2D (sx, sy) en coordonnees homogenes
    static NkMat3x3 Scale(T sx, T sy)
    {
        return NkMat3x3(
            sx,   T(0), T(0),
            T(0), sy,   T(0),
            T(0), T(0), T(1)
        );
    }

    // Translation 2D en coordonnees homogenes
    static NkMat3x3 Translation(T tx, T ty)
    {
        return NkMat3x3(
            T(1), T(0), tx,
            T(0), T(1), ty,
            T(0), T(0), T(1)
        );
    }
};
