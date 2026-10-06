#define _USE_MATH_DEFINES
#include <Novice.h>
#include <assert.h>
#include <cmath>
#include <imgui.h>
#include <algorithm>
#include <cfloat>
#include <numbers>

// インクルードするファイル
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Collision3D.h"
#include "Geometry3D.h"
#include "Draw3D.h"
#include "Spring.h"

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

///// ----- 関数定義エリア ----- /////

// 4x4行列の数値表示
static const int kRowHeight = 30;
static const int kColumnWidth = 60;

// 行列をImGuiに表示する関数
void DisplayMatrix(
	const char* label,
	const Matrix4x4& matrix
) {
	// ラベルと4行4列の要素をまとめて表示する
	ImGui::Text(
		"%s:\n"
		"%f, %f, %f, %f\n"
		"%f, %f, %f, %f\n"
		"%f, %f, %f, %f\n"
		"%f, %f, %f, %f",
		label,

		// 1行目
		matrix.m[0][0],
		matrix.m[0][1],
		matrix.m[0][2],
		matrix.m[0][3],

		// 2行目
		matrix.m[1][0],
		matrix.m[1][1],
		matrix.m[1][2],
		matrix.m[1][3],

		// 3行目
		matrix.m[2][0],
		matrix.m[2][1],
		matrix.m[2][2],
		matrix.m[2][3],

		// 4行目
		matrix.m[3][0],
		matrix.m[3][1],
		matrix.m[3][2],
		matrix.m[3][3]
	);
}

///// ----- ===== -----

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

	///// ----- 定義エリア ----- /////

	/// --- 線 ---

	Segment segment{
		.origin{-0.8f, -0.3f, 0.0f},
		.diff{0.5f, 0.5f, 0.5f},
	};

	Vector3 baseDiff = segment.diff; // 元の方向ベクトルを保存
	// float segmentScale = 1.0f; // 長さを変えるためのスケール値
	//unsigned int segmentColor = 0xFFFFFFFF; // 線の描画色

	Vector3 point{-1.5f, 0.6f, 0.6f};

	/// --- カメラの初期位置 ---

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.25, 0.0f, 0.0f};

	/// --- 球 ---

	Sphere sphere;
	sphere.center = {0.0f, 0.0f, 0.0f};
	sphere.radius = 0.1f;
	sphere.color = 0xFFFFFFFF;

	/// --- 三角形 ---

	Triangle triangle;
	triangle.vertices[0] = {0.0f, 1.0f, 0.0f};
	triangle.vertices[1] = {1.0f, -0.5f, -0.5f};
	triangle.vertices[2] = {-1.0f, -0.5f, -0.5f};

	/// --- AABBと色の初期化 ---

	AABB aabb = {
		.min{-0.5f, -0.5f, -0.5f},
		.max{0.5f, 0.5f, 0.5f}
	};
	//unsigned int aabbColor = 0xFFFFFFFF;

	Vector3 rotate1{0.0f, 0.0f, 0.0f};
	Vector3 rotate2{-0.05f, -2.49f, 0.15f};

	/// --- OBB ---

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

	/// --- 2次ベジェ曲線の制御点 ---

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

	/// --- ベクトルの演算 ---

	// 演算に使用する2つのベクトル
	Vector3 a{0.2f, 1.0f, 0.0f};
	Vector3 b{2.4f, 3.1f, 1.2f};

	// ベクトルの加算
	Vector3 c = a + b;

	// ベクトルの減算
	Vector3 d = a - b;

	// ベクトルを右側からスカラー倍する
	Vector3 e = a * 2.4f;

	// ベクトルを左側からスカラー倍する
	Vector3 f = 2.4f * a;

	// ベクトルの符号を反転する
	Vector3 minusA = -a;

	// ベクトルの符号をそのままにする
	Vector3 plusA = +a;

	/// --- 球面座標系 ---

	// 球面座標系から直交座標(カメラ位置)を計算
	const float halfPi = std::numbers::pi_v<float> / 2.0f;
	Spherical s{6.0f, 0.0f, -halfPi};

	// 注視点 (原点)
	const Vector3 target{0.0f, 0.0f, 0.0f};

	// 注視点を向く3本の軸を作る
	Vector3 worldUp{0.0f, 1.0f, 0.0f};
	// Vector3 forward = Vector3::Normalize(target, eye);
	// Vector3 right = Vector3::Normalize(Vector3::Cross(worldUp, forward));
	// Vector3 up = Vector3::Cross(forward, right);

	/*
	// カメラ行列を作成
	Matrix4x4 cameraMatrix{
		{
			{right.x, right.y, right.z, 0.0f},
		{up.x, up.y, up.z, 0.0f},
		{forward.x, forward.y, forward.z, 0.0f},
		{eye.x, eye.y, eye.z, 1.0f}
		}
	};
	*/

	// カメラ操作では、中心と真上・真下を避ける
	const float limit = halfPi - 0.01f;

	// Vector3 pos = Vector3::ToCartesian(s); // -> (0, 0, -6)

	// 注視点 (原点) への向きからカメラ行列を作成して表示
	// 初期値 : 前 (0, 0, 1)、右 (1, 0, 0)、上 (0, 1, 0)

	///// ----- ===== -----

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

		///// ----- Mouse Camera Control ----- /////

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

		///// ----- ===== -----

		///// ----- ImGui ----- /////

		// 始め
		ImGui::Begin("Window");

		// 中身
		
		/// --- カメラ ---

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

		// 区切り線
		ImGui::Separator();

		// 終わり
		ImGui::End();

		/// --- 球面座標の編集 ---

		ImGui::Begin("Spherical Coordinates");

		ImGui::Text("Target: (0, 0, 0) / +Y up / Camera +Z forward");
		ImGui::Separator();

		ImGui::InputFloat("Radius", &s.radius, 0.1f, 1.0f, "%.3f");
		ImGui::InputFloat("Theta: elevation (rad)", &s.theta, 0.05f, 0.5f, "%.3f");
		ImGui::InputFloat("Phi (rad)", &s.phi, 0.05f, 0.5f, "%.3f");

		// 半径0と真上・真下を避ける
		s.radius = (std::max)(s.radius, 0.1f);
		s.theta = (std::clamp)(s.theta, -limit, limit);

		/// --- カメラ位置を計算 ---

		// 注視点からのずれを位置に足す
		Vector3 eye = target + ToCartesian(s);

		/// --- 注視点を向く3本の軸を作る ---

		// 前 F:カメラから注視点への向き
		Vector3 forward = Vector3::Normalize(target - eye);
		// 右 R:世界の上と前の外積
		Vector3 right = Vector3::Normalize(Vector3::Cross(worldUp, forward));
		// 上 U:前と右の外積
		Vector3 up = Vector3::Cross(forward, right);

		/// --- カメラ行列(ワールド行列)を作成 ---

		// 1〜3行目に右・上・前、4行目に位置を格納する
		Matrix4x4 sphericalCameraMatrix{
			{
				{right.x, right.y, right.z, 0.0f},
			{up.x, up.y, up.z, 0.0f},
			{forward.x, forward.y, forward.z, 0.0f},
			{eye.x, eye.y, eye.z, 1.0f},
			}
		};

		/// --- 結果の表示 ---

		ImGui::Separator();
		ImGui::Text("Spherical: r = %.3f, theta = %.3f rad, phi = %.3f rad", s.radius, s.theta, s.phi);
		ImGui::Text("Cartesian: x = %.3f, y = %.3f, z = %.3f", eye.x, eye.y, eye.z);

		ImGui::Separator();
		ImGui::Text("Camera matrix");
		for (int row = 0; row < 4; ++row) {
			ImGui::Text(
				"%8.3f  %8.3f  %8.3f  %8.3f",
				sphericalCameraMatrix.m[row][0],
				sphericalCameraMatrix.m[row][1],
				sphericalCameraMatrix.m[row][2],
				sphericalCameraMatrix.m[row][3]
			);
		}

		ImGui::End();

		///// ----- ===== -----

		///// ----- 処理 ----- /////




		///// ----- ===== -----

		///// ----- 当たり判定 ----- /////

		///// ----- ===== -----

		///// ----- 行列更新 ----- /////

		Matrix4x4 cameraMatrix = Matrix4x4::MakeAffineMatrix({1, 1, 1}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Matrix4x4::Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(0.45f, screenSize.x / screenSize.y, 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Matrix4x4::Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = Matrix4x4::MakeViewportMatrix(0, 0, screenSize.x, screenSize.y, 0.0f, 1.0f);

		///// ----- ===== -----

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		///// ----- グリッド線 ----- /////

		Draw3D::DrawGrid(viewProjectionMatrix, viewportMatrix);

		///// ----- ===== -----

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
