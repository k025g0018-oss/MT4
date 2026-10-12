#pragma once

#include <algorithm>
#include <cmath>
#include "Vector2.h"

/// <summary>
/// イージング計算
/// </summary>
class Easing {
public:
	/// --- 線形補間 ---

	// start→end を割合 t で補間する
	static float Lerp(float start, float end, float t) {
		return start + (end - start) * t;
	}

	// Vector2版
	static Vector2 Lerp(const Vector2& start, const Vector2& end, float t) {
		return {
			Lerp(start.x, end.x, t),
			Lerp(start.y, end.y, t)
		};
	}

	/// --- イージング ---

	// 開始値から終了値まで、終点に近づくほど減速しながら補間する
	static float EaseOut(float start, float end, float t) {
		t = std::clamp(t, 0.0f, 1.0f);

		// EaseOutの進行度を計算
		float easedT = 1.0f - (1.0f - t) * (1.0f - t);

		// 土台のLerpで補間
		return Lerp(start, end, easedT);
	}

	// 開始値から終了値まで、徐々に加速しながら補間する
	static float EaseIn(float start, float end, float t) {
		t = std::clamp(t, 0.0f, 1.0f);

		float easedT = t * t;

		return Lerp(start, end, easedT);
	}
};
