#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <cmath>
#include <imgui.h>
#include <algorithm>

// インクルードするファイル
#include "Matrix4x4.h"
#include "Vector.h"

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

/// --- 構造体 ---
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

/// --- 関数 ---
// 内積
float Dot(const Vector3& v1, const Vector3& v2) {
	float result;
	result =
		v1.x * v2.x +
		v1.y * v2.y +
		v1.z * v2.z;
	return result;
}

// 長さ(ノルム)
float Length(const Vector3& v) {
	float result;
	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return result;
}

// 正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	// 長さを計算
	float length = Length(v);

	// 0の時はエラーが出るのでそのまま0を返す分岐を作る
	if (length != 0.0f) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	} else {
		// 長さが0の場合は０を返す
		result = {0.0f, 0.0f, 0.0f};
	}

	return result;
}

// 4x4行列の数値表示
static const int kRowHeight = 30;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth, y + (row + 1) * kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}

// Vector3の数値表示
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	Novice::ScreenPrintf(x, y + kRowHeight, "%.02f, %.02f, %.02f", vector.x, vector.y, vector.z);
}

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	// a * b = {(a.y * b.z - a.z * b.y), (a.z * b.x - a.x * b.z), (a.x * b.y - a.y * b.x)}
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

// Gridを表示
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
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

// Sphereを表示する
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
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

// 正射影ベクトルと最近接点
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	float dot = Dot(v1, Normalize(v2));
	return Normalize(v2) * dot; // スカラー倍のオーバーロードを利用
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	// 始点から点へのベクトル
	Vector3 v = point - segment.origin;

	// 線分の向きベクトル（diff）との内積から、どの位置にいるか(t)を計算
	// t = (v1・v2) / |v2|^2
	float t = Dot(v, segment.diff) / powf(Length(segment.diff), 2.0f);

	// 線分なので 0.0(始点) ～ 1.0(終点) の間にクランプする
	t = max(0.0f, min(t, 1.0f));

	// 始点 + 向き * t
	return segment.origin + (segment.diff * t);
}

// 球と球の当たり判定
bool IsCollisionSphereAndSphere(const Sphere& s1, const Sphere& s2) {
	// 2つの球の中心点間の距離を求める
	float distance = Length(s2.center - s1.center);

	// 半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}

	return false;
}

// 球と平面の当たり判定
bool IsCollisionSphereAndPlane(const Sphere& sphere, const Plane& plane) {
	float distanceFromPlane = Dot(sphere.center, plane.normal) - plane.distance;

	// 距離の絶対値が半径以下なら衝突
	if (std::fabs(distanceFromPlane) <= sphere.radius) {
		return true;
	}
	return false;
}

// 平面の描画
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}

	return {0.0f, -vector.z, vector.y};
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = plane.normal * plane.distance; // 1
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal)); // 2
	perpendiculars[1] = {-perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z}; // 3
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]); // 4
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

// 線と平面の当たり判定
bool IsCollisionLineAndPlane(const Segment& segment, const Plane& plane) {
	// まず垂直判定を行うために、法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);

	// 垂直=平行であるので、衝突しているはずがない
	if (dot == 0.0f) {
		return false;
	}

	// tを求める
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;

	// tの値と線の種類によって衝突しているかを判断する
	// 線分なのでtが0.0f~1.0fの間であれば衝突している
	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}

	return false;
}

// 三角形と線の当たり判定
bool IsCollisionTriangleAndSegment(const Triangle& triangle, const Segment& segment) {
	// 三角形の法線を求める
	Vector3 v01 = triangle.vertices[1] - triangle.vertices[0];
	Vector3 v02 = triangle.vertices[2] - triangle.vertices[0];
	Vector3 normal = Normalize(Cross(v01, v02));

	// 三角形が乗っている平面の方程式のdistanceを求める
	// Dot(normal, p) = distance
	float distance = Dot(normal, triangle.vertices[0]);

	// 線分と平面の交点パラメータ t を計算する
	float dot = Dot(normal, segment.diff);
	if (dot == 0.0f) {
		return false; // 平行な場合は当たらない
	}

	float t = (distance - Dot(segment.origin, normal)) / dot;

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
	Vector3 cross01 = Cross(v01, v1p);
	Vector3 cross12 = Cross(v12, v2p);
	Vector3 cross20 = Cross(v20, v0p);

	// 全てのクロス積が、平面の法線（normal）と同じ方向を向いているかを内積でチェック
	// すべてが 0.0f以上、もしくはすべてが0.0f以下なら内側にある
	if (Dot(cross01, normal) >= 0.0f &&
		Dot(cross12, normal) >= 0.0f &&
		Dot(cross20, normal) >= 0.0f
		) {
		return true;
	}

	return false;
}

// 三角形の描画
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i) {
		screenVertices[i] = Vector3::Transform(Vector3::Transform(triangle.vertices[i], viewProjectionMatrix), viewportMatrix);
	}

	// 3つの頂点を線で結ぶ
	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[1].x, (int)screenVertices[1].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[2].x, (int)screenVertices[2].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[0].x, (int)screenVertices[0].y, color);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//////////
	/// 変数の宣言
	//////////

	// 画面
	Vector2 screenSize;
	screenSize.x = 1280.0f;
	screenSize.y = 720.0f;

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, static_cast<int>(screenSize.x), static_cast<int>(screenSize.y));

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	/// ---定義エリア---

	// 線
	Segment segment{{-2.0f, -1.0f, 0.0f}, {3.0f, 2.0f, 2.0f}};
	Vector3 baseDiff = segment.diff; // 元の方向ベクトルを保存
	float segmentScale = 1.0f; // 長さを変えるためのスケール値
	unsigned int segmentColor = 0xFFFFFFFF; // 線の描画色

	Vector3 point{-1.5f, 0.6f, 0.6f};

	// カメラの初期位置
	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.25, 0.0f, 0.0f};

	// 球
	Sphere sphere;
	sphere.center = {0.0f, 0.0f, 0.0f};
	sphere.radius = 0.6f;
	sphere.color = 0x000000FF;

	// 平面
	Plane plane;
	plane.normal = {0.0f, 1.0f, 0.0f};
	plane.distance = 1.0f;
	sphere.color = 0x000000FF;

	// 三角形
	Triangle triangle;
	triangle.vertices[0] = {0.0f, 1.0f, 0.0f};
	triangle.vertices[1] = {1.0f, -0.5f, -0.5f};
	triangle.vertices[2] = {-1.0f, -0.5f, -0.5f};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		/// ---ImGui---
		// 始め
		ImGui::Begin("Window");

		// 中身
		// カメラ
		ImGui::Text("Camera");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);

		// 三角形
		ImGui::Separator();
		ImGui::Text("Triangle Vertices");
		ImGui::DragFloat3("Triangle v0", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("Triangle v1", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("Triangle v2", &triangle.vertices[2].x, 0.01f);

		// 線
		ImGui::Separator();
		ImGui::Text("Segment");
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &baseDiff.x, 0.01f);
		ImGui::SliderFloat("Segment Scale", &segmentScale, 0.0f, 5.0f, "%.2f");
		segment.diff = baseDiff * segmentScale; // 元の方向にスケールを掛け算して現在のdiffを確定させる

		// 終わり
		ImGui::End();

		// --- 当たり判定 ---
		// 三角形と線
		if (IsCollisionTriangleAndSegment(triangle, segment)) {
			segmentColor = 0xFF0000FF;
		} else {
			segmentColor = 0xFFFFFFFF;
		}

		/// ---行列の計算---
		Matrix4x4 cameraMatrix = Matrix4x4::MakeAffineMatrix({1, 1, 1}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Matrix4x4::Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(0.45f, screenSize.x / screenSize.y, 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Matrix4x4::Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = Matrix4x4::MakeViewportMatrix(0, 0, screenSize.x, screenSize.y, 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// グリッド線
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 線
		Vector3 start = Vector3::Transform(Vector3::Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Vector3::Transform(Vector3::Transform(segment.origin + segment.diff, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), segmentColor);

		// 球
		// DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, sphere.color);

		// 平面
		// DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// 三角形
		DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}

