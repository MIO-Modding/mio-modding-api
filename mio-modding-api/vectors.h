#pragma once

typedef struct Vector3 {
	float x, y, z;
	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};
typedef struct Vector2 {
	float x, y;
	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}
};