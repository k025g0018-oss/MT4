#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"
#include "Geometry3D.h"

class Collision3D {
public:
	// 正射影ベクトルと最近接点
	static Vector3 ClosestPoint(const Vector3& point, const Segment& segment);

	// 球と球の当たり判定
	static bool IsCollisionSphereAndSphere(const Sphere& s1, const Sphere& s2);

	// 球と平面の当たり判定
	static bool IsCollisionSphereAndPlane(const Sphere& sphere, const Plane& plane);

	// 線と平面の当たり判定
	static bool IsCollisionLineAndPlane(const Segment& segment, const Plane& plane);

	// 三角形と線の当たり判定
	static bool IsCollisionTriangleAndSegment(const Triangle& triangle, const Segment& segment);

	// ==========================================

	/// --- AABB ---
	// AABB同士の衝突判定
	static bool IsCollisionAabbAndAabb(const AABB& a, const AABB& b);

	// AABBと球の衝突判定
	static bool IsCollisionAabbAndSphere(const AABB& aabb, const Sphere& sphere);

	// AABBと線分の衝突判定
	static bool IsCollisionAabbAndSegment(const AABB& aabb, const Segment& segment);

	/// --- OBB ---

	// OBBと球の衝突判定
	static bool IsCollisionObbAndSphere(const OBB& obb, const Sphere& sphere);

	// OBBと線の衝突判定
	static bool IsCollisionObbAndSegment(const OBB& obb, const Segment& segment);

	// OBBとOBBの衝突判定
	static bool IsCollisionObbAndObb(const OBB& obb1, const OBB& obb2);
	
private:
	// OBBの8頂点をワールド座標で求める
	static void GetObbVertices(const OBB& obb, Vector3 vertices[8]);

	// 指定した軸で2つのOBBが分離しているか判定する
	static bool IsSeparatedOnAxis(const Vector3& axis, const Vector3 vertices1[8], const Vector3 vertices2[8]);
};