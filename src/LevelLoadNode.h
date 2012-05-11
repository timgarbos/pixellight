#pragma once

#include "LevelNode.h"

class LevelLoadNode
{
public:
	LevelLoadNode(LevelNode* node);
	~LevelLoadNode(void);

	

	int e,w,n,s;
	LevelNode* node;

};

