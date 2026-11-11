#include "Quaternion.h"

#include <cmath>
#include <algorithm>

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

	return {conj.x / normSq, conj.y / normSq, conj.z / normSq, conj.w / normSq};
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
	Quaternion r = {vector.x, vector.y, vector.z, 0.0f};

	// q * r * 共役q
	Quaternion rotated = Multiply(Multiply(quaternion, r), Conjugate(quaternion));

	// ベクトル部を取り出す
	return {rotated.x, rotated.y, rotated.z};
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

/// --- ある方向から別の方向への回転 ---

// q = normalize(from×to, 1 + from・to)
Quaternion DirectionToDirection(const Vector3& from, const Vector3& to) {
	const float tolerance = 1.0e-6f;
	float dot = Vector3::Dot(from, to);

	// 真逆の向きは q0 が0になり正規化できないので、垂直な軸の周りにπ回転する
	if (dot <= -1.0f + tolerance) {
		// from が z軸方向でなければこの軸を使う
		Vector3 axis = {from.y, -from.x, 0.0f};

		// from が z軸方向のときは最初の候補がほぼ0になるので、別の軸を使う
		if (Vector3::Length(axis) < tolerance) {
			axis = {from.z, 0.0f, -from.x};
		}
		axis = Vector3::Normalize(axis);

		// q = (0, n)
		return {axis.x, axis.y, axis.z, 0.0f};
	}

	Vector3 cross = Vector3::Cross(from, to);

	// 倍率付きの回転 q0 を作る
	Quaternion rotation = {cross.x, cross.y, cross.z, 1.0f + dot};

	// 全体を正規化して倍率を取り除く
	return Normalize(rotation);
}

/// <summary>
/// 球面線形補間 (Slerp : Spherical Linear Interpolation)
/// Slerp(q0, q1, t) = sin((1-t)θ) / sinθ * q0 + sin(tθ) / sinθ * q1
/// </summary>
/// <param name="q0"></param>
/// <param name="q1"></param>
/// <param name="t"></param>
/// <returns></returns>
Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {

	// 符号を反転することがあるのでコピーしておく
	// (引数は const なので反転用にコピーする)
	Quaternion start = q0;

	// q0 と q1 の内積
	float dot = start.x * q1.x + start.y * q1.y + start.z * q1.z + start.w * q1.w;

	// 最短経路か
	// 内積が負なら、もう片方の回転 (-q0) を使って最短経路にする
	if (dot < 0.0f) {
		start = {
			-start.x,
			-start.y,
			-start.z,
			-start.w
		};
		dot = -dot;
	}

	// 誤差で 1 を少し超えると acos が NaN になるので抑える
	// (誤差で 1 を超えると acos が NaN になるので抑える)
	dot = std::min(dot, 1.0f);

	// なす角を求める
	float theta = std::acos(dot);
	float sinTheta = std::sin(theta);

	// 補間係数
	float scale0;
	float scale1;

	// なす角がほぼ 0 のときは sinθ で割れないので線形補間にする
	// (sinθ がほぼ 0 だと割れないので線形補間で代用する)
	const float epsilon = 1.0e-5f;
	if (sinTheta < epsilon) {
		scale0 = 1.0f - t;
		scale1 = t;
	} else {
		scale0 = std::sin((1.0f - t) * theta) / sinTheta;
		scale1 = std::sin(t * theta) / sinTheta;
	}

	// それぞれの補間係数を利用して補間後の Quaternion を求める
	return {
		scale0 * start.x + scale1 * q1.x,
		scale0 * start.y + scale1 * q1.y,
		scale0 * start.z + scale1 * q1.z,
		scale0 * start.w + scale1 * q1.w
	};
}

///// ----- ===== ----- /////