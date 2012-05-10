#pragma once
#include "LevelNode.h"

class LevelNode;

class LevelEdge
{
public:
	LevelEdge(void);
	~LevelEdge(void);

	int  ccwSteps;
	LevelNode * Node;
};

