#pragma once

#include "Vec2.h"

class Geom
{
public:
	Geom(bool isStatic, bool isGoal, Vec2 extends, unsigned int colorR, unsigned int colorG, unsigned int colorB);
	~Geom(void);

	bool isStatic;
	bool isGoal;
	Vec2 extends;
	unsigned int colorR;
	unsigned int colorG;
	unsigned int colorB;
};

