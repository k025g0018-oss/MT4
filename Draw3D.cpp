#define _USE_MATH_DEFINES
#include "Draw3D.h"
#include <Novice.h>
#include <cmath>

// グリッド線の描画
void Draw3D::DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f; // Gridの半分の幅
	const uint32_t kSubdivision = 10; // 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision); // 1つ文の長さ

	// 奥から手前への線を順々にひいていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		// 上の情報を使ってワールド座標系上の始点と終点を求める
		// スクリーン座標系まで変換を掛ける
		// 変換した座標を使って表示
		float x = -kGridHalfWidth + (xIndex * kGridEvery);
		unsigned int color = (x == 0.0f) ? 0x000000FF : 0xAAAAAAFF; // 中心線は黒、他は白

		// 始点と終点（Z方向の線）
		Vector3 start = {x, 0, -kGridHalfWidth};
		Vector3 end = {x, 0, kGridHalfWidth};

		Vector3 screenStart = Vector3::Transform(Vector3::Transform({x, 0, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Vector3::Transform(Vector3::Transform({x, 0, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);

		Novice::DrawLine((int)screenStart.x, (int)screenStart.y, (int)screenEnd.x, (int)screenEnd.y, color);
	}

	// 左から右も同じように
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);
		unsigned int color = (z == 0.0f) ? 0x000000FF : 0xAAAAAAFF;

		// 始点と終点（X方向の線）
		Vector3 start = {-kGridHalfWidth, 0, z};
		Vector3 end = {kGridHalfWidth, 0, z};

		Vector3 screenStart = Vector3::Transform(Vector3::Transform({-kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Vector3::Transform(Vector3::Transform({kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);

		Novice::DrawLine((int)screenStart.x, (int)screenStart.y, (int)screenEnd.x, (int)screenEnd.y, color);
	}
}

// 3次元空間の線分を描画
void Draw3D::DrawSegment(
	const Segment& segment,
	const Matrix4x4& viewProjectionMatrix,
	const Matrix4x4& viewportMatrix,
	uint32_t color
) {
	// 始点と終点をスクリーン座標へ変換
	Vector3 start = Vector3::Transform(
		Vector3::Transform(segment.origin, viewProjectionMatrix),
		viewportMatrix
	);
	Vector3 end = Vector3::Transform(
		Vector3::Transform(segment.origin + segment.diff, viewProjectionMatrix),
		viewportMatrix
	);

	// スクリーン座標上の2点を線で結ぶ
	Novice::DrawLine(
		static_cast<int>(start.x), static_cast<int>(start.y),
		static_cast<int>(end.x), static_cast<int>(end.y),
		color
	);
}

// Sphereを表示する
void Draw3D::DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	const uint32_t kSubdivision = 16; // 分割数
	const float kLonEvery = (float)M_PI * 2.0f / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = (float)M_PI / kSubdivision;; // 緯度分割1つ分の角度

	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex; // 現在の緯度

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の緯度

			/// world座標系でのa,b,cを求める
			// a
			Vector3 a;
			a.x = sphere.radius * cosf(lat) * cosf(lon);
			a.y = sphere.radius * sinf(lat);
			a.z = sphere.radius * cosf(lat) * sinf(lon);
			a = a + sphere.center;

			// b
			Vector3 b;
			b.x = sphere.radius * cosf(lat) * cosf(lon + kLonEvery);
			b.y = sphere.radius * sinf(lat);
			b.z = sphere.radius * cosf(lat) * sinf(lon + kLonEvery);
			b = b + sphere.center;

			// c
			Vector3 c;
			c.x = sphere.radius * cosf(lat + kLatEvery) * cosf(lon);
			c.y = sphere.radius * sinf(lat + kLatEvery);
			c.z = sphere.radius * cosf(lat + kLatEvery) * sinf(lon);
			c = c + sphere.center;

			// a,b,cをScreen座標系まで変換
			Vector3 sa = Vector3::Transform(Vector3::Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 sb = Vector3::Transform(Vector3::Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 sc = Vector3::Transform(Vector3::Transform(c, viewProjectionMatrix), viewportMatrix);

			// ab,bcで線を引く
			Novice::DrawLine((int)sa.x, (int)sa.y, (int)sb.x, (int)sb.y, color);
			Novice::DrawLine((int)sa.x, (int)sa.y, (int)sc.x, (int)sc.y, color);
		}
	}
}

// 平面の描画
Vector3 Draw3D::Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}

	return {0.0f, -vector.z, vector.y};
}

void Draw3D::DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	Vector3 center = plane.normal * plane.distance; // 1
	Vector3 perpendiculars[4];
	perpendiculars[0] = Vector3::Normalize(Perpendicular(plane.normal)); // 2
	perpendiculars[1] = {-perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z}; // 3
	perpendiculars[2] = Vector3::Cross(plane.normal, perpendiculars[0]); // 4
	perpendiculars[3] = {-perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z}; // 5
	// 6
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = perpendiculars[index] * 2.0f;
		Vector3 point = center + extend;
		points[index] = Vector3::Transform(Vector3::Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	// pointsをそれぞれ結んでDrawLineで矩形を描画する
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
}

// 三角形の描画
void Draw3D::DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i) {
		screenVertices[i] = Vector3::Transform(Vector3::Transform(triangle.vertices[i], viewProjectionMatrix), viewportMatrix);
	}

	// 3つの頂点を線で結ぶ
	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[1].x, (int)screenVertices[1].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[2].x, (int)screenVertices[2].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[0].x, (int)screenVertices[0].y, color);
}

// ---AABBの描画関数---
void Draw3D::DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	// AABBの8つの頂点を定義
	Vector3 vertices[8] = {
		{aabb.min.x, aabb.min.y, aabb.min.z}, // 0: 左下前
		{aabb.max.x, aabb.min.y, aabb.min.z}, // 1: 右下前
		{aabb.max.x, aabb.min.y, aabb.max.z}, // 2: 右下奥
		{aabb.min.x, aabb.min.y, aabb.max.z}, // 3: 左下奥
		{aabb.min.x, aabb.max.y, aabb.min.z}, // 4: 左上前
		{aabb.max.x, aabb.max.y, aabb.min.z}, // 5: 右上前
		{aabb.max.x, aabb.max.y, aabb.max.z}, // 6: 右上奥
		{aabb.min.x, aabb.max.y, aabb.max.z}  // 7: 左上奥
	};

	// 描画用のスクリーン座標に変換
	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		Vector3 ndc = Vector3::Transform(vertices[i], viewProjectionMatrix);
		screenVertices[i] = Vector3::Transform(ndc, viewportMatrix);
	}

	// ラインを結ぶインデックス（12本の辺）
	int indices[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 底面
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 上面
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 側面（柱）
	};

	// 12本の線を描画
	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
			int(screenVertices[indices[i][0]].x), int(screenVertices[indices[i][0]].y),
			int(screenVertices[indices[i][1]].x), int(screenVertices[indices[i][1]].y),
			color
		);
	}
}

/// --- OBB描画 ---
void Draw3D::DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	// ローカル空間での8頂点（中心が原点なので -size ～ +size）
	Vector3 vertices[8] = {
		{-obb.size.x, -obb.size.y, -obb.size.z}, {obb.size.x, -obb.size.y, -obb.size.z},
		{obb.size.x, -obb.size.y, obb.size.z}, {-obb.size.x, -obb.size.y, obb.size.z},
		{-obb.size.x, obb.size.y, -obb.size.z}, {obb.size.x, obb.size.y, -obb.size.z},
		{obb.size.x, obb.size.y, obb.size.z}, {-obb.size.x, obb.size.y, obb.size.z}
	};

	// WorldMatrixを取得
	Matrix4x4 obbWorldMatrix = Matrix4x4::MakeOBBWorldMatrix(obb);

	// 各頂点をワールド座標へ変換し、さらにスクリーン座標へ変換
	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		// World変換
		Vector3 worldPos = Vector3::Transform(vertices[i], obbWorldMatrix);
		// ViewProjection & Viewport変換
		Vector3 ndc = Vector3::Transform(worldPos, viewProjectionMatrix);
		screenVertices[i] = Vector3::Transform(ndc, viewportMatrix);
	}

	// 12本の辺を描画
	int indices[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 底面
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 上面
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 側面
	};

	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
			(int)screenVertices[indices[i][0]].x, (int)screenVertices[indices[i][0]].y,
			(int)screenVertices[indices[i][1]].x, (int)screenVertices[indices[i][1]].y,
			color
		);
	}
}

// 3次元空間に2次ベジェ曲線を描画
void Draw3D::DrawBezier(
	const Vector3& controlPoint0,
	const Vector3& controlPoint1,
	const Vector3& controlPoint2,
	const Matrix4x4& viewProjectionMatrix,
	const Matrix4x4& viewportMatrix,
	uint32_t color
) {
	// 曲線を32本の短い線分に分けて描画する
	const uint32_t kSubdivision = 32;

	for (uint32_t i = 0; i < kSubdivision; ++i) {
		// 現在の線分の始点と終点に対応する割合を求める
		float t0 = static_cast<float>(i) / kSubdivision;
		float t1 = static_cast<float>(i + 1) / kSubdivision;

		// 制御点0と1、制御点1と2の間をt0で線形補間
		Vector3 p01 = Vector3::Lerp(controlPoint0, controlPoint1, t0);
		Vector3 p12 = Vector3::Lerp(controlPoint1, controlPoint2, t0);

		// 2つの補間点をさらに補間して、曲線上の始点を求める
		Vector3 point0 = Vector3::Lerp(p01, p12, t0);

		// 制御点0と1、制御点1と2の間をt1で線形補間
		p01 = Vector3::Lerp(controlPoint0, controlPoint1, t1);
		p12 = Vector3::Lerp(controlPoint1, controlPoint2, t1);

		// 2つの補間点をさらに補間して、曲線上の終点を求める
		Vector3 point1 = Vector3::Lerp(p01, p12, t1);

		// 曲線上の2点をワールド座標からスクリーン座標へ変換
		Vector3 screen0 = Vector3::Transform(
			Vector3::Transform(point0, viewProjectionMatrix),
			viewportMatrix
		);
		Vector3 screen1 = Vector3::Transform(
			Vector3::Transform(point1, viewProjectionMatrix),
			viewportMatrix
		);

		// 変換した2点を線で結び、曲線の一部分として描画
		Novice::DrawLine(
			static_cast<int>(screen0.x),
			static_cast<int>(screen0.y),
			static_cast<int>(screen1.x),
			static_cast<int>(screen1.y),
			color
		);
	}
}