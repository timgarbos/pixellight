#pragma once

#include "Vec2.h"

class Geom
{
public:
	Geom(bool isStatic, bool isGoal, bool isBad, Vec2 extends, int colorR, int colorG, int colorB);
	~Geom(void);

	bool isStatic;
	bool isGoal;
	bool isBad;
	Vec2 extends;
	int colorR;
	int colorG;
	int colorB;
};

