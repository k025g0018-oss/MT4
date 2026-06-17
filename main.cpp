#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <cmath>
#include <imgui.h>
#include <algorithm>

// インクルードするファイル
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Collision3D.h"

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

// 4x4行列の数値表示
static const int kRowHeight = 30;
static const int kColumnWidth = 60;

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
	Segment segment{
		.origin{-0.8f, -0.3f, 0.0f},
		.diff{0.5f, 0.5f, 0.5f},
	};

	Vector3 baseDiff = segment.diff; // 元の方向ベクトルを保存
	// float segmentScale = 1.0f; // 長さを変えるためのスケール値
	unsigned int segmentColor = 0xFFFFFFFF; // 線の描画色

	Vector3 point{-1.5f, 0.6f, 0.6f};

	// カメラの初期位置
	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.25, 0.0f, 0.0f};

	// 球
	Sphere sphere;
	sphere.center = {0.0f, 0.0f, 0.0f};
	sphere.radius = 0.6f;
	sphere.color = 0xFFFFFFFF;

	// 平面
	Plane plane;
	plane.normal = {0.0f, 1.0f, 0.0f};
	plane.distance = 1.0f;

	// 三角形
	Triangle triangle;
	triangle.vertices[0] = {0.0f, 1.0f, 0.0f};
	triangle.vertices[1] = {1.0f, -0.5f, -0.5f};
	triangle.vertices[2] = {-1.0f, -0.5f, -0.5f};

	// AABBと色の初期化
	AABB aabb = {
		.min{-0.5f, -0.5f, -0.5f},
		.max{0.5f, 0.5f, 0.5f}
	};
	//unsigned int aabbColor = 0xFFFFFFFF;

	Vector3 rotate{0.0f, 0.0f, 0.0f};

	// OBB
	OBB obb{
		.center{-1.0f, 0.0f, 0.0f},
		.orientations = {
			{1.0f, 0.0f, 0.0f, },
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	},
		.size{0.5f, 0.5f, 0.5f}
	};
	unsigned int obbColor = 0xFFFFFFFF;

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

		ImGui::DragFloat3("OBB Rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("OBB Center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("OBB Size", &obb.size.x, 0.01f);

		// 線
		ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.01f);

		// 終わり
		ImGui::End();

		// ==========================================

		// minとmaxが入れ替わらないようにする処理
		AABB tempAABB = aabb;
		aabb.min.x = (std::min)(tempAABB.min.x, tempAABB.max.x);
		aabb.max.x = (std::max)(tempAABB.min.x, tempAABB.max.x);
		aabb.min.y = (std::min)(tempAABB.min.y, tempAABB.max.y);
		aabb.max.y = (std::max)(tempAABB.min.y, tempAABB.max.y);
		aabb.min.z = (std::min)(tempAABB.min.z, tempAABB.max.z);
		aabb.max.z = (std::max)(tempAABB.min.z, tempAABB.max.z);

		// ==========================================

		/// rotate変数をOBBが対象としているオブジェクトの回転とし、これを基に回転行列を作る
		// OBBの回転行列の更新
		Matrix4x4 rotateMatrix = Matrix4x4::Multiply(Matrix4x4::MakeRotateXMatrix(rotate.x), Matrix4x4::Multiply(Matrix4x4::MakeRotateYMatrix(rotate.y), Matrix4x4::MakeRotateZMatrix(rotate.z)));

		// 回転行列から軸（Orientation）を抽出してOBBにセット
		for (int i = 0; i < 3; ++i) {
			obb.orientations[i].x = rotateMatrix.m[i][0];
			obb.orientations[i].y = rotateMatrix.m[i][1];
			obb.orientations[i].z = rotateMatrix.m[i][2];
		}

		// --- 当たり判定 ---
		// 衝突判定
		if (Collision3D::IsCollisionObbAndSegment(obb, segment)) {
			// 衝突していたら赤にする
			obbColor = 0xFF0000FF;
		} else {
			obbColor = 0xFFFFFFFF;
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
		// Collision3D::DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, sphere.color);

		// 平面
		// Collision3D::DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// 三角形
		//Collision3D::DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// AABB
		// Collision3D::DrawAABB(aabb, viewProjectionMatrix, viewportMatrix, aabbColor);

		// OBB
		Collision3D::DrawOBB(obb, viewProjectionMatrix, viewportMatrix, obbColor);

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
