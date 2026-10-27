#include "Quaternion.h"

#include <cmath>

/// <summary>
/// 積
/// w = lw*rw - (lv・rv)、v = lw*rv + rw*lv + (lv×rv)
/// </summary>
/// <param name="lhs"></param>
/// <param name="rhs"></param>
/// <returns></returns>
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {

	Quaternion result;

	result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
	result.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
	result.z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
	result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;

	return result;
}

/// <summary>
/// 乗法単位元を返す
/// </summary>
/// <returns></returns>
Quaternion Identity() {

	return {0.0f, 0.0f, 0.0f, 1.0f};
}

/// <summary>
/// 共役を返す
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
Quaternion Conjugate(const Quaternion& quaternion) {

	return {-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w};
}

/// <summary>
/// ノルムを返す
/// 4成分の2乗和の平方根
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
float Norm(const Quaternion& quaternion) {

	return std::sqrt(
		quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w
	);
}

/// <summary>
/// 正規化
/// 各成分をノルムで割る
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
Quaternion Normalize(const Quaternion& quaternion) {

	float norm = Norm(quaternion);
	// 0除算を防ぐ
	if (norm == 0.0f) {
		return quaternion;
	}

	return {quaternion.x / norm, quaternion.y / norm, quaternion.z / norm, quaternion.w / norm};
}

/// <summary>
/// 逆
/// 共役 / ノルム
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
Quaternion Inverse(const Quaternion& quaternion) {

	float norm = Norm(quaternion);
	float normSq = norm * norm;
	// 0除算を防ぐ
	if (normSq == 0.0f) {
		return quaternion;
	}
	Quaternion conj = Conjugate(quaternion);

	return { conj.x / normSq, conj.y / normSq, conj.z / normSq, conj.w / normSq };
}

///// ----- 回転用の関数 ----- /////

/// --- 任意軸回転 ---
// q = (n * sin(θ/2), cos(θ/2))
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	float halfAngle = angle * 0.5f;
	float sinHalf = std::sin(halfAngle);

	Quaternion result;
	result.x = axis.x * sinHalf;
	result.y = axis.y * sinHalf;
	result.z = axis.z * sinHalf;
	result.w = std::cos(halfAngle);
	return result;
}

/// --- ベクトルの回転 ---
// r' = q * r * q*
Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	// ベクトルを Quaternion にする (w = 0)
	Quaternion r = { vector.x, vector.y, vector.z, 0.0f };

	// q * r * 共役q
	Quaternion rotated = Multiply(Multiply(quaternion, r), Conjugate(quaternion));

	// ベクトル部を取り出す
	return { rotated.x, rotated.y, rotated.z };
}

/// --- 回転行列 ---
Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
	float x = quaternion.x;
	float y = quaternion.y;
	float z = quaternion.z;
	float w = quaternion.w;

	Matrix4x4 result;

	// 1行目
	result.m[0][0] = w * w + x * x - y * y - z * z;
	result.m[0][1] = 2.0f * (x * y + w * z);
	result.m[0][2] = 2.0f * (x * z - w * y);
	result.m[0][3] = 0.0f;

	// 2行目
	result.m[1][0] = 2.0f * (x * y - w * z);
	result.m[1][1] = w * w - x * x + y * y - z * z;
	result.m[1][2] = 2.0f * (y * z + w * x);
	result.m[1][3] = 0.0f;

	// 3行目
	result.m[2][0] = 2.0f * (x * z + w * y);
	result.m[2][1] = 2.0f * (y * z - w * x);
	result.m[2][2] = w * w - x * x - y * y + z * z;
	result.m[2][3] = 0.0f;

	// 4行目
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

///// ----- ===== ----- /////