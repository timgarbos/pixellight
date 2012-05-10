#pragma once

#include <list>
using namespace std;

#include "Geom.h"
#include "GeomInstance.h"
#include "LevelEdge.h"
class LevelEdge;

class LevelNode
{
public:
	LevelNode(void);
	~LevelNode(void);
	void CreateRandomWorld(int depth, int dir, LevelNode* parent);

	LevelEdge *w;
	LevelEdge *e;
	LevelEdge *n;
	LevelEdge *s;

	list<GeomInstance*> objs;

};

