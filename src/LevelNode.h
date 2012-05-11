#pragma once

#include <list>
using namespace std;

#include "Geom.h"
#include "GeomInstance.h"
#include "LevelEdge.h"

// GL
#include <GL/glfw.h>


class LevelEdge;

class LevelNode
{
public:
	LevelNode(void);
	~LevelNode(void);
	void CreateRandomWorld(int depth);
	void Draw(Vec2 pos);
	LevelEdge *w;
	LevelEdge *e;
	LevelEdge *n;
	LevelEdge *s;
	

	list<GeomInstance*> objs;

};

