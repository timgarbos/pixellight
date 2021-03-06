#include "LevelLoader.h"

#define LEVELDEBUG 0

LevelLoader::LevelLoader(void)
{
}


LevelLoader::~LevelLoader(void)
{
}
/*
//Didn't plan for this. Now it's static!
hash_map<LevelNode*, XMLElement*>  LevelLoader::levelNodes;


XMLElement* LevelLoader::GetOrCreateXMLNode(LevelNode* levelNode, XMLElement* levels, XMLDocument* doc)
{
	if(levelNodes.find(levelNode)!=levelNodes.end())
	{// does not exists
		
		XMLElement * node = doc->NewElement("node"); 
		XMLElement * objects = doc->NewElement("objects");
		node->LinkEndChild(objects);
		list<GeomInstance*>::iterator i;;
		
		for(i = levelNode->objs.begin();i!=levelNode->objs.end();i++)
		{
			
			XMLElement * object = doc->NewElement("object");
			//<object x="0.0f" y="0.0f" extendsX="0.1f" extendsY="0.1f" type="1"/>
			object->SetAttribute("x",(*i)->pos.x);
			object->SetAttribute("y",(*i)->pos.y);
			object->SetAttribute("extendsX",(*i)->masterGeom->extends.x);
			object->SetAttribute("extendsY",(*i)->masterGeom->extends.y);
			object->SetAttribute("type",(*i)->masterGeom->isStatic);
			objects->LinkEndChild(node);
		}
		//Then create it's neighbors
		levels->LinkEndChild(node);
		levelNodes[levelNode] = node;

		GetOrCreateXMLNode(levelNode->e->Node,levels,doc);
		GetOrCreateXMLNode(levelNode->w->Node,levels,doc);
		GetOrCreateXMLNode(levelNode->n->Node,levels,doc);
		GetOrCreateXMLNode(levelNode->s->Node,levels,doc);

		
		return node;
	}
	else
		return levelNodes[levelNode];
}
void LevelLoader::SaveXml(int index, LevelNode* root)
{
	
	
	//First let's create all the xml nodes. With geometry
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
	XMLElement * level = doc->NewElement("level");
	XMLElement * nodes = doc->NewElement("nodes");
	level->LinkEndChild(nodes);
	
	//Then all the edges
}*/
LevelNode* LevelLoader::LoadXml(int index)
{
	LevelNode* rootNode = NULL;
	XMLDocument* doc = new XMLDocument();
	if(doc->LoadFile("levels/levels.xml") == XML_SUCCESS)
	{
#if LEVELDEBUG
		printf("\n xml: %s",doc->Value());
		printf("\nLoaded XML.",0);
#endif
	}
	else
	{
#if LEVELDEBUG
		printf("\nLoaded XML FALIED. %i",doc->ErrorID());
#endif
	}
	XMLNode* nodes = doc->FirstChild()->FirstChild()->NextSibling();
		//To make relations easy we create a list of all the nodes, in order to later connect them
		vector<LevelNode*> levelNodes;

		XMLNode * node;
		for(node = nodes->FirstChild(); node; node = node->NextSibling() )
		{
			LevelNode* newNode = new LevelNode();
			
			//Assuming we are getting them in order
			newNode->colorR = node->ToElement()->IntAttribute("r");
			newNode->colorG = node->ToElement()->IntAttribute("g");
			newNode->colorB = node->ToElement()->IntAttribute("b");
			newNode->colorMod = node->ToElement()->IntAttribute("m");
			//if(newNode->colorMod==0)
				newNode->colorMod = 2;
            
            newNode->audio = node->ToElement()->IntAttribute("audio");
			
			XMLNode * object;
			if(node->FirstChild())
			for(object = node->FirstChild()->FirstChild(); object; object = object->NextSibling() )
			{
				Geom* newGeom = new Geom(true,false,false,Vec2(0,0),0,0,0);
				GeomInstance* newGeomInstance = new GeomInstance(Vec2(0,0),newGeom);

				newGeomInstance->pos.x = object->ToElement()->FloatAttribute("x");
				newGeomInstance->pos.y = object->ToElement()->FloatAttribute("y");

				newGeom->colorR = object->ToElement()->IntAttribute("colorR");
				newGeom->colorG = object->ToElement()->IntAttribute("colorG");
				newGeom->colorB = object->ToElement()->IntAttribute("colorB");

				newGeom->extends.x = object->ToElement()->FloatAttribute("extendsX")-(1e-7);
				newGeom->extends.y = object->ToElement()->FloatAttribute("extendsY")-(1e-7);

				newGeom->isStatic = object->ToElement()->BoolAttribute("type");
				newGeom->isGoal = object->ToElement()->BoolAttribute("goal");
				newGeom->isBad = object->ToElement()->BoolAttribute("bad");

				newNode->objs.push_back(newGeomInstance);

#if LEVELDEBUG
				printf("\n LOADED OBJECT \n",0);
#endif
			}
		
			levelNodes.push_back(newNode);
#if LEVELDEBUG
			printf("\n LOADED NODE \n",0);
#endif
		}
		//Go through all edges
		 
		XMLNode * level = doc->FirstChild()->FirstChild()->FirstChild();
		for(int i = 0; i<index;i++)
			level = level->NextSibling();

		rootNode = levelNodes[level->ToElement()->IntAttribute("start")];
		XMLNode * edge;
		
		for(edge = level->FirstChild(); edge; edge = edge->NextSibling() )
		{
			
			int start = edge->ToElement()->IntAttribute("start");
			int end = edge->ToElement()->IntAttribute("end");
			int side = edge->ToElement()->IntAttribute("side");
			int ccw = edge->ToElement()->IntAttribute("ccw");

#if LEVELDEBUG
			printf("\n loading edge start: %i end: %i side: %i  ccw: %i",start,end,side,ccw);
#endif
			if(end<0)
				continue;
			//Set start edges
			int nccw = (4-ccw)%4;;
			switch(side)
			{
			case 0:
				levelNodes[start]->s->Node = levelNodes[end];
				levelNodes[start]->s->ccwSteps = ccw;
				if(ccw==0)
				{
					levelNodes[end]->n->Node = levelNodes[start];
					levelNodes[end]->n->ccwSteps = nccw;
				}
				if(ccw==1)
				{
					levelNodes[end]->w->Node = levelNodes[start];
					levelNodes[end]->w->ccwSteps = nccw;
				}
				
				if(ccw==2)
				{
					levelNodes[end]->s->Node = levelNodes[start];
					levelNodes[end]->s->ccwSteps = nccw;
				}
				if(ccw==3)
				{
					levelNodes[end]->e->Node = levelNodes[start];
					levelNodes[end]->e->ccwSteps = nccw;
				}

				break;
			case 1:
				levelNodes[start]->e->Node = levelNodes[end];
				levelNodes[start]->e->ccwSteps = ccw;
				if(ccw==0)
				{
					levelNodes[end]->w->Node = levelNodes[start];
					levelNodes[end]->w->ccwSteps = nccw;
				}
				if(ccw==1)
				{
					levelNodes[end]->s->Node = levelNodes[start];
					levelNodes[end]->s->ccwSteps = nccw;
				}
				
				if(ccw==2)
				{
					levelNodes[end]->e->Node = levelNodes[start];
					levelNodes[end]->e->ccwSteps = nccw;
				}
				if(ccw==3)
				{
					levelNodes[end]->n->Node = levelNodes[start];
					levelNodes[end]->n->ccwSteps = nccw;
				}

				break;
			case 2:
				levelNodes[start]->n->Node = levelNodes[end];
				levelNodes[start]->n->ccwSteps = ccw;
				if(ccw==0)
				{
					levelNodes[end]->s->Node = levelNodes[start];
					levelNodes[end]->s->ccwSteps = nccw;
				}
				if(ccw==1)
				{
					levelNodes[end]->e->Node = levelNodes[start];
					levelNodes[end]->e->ccwSteps = nccw;
				}
				
				if(ccw==2)
				{
					levelNodes[end]->n->Node = levelNodes[start];
					levelNodes[end]->n->ccwSteps = nccw;
				}
				if(ccw==3)
				{
					levelNodes[end]->w->Node = levelNodes[start];
					levelNodes[end]->w->ccwSteps = nccw;
				}

				break;
			case 3:
				levelNodes[start]->w->Node = levelNodes[end];
				levelNodes[start]->w->ccwSteps = ccw;
				if(ccw==0)
				{
					levelNodes[end]->e->Node = levelNodes[start];
					levelNodes[end]->e->ccwSteps = nccw;
				}
				if(ccw==1)
				{
					levelNodes[end]->n->Node = levelNodes[start];
					levelNodes[end]->n->ccwSteps = nccw;
				}
				
				if(ccw==2)
				{
					levelNodes[end]->w->Node = levelNodes[start];
					levelNodes[end]->w->ccwSteps = nccw;
				}
				if(ccw==3)
				{
					levelNodes[end]->s->Node = levelNodes[start];
					levelNodes[end]->s->ccwSteps = nccw;
				}

				break;
			}
		}
#if LEVELDEBUG
		printf("\n LOADED LEVEL \n",0);
#endif

	delete doc;
	return rootNode;
}