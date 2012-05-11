#include "LevelLoader.h"


LevelLoader::LevelLoader(void)
{
}


LevelLoader::~LevelLoader(void)
{
}


LevelNode* LevelLoader::LoadXml(int index)
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
		printf("\nLoaded XML FALIED. %i",doc->ErrorID());
	}
	XMLNode* levels = doc->FirstChild()->ToElement();
	XMLNode * level;
	
	level = levels->FirstChild();
	for(int i=0;i<index;i++)
		levels->NextSibling();
	
		//To make relations easy we create a list of all the nodes, in order to later connect them
		vector<LevelNode*> levelNodes;

		XMLNode * node;
		for(node = level->FirstChild()->NextSibling()->FirstChild(); node; node = node->NextSibling() )
		{
			LevelNode* newNode = new LevelNode();
			
			//Assuming we are getting them in order
			XMLNode * object;
			for(object = node->FirstChild(); object; object = object->NextSibling() )
			{
				Geom* newGeom = new Geom(true,Vec2(0,0));
				GeomInstance* newGeomInstance = new GeomInstance(Vec2(0,0),newGeom);

			
				newGeomInstance->pos.x = object->ToElement()->FloatAttribute("x");
				newGeomInstance->pos.y = object->ToElement()->FloatAttribute("y");

				newGeom->extends.x = object->ToElement()->FloatAttribute("extendsX");
				newGeom->extends.y = object->ToElement()->FloatAttribute("extendsY");

				newGeom->isStatic = object->ToElement()->BoolAttribute("type");


				newNode->objs.push_back(newGeomInstance);

				printf("\n LOADED OBJECT \n",0);
			}
		
			levelNodes.push_back(newNode);
			printf("\n LOADED NODE \n",0);
		}

		rootNode = levelNodes[0];
		//Go through all edges
		 
		XMLNode * edge;
		
		for(edge = level->FirstChild()->FirstChild()->FirstChild(); edge; edge = edge->NextSibling() )
		{
			
			int start = edge->ToElement()->IntAttribute("start");
			int end = edge->ToElement()->IntAttribute("end");
			int side = edge->ToElement()->IntAttribute("side");
			int ccw = edge->ToElement()->IntAttribute("ccw");

			printf("\n loading edge start: %i end: %i side: %i  ccw: %i",start,end,side,ccw);
			//Set start edges
			switch(side)
			{
			case 0:
				levelNodes[start]->s->Node = levelNodes[end];
				levelNodes[start]->s->ccwSteps = ccw;
				levelNodes[end]->n->Node = levelNodes[start];
				levelNodes[end]->n->ccwSteps = 4-ccw;
				break;
			case 1:
				levelNodes[start]->e->Node = levelNodes[end];
				levelNodes[start]->e->ccwSteps = ccw;
				levelNodes[end]->w->Node = levelNodes[start];
				levelNodes[end]->w->ccwSteps = 4-ccw;
				break;
			case 2:
				levelNodes[start]->n->Node = levelNodes[end];
				levelNodes[start]->n->ccwSteps = ccw;
				levelNodes[end]->s->Node = levelNodes[start];
				levelNodes[end]->s->ccwSteps = 4-ccw;
				break;
			case 3:
				levelNodes[start]->w->Node = levelNodes[end];
				levelNodes[start]->w->ccwSteps = ccw;
				levelNodes[end]->e->Node = levelNodes[start];
				levelNodes[end]->e->ccwSteps = 4-ccw;
				break;
			}
		}
		printf("\n LOADED LEVEL \n",0);

	delete doc;
	return rootNode;
}