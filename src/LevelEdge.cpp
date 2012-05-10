#include "LevelEdge.h"


LevelEdge::LevelEdge(int ccwSteps, LevelNode * Node)
{
	this->ccwSteps = ccwSteps;
	this->Node = Node;
}


LevelEdge::~LevelEdge(void)
{
}
