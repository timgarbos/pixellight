#pragma once

#include "tinyxml2.h"
#include "LevelNode.h"
#include "LevelLoadNode.h"

#include <vector>

using namespace std;
using namespace tinyxml2;

class LevelLoader
{
public:
	LevelLoader(void);
	~LevelLoader(void);

	static LevelNode* LoadXml(int level);
};

