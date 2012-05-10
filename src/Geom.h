#pragma once

#include "Vec2.h"

class Geom
{
public:
	Geom(bool isStatic, Vec2 extends);
	~Geom(void);

	bool isStatic;
	Vec2 extends;
};

