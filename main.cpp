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

struct Sphere {
	Vector3 center; // 中心点
	float radius; // 半径
};

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

	Segment segment{{-2.0f, -1.0f, 0.0f}, {3.0f, 2.0f, 2.0f}};
	Vector3 point{-1.5f, 0.6f, 0.6f};

	// pointを線分に射影したベクトル。今回は正しく計算で来ているかを確認するために使う
	Vector3 project = Project(point - segment.origin, segment.diff);

	// この値が線分上の点を表す
	Vector3 closestPoint = ClosestPoint(point, segment);

	// カメラの初期位置
	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.25, 0.0f, 0.0f};

	// 球
	Sphere sphere[2];
	sphere[0].center = {0.0f, 0.0f, 0.0f};
	sphere[0].radius = 0.6f;
	sphere[1].center = {0.0f, 0.0f, 1.0f};
	sphere[1].radius = 0.4f;

	// 2つの球の中心点間の距離を求める
	float distance = Length(sphere[1].center - sphere[0].center);
	// 半径の合計よりも短ければ衝突
	if (distance <= sphere[0].radius + sphere[1].radius) {

	}

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
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &segment.diff.x, 0.01f);
		ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);

		// 終わり
		ImGui::End();

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
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);

		// 点
		Sphere pointSphere{point, 0.01f}; // 1cmの弾を描画
		Sphere closestPointSphere{closestPoint, 0.01f};
		DrawSphere(pointSphere, viewProjectionMatrix, viewportMatrix, RED);
		DrawSphere(closestPointSphere, viewProjectionMatrix, viewportMatrix, BLACK);

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
