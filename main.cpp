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
// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

// スカラー倍
Vector3 Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result = {scalar * v.x, scalar * v.y, scalar * v.z};
	return result;
}

// 内積
float Dot(const Vector3& v1, const Vector3& v2) {
	float result;
	result =
		v1.x * v2.x +
		v1.y * v2.y +
		v1.z * v2.z;
	return result;
}

// 長さ(ノルム)
float Length(const Vector3& v) {
	float result;
	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return result;
}

// 正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	// 長さを計算
	float length = Length(v);

	// 0の時はエラーが出るのでそのまま0を返す分岐を作る
	if (length != 0.0f) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	} else {
		// 長さが0の場合は０を返す
		result = {0.0f, 0.0f, 0.0f};
	}

	return result;
}

// x軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);

	return{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, c, s, 0.0f,
		0.0f, -s, c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

// y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);

	return{
		c, 0.0f, -s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		s, 0.0f, c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

// z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	float s = std::sin(radian);
	float c = std::cos(radian);

	return{
		c, s, 0.0f, 0.0f,
		-s, c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

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

// Vector3の数値表示
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	Novice::ScreenPrintf(x, y + kRowHeight, "%.02f, %.02f, %.02f", vector.x, vector.y, vector.z);
}

// Matrix4x4同士の掛け算
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

// 3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result{};

	// 各成分の行列を作成
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// 回転行列の合成
	Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	// SRTの順番で行列を合成
	result = Multiply(scaleMatrix, rotateMatrix);
	result = Multiply(result, translateMatrix);

	return result;
}

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result{};

	// コンタンジェントを計算 ( 1 / tan(fovY / 2) )
	float cot = 1.0f / std::tan(fovY / 2.0f);

	// tanθ = a/b, cotθ = 1 / tanθ = b/a, y * cotθ -> a * b/a = b

	// result.m[0][0] = (1.0f / aspectRatio) * (fovY / 2.0f);
	// result.m[1][1] = fovY / 2.0f;

	result.m[0][0] = cot / aspectRatio;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = cot;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = -(nearClip * farClip) / (farClip - nearClip);
	result.m[3][3] = 0.0f;

	return result;
}

// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{};

	result.m[0][0] = 2.0f / (right - left);
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[2][3] = 0.0f;
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}

// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result{};

	result.m[0][0] = width / 2.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[2][3] = 0.0f;
	result.m[3][0] = left + (width / 2.0f);
	result.m[3][1] = top + (height / 2.0f);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;

	return result;
}

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	// a * b = {(a.y * b.z - a.z * b.y), (a.z * b.x - a.x * b.z), (a.x * b.y - a.y * b.x)}
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

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
	// クロス積の確認用
	Vector3 v1{1.2f, -3.9f, 2.5f};
	Vector3 v2{2.8f, 0.4f, -1.3f};
	Vector3 cross = Cross(v1, v2);

	// 三角形のローカル座標、中心を原点
	Vector3 kLocalVertices[3] = {
		{0.0f, 0.5f, 0.0f}, // 上
		{0.5f, -0.5f, 0.0f}, // 右下
		{-0.5f, -0.5f, 0.0f}, // 左下
	};

	// 三角形の初期トランスフォーム
	Vector3 translate = {0.0f, 0.0f, 0.0f};
	Vector3 rotate = {0.0f, 0.0f, 0.0f};

	// カメラの初期位置
	Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};

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

		// w,sキーで前後に、a,dキーで左右に三角形を動かす
		if (keys[DIK_W]) {
			translate.z += 0.1f;
		}

		if (keys[DIK_S]) {
			translate.z -= 0.1f;
		}

		if (keys[DIK_A]) {
			translate.x -= 0.1f;
		}

		if (keys[DIK_D]) {
			translate.x += 0.1f;
		}

		// y軸を回転させる、translateとrotateの値を変更させる
		rotate.y += 0.03f;

		/// 各種行列の計算
		// ワールド行列、SRT
		Matrix4x4 worldMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, rotate, translate);

		// カメラ
		Matrix4x4 cameraMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, cameraPosition);

		// ビュー行列
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);

		// 投影行列
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(screenSize.x) / float(screenSize.y), 0.1f, 100.0f);

		// 合成行列
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

		// ビューポート行列
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(screenSize.x), float(screenSize.y), 0.0f, 1.0f);

		// 頂点変換
		Vector3 screenVertices[3];
		for (uint32_t i = 0; i < 3; ++i) {
			// ローカル空間からNDC空間
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			// NDC空間からスクリーン空間
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// 三角ポリゴン
		Novice::DrawTriangle(
			int(screenVertices[0].x), int(screenVertices[0].y),
			int(screenVertices[1].x), int(screenVertices[1].y),
			int(screenVertices[2].x), int(screenVertices[2].y),
			RED,
			kFillModeSolid
		);

		// クロス積の確認用
		VectorScreenPrintf(0, 0, cross, "Cross");

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
