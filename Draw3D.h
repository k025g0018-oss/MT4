#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Geometry3D.h"
#include <cstdint>

class Draw3D {
public:
	/// --- 描画 ---
	// グリッドの描画
	static void DrawGrid(
		const Matrix4x4& viewProjectionMatrix,
		const Matrix4x4& viewportMatrix
	);

	// 線分の描画
	static void DrawSegment(
		const Segment& segment,
		const Matrix4x4& viewProjectionMatrix,
		const Matrix4x4& viewportMatrix,
		uint32_t color
	);

	// 球の描画
	static void DrawSphere(
		const Sphere& sphere,
		const Matrix4x4& viewProjectionMatrix,
		const Matrix4x4& viewportMatrix,
		uint32_t color
	);

	// 平面の描画
	static void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color);

	// 三角形の描画
	static void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color);

	// ---AABBの描画関数---
	static void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color);

	/// --- OBB描画 ---
	static void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color);

	// 2次ベジェ曲線
	static void DrawBezier(
		const Vector3& controlPoint0,
		const Vector3& controlPoint1,
		const Vector3& controlPoint2,
		const Matrix4x4& viewProjectionMatrix,
		const Matrix4x4& viewportMatrix,
		uint32_t color
	);

private:
	// 平面描画用の垂直ベクトルを求める
	static Vector3 Perpendicular(const Vector3& vector);
};