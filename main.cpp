#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <cmath>

const char kWindowTitle[] = "LE2B_17_タヤ_ナオユキ_MT3";

//////////
/// 構造体
//////////

typedef struct Vector2 {
	float x;
	float y;
} Vector2;

// 3次元ベクトル
struct Vector3 {
	float x, y, z;
};

// 4x4行列
struct Matrix4x4 {
	float m[4][4];
};

/// 関数
// x軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {

}

// y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {

}

// z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {

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

// スカラー倍
Vector3 Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result = {scalar * v.x, scalar * v.y, scalar * v.z};
	return result;
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

	/// 定義エリア
	std::sin(radian);
	std::cos(radian);

	Vector3 rotate{0.4f, 1.43f, -0.8f};

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

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

		Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

		Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

		Vector3 transformed = Transform(point, transformMatrix);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, rotateXMatrix, "rotateXMatrix");

		MatrixScreenPrintf(0, kRowHeight * 5, rotateYMatrix, "rotateYMatrix");

		MatrixScreenPrintf(0, kRowHeight * 5 * 2, rotateZMatrix, "rotateZMatrix");

		MatrixScreenPrintf(0, kRowHeight * 5 * 3, rotateXYZMatrix, "rotateXYZMatrix");

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
