#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <math.h>

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
// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{};

	// 単位行列
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	// 4行目に移動量
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

// 拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{};

	// 対角に
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

// 座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};

	// (x, y, z, 1) * Matrix
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	// w成分の計算
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	// 同次座標のwで割る
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

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

	Vector3 translate{4.1f, 2.6f, 0.8f};

	Vector3 scale{1.5f, 5.2f, 7.3f};

	Vector3 point{2.3f, 3.8f, 1.4f};

	Matrix4x4 transformMatrix = {
		1.0f, 2.0f, 3.0f, 4.0f,
		3.0f, 1.0f, 1.0f, 2.0f,
		1.0f, 4.0f, 2.0f, 3.0f,
		2.0f, 2.0f, 1.0f, 3.0f
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

		Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

		Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

		Vector3 transformed = Transform(point, transformMatrix);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, transformed, "transformed");

		MatrixScreenPrintf(0, kRowHeight * 2, translateMatrix, "translateMatrix");

		MatrixScreenPrintf(0, kRowHeight * 8, scaleMatrix, "scaleMatrix");

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
