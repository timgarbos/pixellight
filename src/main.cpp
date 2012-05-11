/* main.cpp

pixellight!

*/

// libc++
#include <iostream>

// GL
#include <GL/glfw.h>

// fmod
#include <fmod.hpp>

// local
#include "Vec2.h"
#include "LevelNode.h"
#include "LevelEdge.h"
#include "Geom.h"

/*
	MACROS
*/
#define WINW		512
#define WINH		512
#define RAYSFRAME	50

/*
	globals
*/
Vec2		v00 = Vec2(-1.0f, -1.0f);	// lower left
Vec2		v10 = Vec2(1.0f, -1.0f);	// lower right
Vec2		v01 = Vec2(-1.0f, 1.0f);	// upper left
Vec2		v11 = Vec2(1.0f, 1.0f);		// upper right

LevelNode *	root;
Vec2		pos;

/*
	rayline
*/
inline bool rayline(Vec2 const & u0, Vec2 const & ud, Vec2 const & v0, Vec2 const & v1, float & ut, float & vt)
{
	float const vdx	= v1.x-v0.x;
	float const vdy	= v1.y-v0.y;
	float const d	= ud.y*vdx - ud.x*vdy;

	if (d != 0.0f)
	{
		float const r = 1.0f / d;
		float const a = r * (v0.y - u0.y);
		float const b = r * (u0.x - v0.x);

		ut	= vdx*a + vdy*b;
		vt	= ud.x*a + ud.y*b;

		return (ut > (0.0f + 1e-7/* f32 epsilon */) && vt >= 0.0f && vt <= 1.0f);
	}
	else
	{
		return false;
	}
}

/*
	rot90
*/
inline void rot90(Vec2 & v, int ccwSteps)
{
	switch (ccwSteps)
	{
	case 1:
		v.y *= -1.0f;
		std::swap(v.x, v.y);
		break;

	case 2:
		v.x *= -1.0f;
		v.y *= -1.0f;
		break;

	case 3:
		v.x *= -1.0f;
		std::swap(v.x, v.y);
		break;
	}
}

/*
	trace
*/
inline void trace(LevelNode * node, Vec2 pos, Vec2 dir, float dMax, float & dOut, Geom * & geom, LevelNode * & dest, Vec2 & npos)
{
	int c = 0;
	LevelEdge *	ce = NULL;
	Vec2		x;
	Vec2		startPos = pos;
	LevelNode* startNode = node;
	float		sx;
	float		sy;
	float		ut;	// distance along trace direction
	float		vt;	// distance along edge
	float		d = 0.0f;
	bool		f = false;

	//std::cout << "-- new trace" << std::endl;

	// assume no node changes
	dest = node;

	// loop until the maximum distance has been reached, or no more nodes
	while ((d < dMax) && node != NULL)
	{
		//std::cout << "node = " << node << std::endl;
		// find nearest intersection with local geometry
		//TODO
		//if (++c > 10)
		//{
		//	break;
		//}

		// if unsuccessful, then find intersection with node boundary
		if (!f)
		{
			// transfer to edge
			if (rayline(pos, dir, v00, v10, ut, vt))
			{
				//std::cout << "south, ut = " << ut << std::endl;
				ce		= node->s;// => south
				pos.x	= pos.x + ut*dir.x;
				pos.y	= -1.0f;
				sx		= 1.0f;
				sy		= -1.0f;
			}
			else if (rayline(pos, dir, v10, v11, ut, vt))
			{
				//std::cout << "east, ut = " << ut << std::endl;
				ce		= node->e;// => east
				pos.x	= 1.0f;
				pos.y	= pos.y + ut*dir.y;
				sx		= -1.0f;
				sy		= 1.0f;
			}
			else if (rayline(pos, dir, v11, v01, ut, vt))
			{
				//std::cout << "north, ut = " << ut << std::endl;
				ce		= node->n;// => north
				pos.x	= pos.x + ut*dir.x;
				pos.y	= 1.0f;
				sx		= 1.0f;
				sy		= -1.0f;
			}
			else if (rayline(pos, dir, v01, v00, ut, vt))
			{
				//std::cout << "west, ut = " << ut << std::endl;
				ce		= node->w;// => west
				pos.x	= -1.0f;
				pos.y	= pos.y + ut*dir.y;
				sx		= -1.0f;
				sy		= 1.0f;
			}

			// transform position
			rot90(pos, ce->ccwSteps);

			if ((ce->ccwSteps % 2) == 0)
			{
				pos.x *= sx;
				pos.y *= sy;
			}
			else
			{
				pos.x *= sy;
				pos.y *= sx;
			}

			//std::cout << "dir " << dir.x << ", " << dir.y << std::endl;
			//std::cout << "pos " << pos.x << ", " << pos.y << std::endl;

			// transform direction
			rot90(dir, ce->ccwSteps);

			// move to connecting node
			node = ce->Node;
			dest = node;
		}

		// increment distance travelled
		d += ut;
	}

	// cap the output
	if (d > dMax)
	{
		dOut = dMax;
		geom = NULL;
		dest = startNode;
		npos.y = startPos.y+dir.y;
		npos.x = startPos.x+dir.x;
	}
	else
	{
		dOut = d;
	}

	npos.x = pos.x;
	npos.y = pos.y;
}

/*
	debug world
*/
LevelNode* createDebugWorld()
{
	LevelNode* worldCenter = new LevelNode();
	worldCenter->CreateRandomWorld(0);
	return worldCenter;
}

/*
	move
*/
void move_view(Vec2 const & v)
{
	float		vlen = std::sqrt(v.x*v.x + v.y*v.y);
	Vec2		dir = Vec2(v.x / vlen, v.y / vlen);
	LevelNode *	dest = NULL;
	Geom *		geom = NULL;
	Vec2		npos;
	float		d;

	std::cout << "vlen = " << vlen << std::endl;
	trace(root, pos, dir, vlen, d, geom, dest, npos);

	root = dest;

	pos.x = npos.x;
	pos.y = npos.y;
}

/*
	game
*/
void game()
{
	float		step = (2.0f * 3.14f) / static_cast<float>(RAYSFRAME);
	Vec2		dir;
	Vec2		tmp;
	float		d;
	Geom *		geom = NULL;
	LevelNode *	dest = NULL;

	root = createDebugWorld();
	pos.x = 0;
	pos.y = 0;

	while (true)
	{
		glfwPollEvents();

		// quit on escape
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			break;
		}

		// handle input
		if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			std::cout << "ksbghfdbg" << std::endl;
			move_view(Vec2(-0.05f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			move_view(Vec2(0.5f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
		{
			move_view(Vec2(0.0f, 0.5f));
		}
		else if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			move_view(Vec2(0.0f, -0.5f));
		}

		// prep
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glScalef(0.1f, 0.1f, 0.1f);
		glColor3f(1.0f, 1.0f, 1.0f);

		// draw some rays
		for (unsigned int i = 0; i < RAYSFRAME; i++)
		{
			dir.x = cos(step * i);
			dir.y = sin(step * i);

			trace(root, pos, dir, 20.0f, d, geom, dest, tmp);

			glBegin(GL_LINES);
			{
				glVertex2f(0.0f, 0.0f);
				glVertex2f(d*dir.x, d*dir.y);
			}
			glEnd();
		}

		// swap
		glfwSwapBuffers();

		// next frame
		;
	}
}

/*
	main
*/
int main(int argc, char * argv[])
{
	// init
	glfwInit();
	glfwSwapInterval(1);// vsync
	glfwEnable(GLFW_STICKY_KEYS);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindow(WINW, WINH, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("pixellight");

	// loop
	game();

	// nuke
	glfwCloseWindow();

	// done
	return 0;
}
