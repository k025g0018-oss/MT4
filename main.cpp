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

	// 4x4行列
	Marix4x4

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

		Matrix4x4 resultAdd = Add(m1, m2);

		Matrix4x4 resultMultiply = Multiply(m1, m2);

		Matrix4x4 resultSubtract = Subtract(m1, m2);

		Matrix4x4 inverseM1 = Inverse(m1);

		Matrix4x4 inverseM2 = Inverse(m2);

		Matrix4x4 transposeM1 = Transpose(m1);

		Matrix4x4 transposeM2 = Transpose(m2);

		Matrix4x4 identity = MakeIdentity4x4();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, resultAdd, "Add");

		MatrixScreenPrintf(0, kRowHeight * 5, resultSubtract, "Subtract");

		MatrixScreenPrintf(0, kRowHeight * 5 * 2, resultMultiply, "Multiply");

		MatrixScreenPrintf(0, kRowHeight * 5 * 3, inverseM1, "inverseM1");

		MatrixScreenPrintf(0, kRowHeight * 5 * 4, inverseM2, "inverseM2");

		MatrixScreenPrintf(kColumnWidth * 5, 0, transposeM1, "transposeM1");

		MatrixScreenPrintf(kColumnWidth * 5, kRowHeight * 5, transposeM2, "transposeM2");

		MatrixScreenPrintf(kColumnWidth * 5, kRowHeight * 5 * 2, identity, "identity");

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
