#include "LevelLoader.h"


LevelLoader::LevelLoader(void)
{
}


LevelLoader::~LevelLoader(void)
{
}


LevelNode* LevelLoader::LoadXml()
{
	LevelNode* rootNode = NULL;
	XMLDocument* doc = new XMLDocument();
	if(doc->LoadFile("levels/levels.xml") == XML_SUCCESS)
	{
		printf("\n xml: %s",doc->Value());
		printf("\nLoaded XML.",0);
	}
	else
	{
		printf("\nLoaded XML FALIED.",0);
	}
	XMLNode* levels = doc->FirstChild()->ToElement();
	XMLNode * level;
	
	for( level = levels->FirstChild(); level; level = level->NextSibling() )
	{
		printf("\n starting load level",0);
		//To make relations easy we create a list of all the nodes, in order to later connect them
		vector<LevelLoadNode*> levelNodes;

		XMLNode * node;
		for(node = level->FirstChild(); node; node = node->NextSibling() )
		{
			LevelNode* newNode = new LevelNode();
			LevelLoadNode* newLevelNode = new LevelLoadNode(newNode);
			
			newLevelNode->e = node->ToElement()->IntAttribute("e");
			newLevelNode->w = node->ToElement()->IntAttribute("w");
			newLevelNode->n = node->ToElement()->IntAttribute("n");
			newLevelNode->s = node->ToElement()->IntAttribute("s");

			newNode->e->ccwSteps = node->ToElement()->IntAttribute("eccw");
			newNode->w->ccwSteps = node->ToElement()->IntAttribute("eccw");
			newNode->n->ccwSteps = node->ToElement()->IntAttribute("eccw");
			newNode->s->ccwSteps = node->ToElement()->IntAttribute("eccw");

			//Assuming we are getting them in order
			XMLNode * object;
			for(object = node->FirstChild(); object; object = object->NextSibling() )
			{
				Geom* newGeom = new Geom(true,Vec2(0,0));
				GeomInstance* newGeomInstance = new GeomInstance(Vec2(0,0),newGeom);

			
				newGeomInstance->pos.x = object->ToElement()->IntAttribute("x");
				newGeomInstance->pos.y = object->ToElement()->IntAttribute("y");

				newGeom->extends.x = object->ToElement()->IntAttribute("extendsX");
				newGeom->extends.y = object->ToElement()->IntAttribute("extendsY");

				newGeom->isStatic = object->ToElement()->BoolAttribute("type");


				newNode->objs.push_back(newGeomInstance);

				printf("\n LOADED OBJECT \n",0);
			}
		
			levelNodes.push_back(newLevelNode);
			printf("\n LOADED NODE \n",0);
		}

		rootNode = levelNodes[0]->node;
		//Go through all levelnodes and make dependcies
		for(int i=0;i<levelNodes.size();i++)
		{
			levelNodes[i]->node->e->Node = levelNodes[levelNodes[i]->e]->node;
			//Only connecting in one direction for now.
			//levelNodes[levelNodes[i]->e]->node->w->Node = levelNodes[i]->node;

			levelNodes[i]->node->w->Node = levelNodes[levelNodes[i]->w]->node;
			levelNodes[i]->node->n->Node = levelNodes[levelNodes[i]->n]->node;
			levelNodes[i]->node->s->Node = levelNodes[levelNodes[i]->s]->node;
		}


		printf("\n LOADED LEVEL \n",0);
		
	}
	
	return rootNode;
}