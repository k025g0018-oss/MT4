#pragma once

#include "Vector3.h"

// 線
struct Line { // 直線
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Ray { // 半直線
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Segment { // 線分
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

// 球
struct Sphere {
	Vector3 center; // 中心点
	float radius; // 半径
	unsigned int color;
};

// 平面
struct Plane {
	Vector3 normal; // 法線
	float distance; // 距離
};

// 三角形
struct Triangle {
	Vector3 vertices[3]; // 頂点
};

// 振り子
struct Pendulum {
	Vector3 anchor; // アンカーポイント
	float length; // 紐の長さ
	float angle; // 現在の角度
	float angularVelocity; // 角速度ω
	float angularAcceleration; // 角加速度
};

// 円錐振り子
struct ConicalPendulum {
	Vector3 anchor; // アンカーポイント 固定された端の位置
	float length; // 紐の長さ
	float halfApexAngle; // 円錐の頂角の半分
	float angle; // 現在の角度
	float angularVelocity; // 角速度ω
};

// ==========================================

/// --- AABB構造体 ---
struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

/// --- OBB構造体 ---
// AABB + 回転 = OBB
struct OBB {
	Vector3 center; // 中心点
	Vector3 orientations[3]; // 座標軸 正規化・直交必須 3x3回転行列の各行
	Vector3 size; // 座標軸方向の長さの半分 中心から面までの距離S
};

// ==========================================
