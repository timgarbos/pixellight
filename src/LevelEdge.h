#pragma once
#include "LevelNode.h"

class LevelNode;

class LevelEdge
{
public:
	LevelEdge(int ccwSteps, LevelNode * Node);
	~LevelEdge(void);

	int  ccwSteps;
	LevelNode * Node;
};

