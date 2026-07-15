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
#include "Spring.h"

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

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
	sphere.radius = 0.1f;
	sphere.color = 0xFFFFFFFF;

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

	// 円錐振り子
	ConicalPendulum conicalPendulum;
	conicalPendulum.anchor = {0.0f, 1.0f, 0.0f};
	conicalPendulum.length = 0.8f;
	conicalPendulum.halfApexAngle = 0.7f;
	conicalPendulum.angle = 0.0f;
	conicalPendulum.angularVelocity = 0.0f;

	// 円錐振り子の球
	Sphere conicalPendulumBall;
	conicalPendulumBall.center = {0.0f, 0.0f, 0.0f};
	conicalPendulumBall.radius = 0.08f;
	conicalPendulumBall.color = 0xFFFFFFFF;

	// 円錐振り子の紐
	Segment conicalPendulumString;
	conicalPendulumString.origin = conicalPendulum.anchor;
	conicalPendulumString.diff = {0.0f, -conicalPendulum.length, 0.0f};

	// 60FPSを前提とした1フレームの経過時間
	const float deltaTime = 1.0f / 60.0f;

	// Startボタンを押すまでは計算しない
	bool isRunning = false;

	/// --- ボール ---
	Ball ball{
		.position = {0.8f, 1.2f, 0.3f},
		.velocity = {0.0f,0.0f,0.0f},
		.acceleration = {0.0f,-9.8f,0.0f},
		.mass = 2.0f,
		.radius = 0.05f,
		.color = 0xFFFFFFFF,
	};

	// 法線方向の速度にだけ適用する反発係数
	// 1.0fに近いほど強く跳ね、0.0fに近いほど跳ねなくなる
	const float coefficientOfRestitution = 0.8f;

	/// --- 平面 ---
	Plane plane{
		.normal = Vector3::Normalize({-0.2f,0.9f,-0.3f}),
		.distance = 0.0f,
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

		// 区切り線
		ImGui::Separator();

		/// --- 球操作 ---

		// Startを押したら初動かす
		if (ImGui::Button("Start")) {
			// ボールを課題指定の初期位置へ戻す
			ball.position = {0.8f, 1.2f, 0.3f};

			// 前回の速度が残らないように静止状態へ戻す
			ball.velocity = {0.0f, 0.0f, 0.0f};

			// 重力による物理計算を開始する
			isRunning = true;
		}

		ImGui::Separator();

		// 終わり
		ImGui::End();

		///// ----- 処理 ----- /////
		// ==========

		/// --- ボール ---
		// スタートが押されたら
		if (isRunning) {
			// 移動前の位置を保存する
			// この位置から移動後までをカプセルとして判定する
			Vector3 previousPosition = ball.position;

			// 重力加速度によって速度を更新する
			ball.velocity =
				ball.velocity +
				ball.acceleration * deltaTime;

			// 速度によって移動後の予定位置を求める
			Vector3 nextPosition =
				ball.position +
				ball.velocity * deltaTime;

			// 移動前から移動後までをカプセルにする
			// ボールが1フレームで平面を通り抜けても検出できる
			Capsule capsule;
			capsule.segment.origin = previousPosition;
			capsule.segment.diff =
				nextPosition - previousPosition;
			capsule.radius = ball.radius;

			// ボールを予定位置へ移動する
			ball.position = nextPosition;

			// 移動経路を含めて平面との衝突を判定する
			if (Collision3D::IsCollisionCapsuleAndPlane(
				capsule,
				plane
				)) {
				// 更新後のボール中心から平面までの距離を求める
				float distanceFromPlane =
					Vector3::Dot(
						ball.position,
						plane.normal
					) -
					plane.distance;

				// ボールが平面へ埋まっている場合
				if (distanceFromPlane < ball.radius) {
					// 平面に埋まっている深さを求める
					float penetrationDepth =
						ball.radius -
						distanceFromPlane;

					// 埋まった分だけ平面の法線方向へ押し戻す
					ball.position =
						ball.position +
						plane.normal *
						penetrationDepth;

					// 平面の内側へ向かっている場合だけ反射させる
					float velocityDotNormal =
						Vector3::Dot(
							ball.velocity,
							plane.normal
						);

					if (velocityDotNormal < 0.0f) {
						// 元から書かれていた処理と同じように、
						// 反射ベクトルを求める
						Vector3 reflected =
							Vector3::Reflect(
								ball.velocity,
								plane.normal
							);

						// 反射速度の法線方向成分を取り出す
						Vector3 projectToNormal =
							Vector3::Project(
								reflected,
								plane.normal
							);

						// 反射速度の接線方向成分を取り出す
						Vector3 movingDirection =
							reflected -
							projectToNormal;

						// 法線方向だけに反発係数を適用する
						// 接線方向を残すことで斜面を転がり落ちる
						ball.velocity =
							projectToNormal *
							coefficientOfRestitution +
							movingDirection;
					}
				}
			}
		}

		
		

		// ==========

		///// ----- 当たり判定 ----- /////


		///// ----- 行列更新 ----- /////
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

		/// --- 平面 ---
		Draw3D::DrawPlane(
			plane,
			viewProjectionMatrix,
			viewportMatrix,
			0xFFFFFFFF
		);

		/// --- ボールを描画 ---
		// BallとSphereでは位置のメンバー名が違うため、描画用のSphereへ変換する
		Sphere drawBall{
			.center = ball.position,
			.radius = ball.radius,
			.color = ball.color,
		};

		Draw3D::DrawSphere(
			drawBall,
			viewProjectionMatrix,
			viewportMatrix,
			drawBall.color
		);

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
