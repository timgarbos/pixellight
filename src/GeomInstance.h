#pragma once

#include "Vec2.h"
#include "Geom.h"

class GeomInstance
{
public:
	GeomInstance(void);
	~GeomInstance(void);
	
	Vec2 pos;
	Geom * masterGeom;
};

