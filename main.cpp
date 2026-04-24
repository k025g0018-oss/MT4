#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <math.h>

const char kWindowTitle[] = "LE2B_21_タヤ_ナオユキ_MT3_00_01";

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
// 行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m1.m[row][col] + m2.m[row][col];
		}
	}
	return result;
}

// 行列の減法
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m1.m[row][col] - m2.m[row][col];
		}
	}
	return result;
}

// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			// 各成分の計算
			result.m[row][col] = 
				m1.m[row][0] * m2.m[0][col] +
				m1.m[row][1] * m2.m[1][col] +
				m1.m[row][2] * m2.m[2][col] +
				m1.m[row][3] * m2.m[3][col];
		}
	}
	return result;
}

// 逆行列
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 r{};

	float a11 = m.m[0][0], a12 = m.m[0][1], a13 = m.m[0][2], a14 = m.m[0][3],
		a21 = m.m[1][0], a22 = m.m[1][1], a23 = m.m[1][2], a24 = m.m[1][3],
		a31 = m.m[2][0], a32 = m.m[2][1], a33 = m.m[2][2], a34 = m.m[2][3],
		a41 = m.m[3][0], a42 = m.m[3][1], a43 = m.m[3][2], a44 = m.m[3][3];

	// 逆列式 |A|
	float det =
		a11 * a22 * a33 * a44 +
		a11 * a23 * a34 * a42 +
		a11 * a24 * a32 * a43 -
		a11 * a24 * a33 * a42 -
		a11 * a23 * a32 * a44 -
		a11 * a22 * a34 * a43 -
		a12 * a21 * a33 * a44 -
		a13 * a21 * a34 * a42 -
		a14 * a21 * a32 * a43 +
		a14 * a21 * a33 * a42 +
		a13 * a21 * a32 * a44 +
		a12 * a21 * a34 * a43 +
		a12 * a23 * a31 * a44 +
		a13 * a24 * a31 * a42 +
		a14 * a22 * a31 * a43 -
		a14 * a23 * a31 * a42 -
		a13 * a22 * a31 * a44 -
		a12 * a24 * a31 * a43 -
		a12 * a23 * a34 * a41 -
		a13 * a24 * a32 * a41 -
		a14 * a22 * a33 * a41 +
		a14 * a23 * a32 * a41 +
		a13 * a22 * a34 * a41 +
		a12 * a24 * a33 * a41;

	// 逆行列が存在しないとき
	if (det == 0.0f) {
		return r;
	}

	float invDet = 1.0f / det;

	r.m[0][0] = (
		a22 * a33 * a44 +
		a23 * a34 * a42 +
		a24 * a32 * a43 -
		a24 * a33 * a42 -
		a23 * a32 * a44 -
		a22 * a34 * a43
		) * invDet;

	r.m[0][1] = (
		-a12 * a33 * a44 -
		a13 * a34 * a42 -
		a14 * a32 * a43 +
		a14 * a33 * a42 +
		a13 * a32 * a44 +
		a12 * a34 * a43
		) * invDet;
	
	r.m[0][2] = (
		a12 * a23 * a44 +
		a13 * a24 * a42 +
		a14 * a22 * a43 -
		a14 * a23 * a42 -
		a13 * a22 * a44 -
		a12 * a24 * a43
		) * invDet;

	r.m[0][3] = (
		-a12 * a23 * a34 -
		a13 * a24 * a32 -
		a14 * a22 * a33 +
		a14 * a23 * a32 +
		a13 * a22 * a34 +
		a12 * a24 * a33
		) * invDet;

	r.m[1][0] = (
		-a21 * a33 * a44 -
		a23 * a34 * a41 -
		a24 * a31 * a43 +
		a24 * a33 * a41 +
		a23 * a31 * a44 +
		a21 * a34 * a43
		) * invDet;

	r.m[1][1] = (
		a11 * a33 * a44 +
		a13 * a34 * a41 +
		a14 * a31 * a43 -
		a14 * a33 * a41 -
		a13 * a31 * a44 -
		a11 * a34 * a43
		) * invDet;

	r.m[1][2] = (
		-a11 * a23 * a44 -
		a13 * a24 * a41 -
		a14 * a21 * a43 +
		a14 * a23 * a41 +
		a13 * a21 * a44 +
		a11 * a24 * a43
		) * invDet;

	r.m[1][3] = (
		a11 * a23 * a34 +
		a13 * a24 * a31 +
		a14 * a21 * a33 -
		a14 * a23 * a31 -
		a13 * a21 * a34 -
		a11 * a24 * a33
		) * invDet;

	r.m[2][0] = (
		a21 * a32 * a44 +
		a22 * a34 * a41 +
		a24 * a31 * a42 -
		a24 * a32 * a41 -
		a22 * a31 * a44 -
		a21 * a34 * a42
		) * invDet;

	r.m[2][1] = (
		-a11 * a32 * a44 -
		a12 * a34 * a41 -
		a14 * a31 * a42 +
		a14 * a32 * a41 +
		a12 * a31 * a44 +
		a11 * a34 * a42
		) * invDet;

	r.m[2][2] = (
		a11 * a22 * a44 +
		a12 * a24 * a41 +
		a14 * a21 * a42 -
		a14 * a22 * a41 -
		a12 * a21 * a44 -
		a11 * a24 * a42
		) * invDet;


	r.m[2][3] = (
		-a11 * a22 * a34 -
		a12 * a24 * a31 -
		a14 * a21 * a32 +
		a14 * a22 * a31 +
		a12 * a21 * a34 +
		a11 * a24 * a32
		) * invDet;

	r.m[3][0] = (
		-a21 * a32 * a43 -
		a22 * a33 * a41 -
		a23 * a31 * a42 +
		a23 * a32 * a41 +
		a22 * a31 * a43 +
		a21 * a33 * a42
		) * invDet;

	r.m[3][1] = (
		a11 * a32 * a43 +
		a12 * a33 * a41 +
		a13 * a31 * a42 -
		a13 * a32 * a41 -
		a12 * a31 * a43 -
		a11 * a33 * a42
		) * invDet;

	r.m[3][2] = (
		-a11 * a22 * a43 -
		a12 * a23 * a41 -
		a13 * a21 * a42 +
		a13 * a22 * a41 +
		a12 * a21 * a43 +
		a11 * a23 * a42
		) * invDet;

	r.m[3][3] = (
		a11 * a22 * a33 +
		a12 * a23 * a31 +
		a13 * a21 * a32 -
		a13 * a22 * a31 -
		a12 * a21 * a33 -
		a11 * a23 * a32
		) * invDet;

	return r;
}

// 転置行列
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	// 行と列のインデックスを入れ替える
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

// 単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (i == j) {
				result.m[i][j] = 1.0f; // 対角成分は1
			} else {
				result.m[i][j] = 0.0f; // それ以外は0
			}
		}
	}
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
	Matrix4x4 m1 = {
		3.2f, 0.7f, 9.6f, 4.4f,
		5.5f, 1.3f, 7.8f, 2.1f,
		6.9f, 8.0f, 2.6f, 1.0f,
		0.5f, 7.2f, 5.1f, 3.3f
	};

	Matrix4x4 m2 = {
		4.1f, 6.5f, 3.3f, 2.2f,
		8.8f, 0.6f, 9.9f, 7.7f,
		1.1f, 5.5f, 6.6f, 0.0f,
		3.3f, 9.9f, 8.8f, 2.2f
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
