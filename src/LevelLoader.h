#pragma once

#include "tinyxml2.h"
#include "LevelNode.h"
#include "LevelLoadNode.h"
#ifdef _WIN32
#include <hash_map>
#else
#include <hash_map.h>
#endif

#include <vector>

using namespace std;
using namespace tinyxml2;

class LevelLoader
{
public:
	LevelLoader(void);
	~LevelLoader(void);
	//static hash_map<LevelNode*, XMLElement*>  levelNodes;
	//XMLElement* GetOrCreateXMLNode(LevelNode* levelNode, XMLElement* levels, XMLDocument* doc);

	static LevelNode* LoadXml(int level);
	//static void SaveXml(int index, LevelNode* root)
};

