#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

/// <summary>
/// Quaternion
/// </summary>
struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

///// ----- Quaternion の関数 ----- /////

// Quaternion の積
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
// 乗法単位元を返す
Quaternion Identity();
// 共役 Quaternion を返す
Quaternion Conjugate(const Quaternion& quaternion);
// Quaternion の norm を返す
float Norm(const Quaternion& quaternion);
// 正規化した Quaternion を返す
Quaternion Normalize(const Quaternion& quaternion);
// 逆 Quaternion を返す
Quaternion Inverse(const Quaternion& quaternion);

///// ----- ===== ----- /////

///// ----- 回転用の関数 ----- /////

// 任意軸回転を表す Quaternion を作る
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);
// ベクトルを Quaternion で回転させる
Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);
// Quaternion から回転行列を作る
Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

///// ----- ===== ----- /////