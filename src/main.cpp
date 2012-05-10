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
#define RAYSFRAME	100

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
	swap
*/
template <typename T>
inline void swap(T & a, T & b)
{
	T tmp = a;
	a = b;
	b = tmp;
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
		swap(v.x, v.y);
		break;

	case 2:
		v.x *= -1.0f;
		v.y *= -1.0f;
		break;

	case 3:
		v.x *= -1.0f;
		swap(v.x, v.y);
		break;
	}
}

/*
	trace
*/
inline void trace(LevelNode * node, Vec2 pos, Vec2 dir, float dMax, float & dOut, Geom * & geom)
{
	LevelEdge *	e;
	Vec2		x;
	float		ut;	// distance along trace direction
	float		vt;	// distance along edge
	float		d = 0.0f;
	bool		f = false;

	// loop until the maximum distance has been reached, or no more nodes
	while ((d < dMax) && node != NULL)
	{
		// find nearest intersection with local geometry
		//TODO

		// if unsuccessful, then find intersection with node boundary
		if (!f)
		{
			// transfer to edge
			if (rayline(pos, dir, v00, v10, ut, vt))
			{
				e		= node->s;// => south
				pos.x	= ut*dir.x;
				pos.y	= -1.0f;
			}
			else if (rayline(pos, dir, v10, v11, ut, vt))
			{
				e		= node->e;// => east
				pos.x	= 1.0f;
				pos.y	= ut*dir.y;
			}
			else if (rayline(pos, dir, v11, v01, ut, vt))
			{
				e		= node->n;// => north
				pos.x	= ut*dir.x;
				pos.y	= 1.0f;
			}
			else if (rayline(pos, dir, v01, v00, ut, vt))
			{
				e		= node->w;// => west
				pos.x	= -1.0f;
				pos.y	= ut*dir.y;
			}

			// transform position
			rot90(pos, e->ccwSteps);

			if (e->ccwSteps % 2 == 0)
			{
				pos.x *= -1.0f;
			}
			else
			{
				pos.y *= -1.0f;
			}

			// transform direction
			rot90(dir, e->ccwSteps);

			// move to connecting node
			node = e->Node;
		}

		// move to next node
		//TODO

		// increment distance travelled
		d += ut;
	}

	// cap the output
	if (d > dMax)
	{
		dOut = dMax;
		geom = NULL;
	}
	else
	{
		dOut = d;
	}
}

/*
	game
*/
void game()
{
	while (true)
	{
		glfwPollEvents();

		// quit on escape
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			break;
		}

		// handle input
		//todo

		// trace some rays
		for (unsigned int i = 0; i < RAYSFRAME; i++)
		{

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