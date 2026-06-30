#pragma once

#include "Vector3.h"

// ばねを表す構造体
struct Spring {
	// ばねの固定されている端の位置
	Vector3 anchor;

	// 外部から力を受けていないときの長さ
	float naturalLength;

	// ばねの硬さを表す値
	float stiffness;

	// 振動を弱める減衰係数
	float dampingCoefficient;
};

// ばねにつながれたボールを表す構造体
struct Ball {
	// ボールの現在位置
	Vector3 position;

	// ボールの移動速度
	Vector3 velocity;

	// ボールの加速度
	Vector3 acceleration;

	// ボールの質量
	float mass;

	// 描画するボールの半径
	float radius;

	// 描画色
	unsigned int color;
};