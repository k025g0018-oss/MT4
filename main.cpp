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
#include "Geometry3D.h"
#include "Draw3D.h"

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

// 4x4行列の数値表示
static const int kRowHeight = 30;
static const int kColumnWidth = 60;

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

	/// --- 階層構造を構築する ---
	// [0]:肩 Ws = Ls
	// [1]:肘 We = Le * Ws
	// [2]:手 Wh = Lh * We
	Vector3 translates[3] = {
		{0.2f, 1.0f, 0.0f},
		{0.4f, 0.0f, 0.0f},
		{0.3f, 0.0f, 0.0f},
	};

	Vector3 rotates[3] = {
		{0.0f, 0.0f, -6.8f},
		{0.0f, 0.0f, -1.4f},
		{0.0f, 0.0f, 0.0f},
	};

	Vector3 scales[3] = {
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
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

		// 2次ベジェ曲線
		if (ImGui::TreeNode("Bezier Control Points")) {
		ImGui::DragFloat3("Control Point 0", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("Control Point 1", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("Control Point 2", &controlPoints[2].x, 0.01f);
		ImGui::TreePop();
		}
		*/
		// 区切り線
		ImGui::Separator();

		// 腕の各関節のローカル変換を操作
		if (ImGui::TreeNode("Arm Hierarchy")) {
			const char* jointNames[3] = {"Shoulder", "Elbow", "Hand"};

			for (int i = 0; i < 3; ++i) {
				// 関節ごとに同じ項目名を使えるようIDを分ける
				ImGui::PushID(i);

				if (ImGui::TreeNode(jointNames[i])) {
					ImGui::DragFloat3("Translate", &translates[i].x, 0.01f);
					ImGui::DragFloat3("Rotate", &rotates[i].x, 0.01f);
					ImGui::DragFloat3("Scale", &scales[i].x, 0.01f);
					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		// 終わり
		ImGui::End();

		/// --- 処理 ---
		// ==========
		// 各関節のSRTから、親座標系を基準としたローカル行列を作る
		Matrix4x4 localMatrices[3];
		for (int i = 0; i < 3; ++i) {
			localMatrices[i] = Matrix4x4::MakeAffineMatrix(
				scales[i], rotates[i], translates[i]
			);
		}

		// 肩には親がいないため、ローカル行列がそのままワールド行列になる
		Matrix4x4 worldMatrices[3];
		worldMatrices[0] = localMatrices[0];

		// 肘のローカル行列に肩のワールド行列を掛ける
		worldMatrices[1] = Matrix4x4::Multiply(
			localMatrices[1], worldMatrices[0]
		);

		// 手のローカル行列に肘のワールド行列を掛ける
		worldMatrices[2] = Matrix4x4::Multiply(
			localMatrices[2], worldMatrices[1]
		);

		// ワールド行列の4行目から各関節のワールド座標を取り出す
		Vector3 jointPositions[3];
		for (int i = 0; i < 3; ++i) {
			jointPositions[i] = {
				worldMatrices[i].m[3][0],
				worldMatrices[i].m[3][1],
				worldMatrices[i].m[3][2],
			};
		}
		// ==========

		/// --- 当たり判定 ---


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

		/// --- グリッド線 ---
		Draw3D::DrawGrid(viewProjectionMatrix, viewportMatrix);

		/// --- 線 ---
		// Vector3 start = Vector3::Transform(Vector3::Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		// Vector3 end = Vector3::Transform(Vector3::Transform(segment.origin + segment.diff, viewProjectionMatrix), viewportMatrix);
		// Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), segmentColor);

		/// --- 球 ---
		// Draw3D::DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, sphere.color);

		/// --- 平面 ---
		// Draw3D::DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		/// --- 三角形 ---
		//Draw3D::DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		/// --- AABB ---
		// Draw3D::DrawAABB(aabb, viewProjectionMatrix, viewportMatrix, aabbColor);

		/// --- OBB ---
		// Draw3D::DrawOBB(obb1, viewProjectionMatrix, viewportMatrix, obb1Color);
		// Draw3D::DrawOBB(obb2, viewProjectionMatrix, viewportMatrix, obb2Color);

		/// --- 2次ベジェ曲線 ---
		//Draw3D::DrawBezier(
		//	controlPoints[0],
		//	controlPoints[1],
		//	controlPoints[2],
		//	viewProjectionMatrix,
		//	viewportMatrix,
		//	0xFF00FFFF
		//);

		/// --- 制御点を半径0.01mの黒い球で描画 --- 
		//for (int i = 0; i < 3; ++i) {
		//	Sphere controlPointSphere{
		//		controlPoints[i],
		//		0.01f,
		//		0x000000FF
		//	};

		//	//Collision3D::DrawSphere(
		//	//	controlPointSphere,
		//	//	viewProjectionMatrix,
		//	//	viewportMatrix,
		//	//	controlPointSphere.color
		//	//);

		//	// 制御点をスクリーン座標へ変換
		//	Vector3 screenPoint = Vector3::Transform(
		//		Vector3::Transform(controlPoints[i], viewProjectionMatrix),
		//		viewportMatrix
		//	);

		//	// 制御点を小さな黒い円で描画
		//	Novice::DrawEllipse(
		//		static_cast<int>(screenPoint.x),
		//		static_cast<int>(screenPoint.y),
		//		5,
		//		5,
		//		0.0f,
		//		0x000000FF,
		//		kFillModeSolid
		//	);
		//}

		/// --- 階層構造 ---
		Segment armSegments[2] = {
			{jointPositions[0], jointPositions[1] - jointPositions[0]},
			{jointPositions[1], jointPositions[2] - jointPositions[1]},
		};

		// 肩から肘、肘から手へ白い線を描画
		for (int i = 0; i < 2; ++i) {
			Draw3D::DrawSegment(
				armSegments[i],
				viewProjectionMatrix,
				viewportMatrix,
				0xFFFFFFFF
			);
		}

		// 肩は赤、肘は緑、手は青で表示
		const uint32_t jointColors[3] = {
			0xFF0000FF,
			0x00FF00FF,
			0x0000FFFF,
		};

		for (int i = 0; i < 3; ++i) {
			Sphere jointSphere{
				jointPositions[i],
				0.08f,
				jointColors[i],
			};

			Draw3D::DrawSphere(
				jointSphere,
				viewProjectionMatrix,
				viewportMatrix,
				jointSphere.color
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
