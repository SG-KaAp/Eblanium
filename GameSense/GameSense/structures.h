#pragma once

struct ColorS {
	int r, g, b, a;
};

struct Vec1 {
	float x;
};

struct Vec2 {
	float x,y;
};

struct Vec3 {
	float x,y,z;
};

struct Vec4 {
	float x, y, z, w;
};

struct BoneMatrix_t {
	byte pad1[12];
	float x;
	byte pad1[12];
	float y;
	byte pad1[12];
	float z;
};

ColorS BoxColor;