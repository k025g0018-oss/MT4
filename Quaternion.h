#pragma once

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