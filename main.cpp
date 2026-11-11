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

// Vector3 を1行で表示する
void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", v.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", v.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", v.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

///// ----- ===== ----- /////

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

	/// --- 補間する 2 つの回転 ---

	Quaternion rotation0 = MakeRotateAxisAngleQuaternion({0.71f, 0.71f, 0.0f}, 0.3f);
	Quaternion rotation1 = MakeRotateAxisAngleQuaternion({0.71f, 0.0f, 0.71f}, 3.141592f);

	/// --- 球面線形補間 ---

	Quaternion interpolate0 = Slerp(rotation0, rotation1, 0.0f);
	Quaternion interpolate1 = Slerp(rotation0, rotation1, 0.3f);
	Quaternion interpolate2 = Slerp(rotation0, rotation1, 0.5f);
	Quaternion interpolate3 = Slerp(rotation0, rotation1, 0.7f);
	Quaternion interpolate4 = Slerp(rotation0, rotation1, 1.0f);

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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		///// ----- 描画 ----- /////

		QuaternionScreenPrintf(0, kRowHeight * 0, interpolate0, " : interpolate0, Slerp(q0, q1, 0.0f)");
		QuaternionScreenPrintf(0, kRowHeight * 1, interpolate1, " : interpolate1, Slerp(q0, q1, 0.3f)");
		QuaternionScreenPrintf(0, kRowHeight * 2, interpolate2, " : interpolate2, Slerp(q0, q1, 0.5f)");
		QuaternionScreenPrintf(0, kRowHeight * 3, interpolate3, " : interpolate3, Slerp(q0, q1, 0.7f)");
		QuaternionScreenPrintf(0, kRowHeight * 4, interpolate4, " : interpolate4, Slerp(q0, q1, 1.0f)");

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
