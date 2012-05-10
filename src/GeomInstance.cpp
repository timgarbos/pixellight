#include "GeomInstance.h"


GeomInstance::GeomInstance(Vec2 pos,Geom * masterGeom)
{
	this->pos = pos;
	this->masterGeom = masterGeom;
}


GeomInstance::~GeomInstance(void)
{
}
