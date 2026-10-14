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