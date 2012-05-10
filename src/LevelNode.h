#pragma once

#include "Geom.h"
#include "LevelEdge.h"
class LevelEdge;

class LevelNode
{
public:
	LevelNode(void);
	~LevelNode(void);

	LevelEdge *w;
	LevelEdge *e;
	LevelEdge *n;
	LevelEdge *s;
};

