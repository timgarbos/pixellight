#include "LevelEdge.h"
#include "LevelNode.h"

LevelNode::LevelNode(void)
{

	//Default empty null edges
	w = new LevelEdge(0,0);
	e = new LevelEdge(0,0);
	s = new LevelEdge(0,0);
	n = new LevelEdge(0,0);
}


LevelNode::~LevelNode(void)
{
}


void LevelNode::CreateRandomWorld(int depth, int dir, LevelNode* back)
{
	//create piece of geometry
	Geom* newGeom = new Geom(true,Vec2(2,1));
	GeomInstance* newGeomInst = new GeomInstance(Vec2(0,0),newGeom);
	objs.push_back(newGeomInst);

	if(depth<1) //stop at some test level
	{
		depth++;

		LevelNode* wNode = new LevelNode();
		wNode->CreateRandomWorld(depth,0,this);
		w->Node = wNode;

		LevelNode* eNode = new LevelNode();
		eNode->CreateRandomWorld(depth,1,this);
		e->Node = eNode;

		LevelNode* nNode = new LevelNode();
		nNode->CreateRandomWorld(depth,2,this);
		n->Node = nNode;

		LevelNode* sNode = new LevelNode();
		sNode->CreateRandomWorld(depth,3,this);
		s->Node = sNode;
	}

	
	switch(dir) //set back node. TODO: We should delete the old one first!
	{
		case 0: w->Node = back;
			break;
		case 1: e->Node = back;
			break;
		case 2: n->Node = back;
			break;
		case 3: s->Node = back;
			break;

	}


}