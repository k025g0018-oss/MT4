#pragma once

// 前方宣言
struct Matrix4x4;

// --- Vector3 ---
struct Vector3 {
	float x, y, z;

	// 演算子オーバーロードの宣言
	/*
	Vector3 operator+(const Vector3& obj) const;
	Vector3 operator-(const Vector3& obj) const;
	Vector3 operator*(float scalar) const;
	*/

	// ---静的関数の宣言---
	// 内積
	static float Dot(const Vector3& v1, const Vector3& v2);
	// 長さ(ノルム)
	static float Length(const Vector3& v);
	// 正規化
	static Vector3 Normalize(const Vector3& v);
	// クロス積
	static Vector3 Cross(const Vector3& v1, const Vector3& v2);
	// 正射影ベクトル
	static Vector3 Project(const Vector3& v1, const Vector3& v2);
	// Vector3の数値表示
	static void ScreenPrintf(int x, int y, const Vector3& vector, const char* label);
	// 座標変換
	static Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	// 線形補間
	static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
};

/// --- Vector3の演算子オーバーロード ---

// 2つのベクトルを加算する
Vector3 operator+(const Vector3& v1, const Vector3& v2);

// 左のベクトルから右のベクトルを減算する
Vector3 operator-(const Vector3& v1, const Vector3& v2);

// ベクトルをスカラー倍する
Vector3 operator*(const Vector3& v, float scalar);

// 「スカラー * ベクトル」の順番にも対応する
Vector3 operator*(float scalar, const Vector3& v);

// ベクトルの符号をそのまま返す
Vector3 operator+(const Vector3& v);

// ベクトルのすべての符号を反転する
Vector3 operator-(const Vector3& v);

/// グローバル

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};