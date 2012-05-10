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


void LevelNode::CreateRandomWorld(int depth)
{
	//create piece of geometry
	Geom* newGeom = new Geom(true,Vec2(2,1));
	GeomInstance* newGeomInst = new GeomInstance(Vec2(0,0),newGeom);
	objs.push_back(newGeomInst);

	printf("\ndepth: %i",depth);

	if(false) //stop at some test level
	{
		depth++;

		LevelNode* wNode = new LevelNode();
		wNode->CreateRandomWorld(depth);
		w->Node = wNode;
		wNode->e->Node = this;

		LevelNode* eNode = new LevelNode();
		eNode->CreateRandomWorld(depth);
		e->Node = eNode;
		eNode->w->Node = this;

		LevelNode* nNode = new LevelNode();
		nNode->CreateRandomWorld(depth);
		n->Node = nNode;
		nNode->s->Node = this;

		LevelNode* sNode = new LevelNode();
		sNode->CreateRandomWorld(depth);
		s->Node = sNode;
		sNode->n->Node = this;
	}
	



}