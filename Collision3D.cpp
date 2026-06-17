#include "Collision3D.h"
#define _USE_MATH_DEFINES
#include <Novice.h>
#include <algorithm>
#include <cmath>

// 正射影ベクトルと最近接点
Vector3 Collision3D::ClosestPoint(const Vector3& point, const Segment& segment) {
	// 始点から点へのベクトル
	Vector3 v = point - segment.origin;

	// 線分の向きベクトル（diff）との内積から、どの位置にいるか(t)を計算
	// t = (v1・v2) / |v2|^2
	float t = Vector3::Dot(v, segment.diff) / powf(Vector3::Length(segment.diff), 2.0f);

	// 線分なので 0.0(始点) ～ 1.0(終点) の間にクランプする
	t = (std::max)(0.0f, (std::min)(t, 1.0f));

	// 始点 + 向き * t
	return segment.origin + (segment.diff * t);
}

// 球と球の当たり判定
bool Collision3D::IsCollisionSphereAndSphere(const Sphere& s1, const Sphere& s2) {
	// 2つの球の中心点間の距離を求める
	float distance = Vector3::Length(s2.center - s1.center);

	// 半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}

	return false;
}

// 球と平面の当たり判定
bool Collision3D::IsCollisionSphereAndPlane(const Sphere& sphere, const Plane& plane) {
	float distanceFromPlane = Vector3::Dot(sphere.center, plane.normal) - plane.distance;

	// 距離の絶対値が半径以下なら衝突
	if (std::fabs(distanceFromPlane) <= sphere.radius) {
		return true;
	}
	return false;
}

// 線と平面の当たり判定
bool Collision3D::IsCollisionLineAndPlane(const Segment& segment, const Plane& plane) {
	// まず垂直判定を行うために、法線と線の内積を求める
	float dot = Vector3::Dot(plane.normal, segment.diff);

	// 垂直=平行であるので、衝突しているはずがない
	if (dot == 0.0f) {
		return false;
	}

	// tを求める
	float t = (plane.distance - Vector3::Dot(segment.origin, plane.normal)) / dot;

	// tの値と線の種類によって衝突しているかを判断する
	// 線分なのでtが0.0f~1.0fの間であれば衝突している
	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}

	return false;
}

// 三角形と線の当たり判定
bool Collision3D::IsCollisionTriangleAndSegment(const Triangle& triangle, const Segment& segment) {
	// 三角形の法線を求める
	Vector3 v01 = triangle.vertices[1] - triangle.vertices[0];
	Vector3 v02 = triangle.vertices[2] - triangle.vertices[0];
	Vector3 normal = Vector3::Normalize(Vector3::Cross(v01, v02));

	// 三角形が乗っている平面の方程式のdistanceを求める
	// Dot(normal, p) = distance
	float distance = Vector3::Dot(normal, triangle.vertices[0]);

	// 線分と平面の交点パラメータ t を計算する
	float dot = Vector3::Dot(normal, segment.diff);
	if (dot == 0.0f) {
		return false; // 平行な場合は当たらない
	}

	float t = (distance - Vector3::Dot(segment.origin, normal)) / dot;

	// 線分なので t が 0.0f から 1.0f の間でなければ交点を持たない
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	//  交点pの座標を計算
	Vector3 p = segment.origin + (segment.diff * t);

	// 交点が三角形の内側にあるかをクロス積で判定
	Vector3 v12 = triangle.vertices[2] - triangle.vertices[1];
	Vector3 v20 = triangle.vertices[0] - triangle.vertices[2];

	Vector3 v0p = p - triangle.vertices[0];
	Vector3 v1p = p - triangle.vertices[1];
	Vector3 v2p = p - triangle.vertices[2];

	// 各辺と交点へのベクトルのクロス積を計算
	Vector3 cross01 = Vector3::Cross(v01, v1p);
	Vector3 cross12 = Vector3::Cross(v12, v2p);
	Vector3 cross20 = Vector3::Cross(v20, v0p);

	// 全てのクロス積が、平面の法線（normal）と同じ方向を向いているかを内積でチェック
	// すべてが 0.0f以上、もしくはすべてが0.0f以下なら内側にある
	if (Vector3::Dot(cross01, normal) >= 0.0f &&
		Vector3::Dot(cross12, normal) >= 0.0f &&
		Vector3::Dot(cross20, normal) >= 0.0f
		) {
		return true;
	}

	return false;
}

// ---AABB同士の衝突判定---
bool Collision3D::IsCollisionAabbAndAabb(const AABB& a, const AABB& b) {
	// X, Y, Z軸すべてで重なりがあるか判定
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true; // 衝突している
	}
	return false; // 衝突していない
}

// AABBと球の衝突判定
bool Collision3D::IsCollisionAabbAndSphere(const AABB& aabb, const Sphere& sphere) {
	// 最近接点を求める (各軸について、球の中心座標をAABBの最小値と最大値でクランプする)
	Vector3 closestPoint{
		(std::clamp)(sphere.center.x, aabb.min.x, aabb.max.x),
		(std::clamp)(sphere.center.y, aabb.min.y, aabb.max.y),
		(std::clamp)(sphere.center.z, aabb.min.z, aabb.max.z)
	};

	// 最近接点と球の中心との距離を求める
	float distance = Vector3::Length(closestPoint - sphere.center);

	// 距離が半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		return true;
	}

	return false;
}

/// --- AABBと線分の衝突判定 ---
bool Collision3D::IsCollisionAabbAndSegment(const AABB& aabb, const Segment& segment) {
	// 1_線分上の位置を表すtの範囲を用意する
	// t=0.0fが始点、t=1.0fが終点
	float tMin = 0.0f;
	float tMax = 1.0f;

	// 2_X,Y,Zを同じ処理で判定できるように配列へまとめる
	const float origins[3] = {segment.origin.x, segment.origin.y, segment.origin.z};
	const float diffs[3] = {segment.diff.x, segment.diff.y, segment.diff.z};
	const float mins[3] = {aabb.min.x, aabb.min.y, aabb.min.z};
	const float maxs[3] = {aabb.max.x, aabb.max.y, aabb.max.z};

	// 3_X,Y,Zそれぞれの軸で、線分がAABBの範囲に入るtを調べる
	for (int axis = 0; axis < 3; ++axis) {
		// 4_線分がこの軸と平行なら、始点が範囲外の時点で当たらない
		if (diffs[axis] == 0.0f) {
			if (origins[axis] < mins[axis] || origins[axis] > maxs[axis]) {
				return false;
			}
			continue;
		}

		// 5_AABBのmin面とmax面に到達するtを求める
		float t1 = (mins[axis] - origins[axis]) / diffs[axis];
		float t2 = (maxs[axis] - origins[axis]) / diffs[axis];

		// 6_近い方を入口、遠い方を出口として扱う
		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		// 7_全ての軸で共通してAABB内にいるtの範囲を狭める
		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		// 8_共通範囲がなくなったら衝突していない
		if (tMin > tMax) {
			return false;
		}
	}

	// 9_全ての軸で共通範囲が残っていれば衝突している
	return true;
}

/// --- OBBと球の衝突判定 ---
bool Collision3D::IsCollisionObbAndSphere(const OBB& obb, const Sphere& sphere) {
	// 1_OBBのWorldMatrixとその逆行列を計算
	Matrix4x4 obbWorldMatrix = MakeOBBWorldMatrix(obb);
	Matrix4x4 obbWorldMatrixInverse = Matrix4x4::Inverse(obbWorldMatrix);

	// 2_球の中心をOBBのローカル空間へ変換
	Vector3 centerInOBBLocalSpace = Vector3::Transform(sphere.center, obbWorldMatrixInverse);

	// 3_ローカル空間でのAABBを作成
	// 中心が原点(0,0,0)なので、minは-size, maxはsizeとなる
	AABB aabbOBBLocal;
	aabbOBBLocal.min = {-obb.size.x, -obb.size.y, -obb.size.z};
	aabbOBBLocal.max = {obb.size.x, obb.size.y, obb.size.z};

	// 4_ローカル空間での球を作成
	Sphere sphereOBBLocal;
	sphereOBBLocal.center = centerInOBBLocalSpace;
	sphereOBBLocal.radius = sphere.radius;

	// 5_ローカル空間でAABBと球の判定を行う
	return IsCollisionAabbAndSphere(aabbOBBLocal, sphereOBBLocal);
}

/// --- OBBと線の衝突判定 ---
bool Collision3D::IsCollisionObbAndSegment(const OBB& obb, const Segment& segment) {
	// 1_OBBの向きと位置から、OBBのワールド行列を作る
	Matrix4x4 obbWorldMatrix = MakeOBBWorldMatrix(obb);

	// 2_逆行列を作り、ワールド空間からOBBローカル空間へ戻せるようにする
	Matrix4x4 obbInverse = Matrix4x4::Inverse(obbWorldMatrix);

	// 3_線分の始点と終点をOBBローカル空間へ変換する
	Vector3 localOrigin = Vector3::Transform(segment.origin, obbInverse);
	Vector3 localEnd = Vector3::Transform(segment.origin + segment.diff, obbInverse);

	// 4_OBBローカル空間では、OBBは原点中心のAABBとして扱える
	AABB localAabb{
		.min{-obb.size.x, -obb.size.y, -obb.size.z},
		.max{obb.size.x, obb.size.y, obb.size.z},
	};

	// 5_OBBローカル空間で判定するための線分を作り直す
	Segment localSegment{
		.origin = localOrigin,
		.diff = localEnd - localOrigin,
	};

	// 6_OBBと線分の判定を、AABBと線分の判定に置き換える
	return IsCollisionAabbAndSegment(localAabb, localSegment);
}

// --- OBBをWorld座標系へ変換する行列を作成する関数 ---
Matrix4x4 Collision3D::MakeOBBWorldMatrix(const OBB& obb) {
	Matrix4x4 matrix;
	// 3x3の回転行列成分をセット
	matrix.m[0][0] = obb.orientations[0].x; matrix.m[0][1] = obb.orientations[0].y; matrix.m[0][2] = obb.orientations[0].z; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = obb.orientations[1].x; matrix.m[1][1] = obb.orientations[1].y; matrix.m[1][2] = obb.orientations[1].z; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = obb.orientations[2].x; matrix.m[2][1] = obb.orientations[2].y; matrix.m[2][2] = obb.orientations[2].z; matrix.m[2][3] = 0.0f;
	// 平行移動成分をセット
	matrix.m[3][0] = obb.center.x; matrix.m[3][1] = obb.center.y; matrix.m[3][2] = obb.center.z; matrix.m[3][3] = 1.0f;
	return matrix;
}

// Sphereを表示する
void Collision3D::DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
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
Vector3 Collision3D::Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}

	return {0.0f, -vector.z, vector.y};
}

void Collision3D::DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
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
void Collision3D::DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
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
void Collision3D::DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
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
void Collision3D::DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	// ローカル空間での8頂点（中心が原点なので -size ～ +size）
	Vector3 vertices[8] = {
		{-obb.size.x, -obb.size.y, -obb.size.z}, {obb.size.x, -obb.size.y, -obb.size.z},
		{obb.size.x, -obb.size.y, obb.size.z}, {-obb.size.x, -obb.size.y, obb.size.z},
		{-obb.size.x, obb.size.y, -obb.size.z}, {obb.size.x, obb.size.y, -obb.size.z},
		{obb.size.x, obb.size.y, obb.size.z}, {-obb.size.x, obb.size.y, obb.size.z}
	};

	// WorldMatrixを取得
	Matrix4x4 obbWorldMatrix = MakeOBBWorldMatrix(obb);

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