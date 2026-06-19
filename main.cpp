#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <cmath>
#include <imgui.h>
#include <algorithm>
#include <cfloat>

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

// 3次元空間に2次ベジェ曲線を描画
void DrawBezier(
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
	//unsigned int segmentColor = 0xFFFFFFFF; // 線の描画色

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

	Vector3 rotate1{0.0f, 0.0f, 0.0f};
	Vector3 rotate2{-0.05f, -2.49f, 0.15f};

	// OBB
	OBB obb1{
		.center{0.0f, 0.0f, 0.0f},
		.orientations = {
			{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	},
		.size{0.83f, 0.26f, 0.24f}
	};
	// unsigned int obb1Color = 0xFFFFFFFF;

	OBB obb2{
		.center{0.9f, 0.66f, 0.78f},
		.orientations = {
			{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	},
		.size{0.5f, 0.37f, 0.5f}
	};
	// unsigned int obb2Color = 0xFFFFFFFF;

	// 2次ベジェ曲線の制御点
	Vector3 controlPoints[3] = {
		{-0.8f, 0.58f, 1.0f},
		{1.76f, 1.0f, -0.3f},
		{0.94f, -0.7f, 2.3f},
	};

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

		/// --- Mouse Camera Control ---
		// 1_ImGuiを操作している時は、マウス操作でカメラが動かないようにする
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse) {
			const float kCameraRotateSpeed = 0.005f;
			const float kCameraMoveSpeed = 0.01f;
			const float kCameraZoomSpeed = 0.3f;

			// 2_左ドラッグでカメラを回転させる
			// 横移動はY軸回転、縦移動はX軸回転として扱う
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

				cameraRotate.y += drag.x * kCameraRotateSpeed;
				cameraRotate.x += drag.y * kCameraRotateSpeed;

				// 3_真上や真下を向きすぎると操作が反転しやすいので、X回転を制限する
				cameraRotate.x = (std::clamp)(cameraRotate.x, -1.45f, 1.45f);

				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			}

			// 4_右ドラッグでカメラを上下左右に平行移動させる
			// 視点の向きは変えず、見たい位置をずらすための操作
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
				ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

				cameraTranslate.x -= drag.x * kCameraMoveSpeed;
				cameraTranslate.y += drag.y * kCameraMoveSpeed;

				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
			}

			// 5_マウスホイールでカメラを前後に移動させる
			// 距離調整をすぐ行えるようにする
			if (io.MouseWheel != 0.0f) {
				cameraTranslate.z += io.MouseWheel * kCameraZoomSpeed;
			}
		}

		/// --- ImGui ---
		// 始め
		ImGui::Begin("Window");

		// 中身
		// カメラ
		if (ImGui::TreeNode("Camera")) {
			ImGui::DragFloat3("Translate", &cameraTranslate.x, 0.01f);
			ImGui::DragFloat3("Rotate", &cameraRotate.x, 0.01f);

			// 区切り線
			ImGui::Separator();

			// マウスのカメラ操作
			ImGui::Text("Left Drag  : Rotate Camera");
			ImGui::Text("Right Drag : Move Camera");
			ImGui::Text("MouseWheel : Move Forward / Back");

			ImGui::TreePop();
		}

		/*
		// 区切り線
		ImGui::Separator();

		// OBB
		if (ImGui::TreeNode("OBB1")) {
			ImGui::DragFloat3("Rotate", &rotate1.x, 0.01f);
			ImGui::DragFloat3("Center", &obb1.center.x, 0.01f);
			ImGui::DragFloat3("Size", &obb1.size.x, 0.01f, 0.0f, FLT_MAX);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("OBB2")) {
			ImGui::DragFloat3("Rotate", &rotate2.x, 0.01f);
			ImGui::DragFloat3("Center", &obb2.center.x, 0.01f);
			ImGui::DragFloat3("Size", &obb2.size.x, 0.01f, 0.0f, FLT_MAX);
			ImGui::TreePop();
		}

		// 区切り線
		ImGui::Separator();

		// 線
		if (ImGui::TreeNode("Segment")) {
			ImGui::DragFloat3("Origin", &segment.origin.x, 0.01f);
			ImGui::DragFloat3("Diff", &segment.diff.x, 0.01f);
			ImGui::TreePop();
		}
		*/
		// 区切り線
		ImGui::Separator();
		
		// 2次ベジェ曲線
		if (ImGui::TreeNode("Bezier Control Points")) {
			ImGui::DragFloat3("Control Point 0", &controlPoints[0].x, 0.01f);
			ImGui::DragFloat3("Control Point 1", &controlPoints[1].x, 0.01f);
			ImGui::DragFloat3("Control Point 2", &controlPoints[2].x, 0.01f);
			ImGui::TreePop();
		}

		// 終わり
		ImGui::End();

		// --- 当たり判定 ---


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
		// Vector3 start = Vector3::Transform(Vector3::Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		// Vector3 end = Vector3::Transform(Vector3::Transform(segment.origin + segment.diff, viewProjectionMatrix), viewportMatrix);
		// Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), segmentColor);

		// 球
		// Collision3D::DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, sphere.color);

		// 平面
		// Collision3D::DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// 三角形
		//Collision3D::DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// AABB
		// Collision3D::DrawAABB(aabb, viewProjectionMatrix, viewportMatrix, aabbColor);

		// OBB
		// Collision3D::DrawOBB(obb1, viewProjectionMatrix, viewportMatrix, obb1Color);
		// Collision3D::DrawOBB(obb2, viewProjectionMatrix, viewportMatrix, obb2Color);

		// 2次ベジェ曲線
		DrawBezier(
			controlPoints[0],
			controlPoints[1],
			controlPoints[2],
			viewProjectionMatrix,
			viewportMatrix,
			0xFF00FFFF
		);

		// 制御点を半径0.01mの黒い球で描画
		for (int i = 0; i < 3; ++i) {
			Sphere controlPointSphere{
				controlPoints[i],
				0.01f,
				0x000000FF
			};

			//Collision3D::DrawSphere(
			//	controlPointSphere,
			//	viewProjectionMatrix,
			//	viewportMatrix,
			//	controlPointSphere.color
			//);

			// 制御点をスクリーン座標へ変換
			Vector3 screenPoint = Vector3::Transform(
				Vector3::Transform(controlPoints[i], viewProjectionMatrix),
				viewportMatrix
			);

			// 制御点を小さな黒い円で描画
			Novice::DrawEllipse(
				static_cast<int>(screenPoint.x),
				static_cast<int>(screenPoint.y),
				5,
				5,
				0.0f,
				0x000000FF,
				kFillModeSolid
			);
		}

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
