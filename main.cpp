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
#include "Easing.h"
#include "Quaternion.h"

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

/// --- 円の構造体 ---

struct Circle {
	Vector2 pos; // 中心座標
	float radius; // 半径
	unsigned int color; // 色
};

/// --- 追従補間 ---

// 2点間の距離
float Distance(const Vector2& a, const Vector2& b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	return std::sqrt(dx * dx + dy * dy);
}

// 残り距離が閾値未満なら目標位置にそろえ、追従完了とする
bool SnapIfArrived(Vector2& pos, const Vector2& target, float threshold = 0.01f) {
	if (Distance(pos, target) < threshold) {
		pos = target;
		return true;
	}
	return false;
}

// 線形補間による追従 pos += (speed * deltaTime) * (target - pos)
bool FollowLinear(Vector2& pos, const Vector2& target, float speed, float deltaTime) {
	// 割合 t は speed * deltaTime
	float t = speed * deltaTime;
	pos = Easing::Lerp(pos, target, t);

	return SnapIfArrived(pos, target);
}

// 指数補間による追従
bool FollowExponential(Vector2& pos, const Vector2& target, float speed, float deltaTime) {
	// 割合 t の求め方が違うだけで、あとは同じ
	float t = 1.0f - std::exp(-speed * deltaTime);
	pos = Easing::Lerp(pos, target, t);

	return SnapIfArrived(pos, target);
}

// 円を描画
void DrawCircle(const Circle& circle, FillMode fillMode = kFillModeSolid) {
	Novice::DrawEllipse(
		static_cast<int>(circle.pos.x), static_cast<int>(circle.pos.y),
		static_cast<int>(circle.radius), static_cast<int>(circle.radius),
		0.0f, circle.color, fillMode
	);
}

/// --- Quaternion ---

// Quaternion を表示する
void QuaternionScreenPrintf(int x, int y, const Quaternion& q, const char* label) {

	Novice::ScreenPrintf(x, y, "%.02f", q.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", q.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", q.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%.02f", q.w);
	Novice::ScreenPrintf(x + kColumnWidth * 4, y, "%s", label);
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

	/// --- 追従補間 ---

	// 60fps前提で固定
	const float kDeltaTime = 1.0f / 60.0f;

	// 画面中央
	const Vector2 kCenter{640.0f, 360.0f};

	// 円a:マウス位置
	Circle circleA{
		.pos{0.0f, 0.0f},
		.radius = 12.0f,
		.color = 0xFF3333FF
	};

	// 円b:線形補間で追従
	Circle circleB{
		.pos = kCenter,
		.radius = 20.0f,
		.color = 0x33CC33FF
	};

	// 比較用:指数補間で追従
	Circle circleC{
		.pos = kCenter,
		.radius = 20.0f,
		.color = 0xFFDD33FF
	};

	// 追従速度
	float speed = 3.0f;

	// 指数補間の比較表示
	bool showExponential = false;

	// 追従完了フラグ
	bool isArrivedB = false;

	/// --- Quaternion ---

	Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
	Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };
	Quaternion identity = Identity();
	Quaternion conj = Conjugate(q1);
	Quaternion inv = Inverse(q1);
	Quaternion normal = Normalize(q1);
	Quaternion mul1 = Multiply(q1, q2);
	Quaternion mul2 = Multiply(q2, q1);
	float norm = Norm(q1);

	///// ----- ===== ----- /////

	///// ----- while (ループ処理) ----- /////

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

		///// ----- ===== ----- /////

		///// ----- 球面座標 ----- /////

		/// --- 入力値の制限 ---

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

		///// ----- ===== ----- /////

		///// ----- 円 ----- /////

		/// --- 円aをマウス位置へ ---

		int mouseX = 0;
		int mouseY = 0;
		Novice::GetMousePosition(&mouseX, &mouseY);
		circleA.pos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};

		/// --- 線形補間で円bを追従 ---

		isArrivedB = FollowLinear(circleB.pos, circleA.pos, speed, kDeltaTime);

		/// --- 指数補間で円cを追従(比較用) ---

		FollowExponential(circleC.pos, circleA.pos, speed, kDeltaTime);

		///// ----- ===== ----- /////

		///// ----- ImGui ----- /////

		/// --- カメラ ---

		ImGui::Begin("Window");

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

		ImGui::End();

		/// --- 球面座標 ---

		ImGui::Begin("Spherical Coordinates");

		ImGui::Text("Target: (0, 0, 0) / +Y up / Camera +Z forward");
		ImGui::Separator();

		// 編集
		ImGui::InputFloat("Radius", &s.radius, 0.1f, 1.0f, "%.3f");
		ImGui::InputFloat("Theta: elevation (rad)", &s.theta, 0.05f, 0.5f, "%.3f");
		ImGui::InputFloat("Phi (rad)", &s.phi, 0.05f, 0.5f, "%.3f");

		// 結果の表示
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

		/// --- 追従補間 ---

		ImGui::Begin("Interpolation Controller");

		ImGui::Text("Target: Mouse Position (Red Circle)");
		ImGui::Separator();

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "[Linear Interpolation - Student Mode]");
		ImGui::SliderFloat("Speed", &speed, 0.1f, 30.0f, "%.1f");

		ImGui::Text("Mouse Pos: (%.1f, %.1f)", circleA.pos.x, circleA.pos.y);
		ImGui::Text("Circle Pos: (%.1f, %.1f)", circleB.pos.x, circleB.pos.y);
		ImGui::Text("Distance to target: %.1f px", Distance(circleB.pos, circleA.pos));
		ImGui::Text("Arrived: %s", isArrivedB ? "true" : "false");

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.1f, 1.0f, 1.0f), "[Teacher Comparison Option]");
		ImGui::Checkbox("Show Exponential Decay (Compare)", &showExponential);

		// 目標位置へ即座に合わせる
		if (ImGui::Button("Snap to Target")) {
			circleB.pos = circleA.pos;
			circleC.pos = circleA.pos;
		}
		ImGui::SameLine();
		// 画面中央へ戻す
		if (ImGui::Button("Reset to Center")) {
			circleB.pos = kCenter;
			circleC.pos = kCenter;
		}

		ImGui::End();

		///// ----- ===== ----- /////

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

		///// ----- ===== ----- /////

		///// ----- 追従補間 ----- /////

		// 円aと円bを結ぶ線 (追従の遅れを可視化)
		Novice::DrawLine(
			static_cast<int>(circleB.pos.x), static_cast<int>(circleB.pos.y),
			static_cast<int>(circleA.pos.x), static_cast<int>(circleA.pos.y),
			0xFFFFFFAA
		);

		// 比較用の円c
		if (showExponential) {
			DrawCircle(circleC, kFillModeWireFrame);
		}

		// 追従する円b
		DrawCircle(circleB);

		// マウス位置の円a
		DrawCircle(circleA);

		///// ----- ===== ----- /////

		///// ----- 描画 ----- /////

		QuaternionScreenPrintf(0, kRowHeight * 0, identity, " : Identity");
		QuaternionScreenPrintf(0, kRowHeight * 1, conj, " : Conjugate");
		QuaternionScreenPrintf(0, kRowHeight * 2, inv, " : Inverse");
		QuaternionScreenPrintf(0, kRowHeight * 3, normal, " : Normalize");
		QuaternionScreenPrintf(0, kRowHeight * 4, mul1, " : Multiply(q1, q2)");
		QuaternionScreenPrintf(0, kRowHeight * 5, mul2, " : Multiply(q2, q1)");
		Novice::ScreenPrintf(0, kRowHeight * 6, "%.02f : Norm", norm);

		///// ----- ===== ----- /////

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

	///// ----- ===== ----- /////

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
