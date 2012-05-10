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
#define RAYSFRAME	512

/*
	globals
*/
class Node;
class Geom;
//Node *	root;
Vec2	pos;

/*
	rayline
*/
inline bool rayline(Vec2 const & u0, Vec2 const & ud, Vec2 const & v0, Vec2 const & v1, float & ut, float & vt)
{
	float vdx	= v1.x-v0.x;
	float vdy	= v1.y-v0.y;
	float d		= ud.y*vdx - ud.x*vdy;

	if (d != 0.0f)
	{
		float const r	= 1.0f / d;
		float const a	= r * (v0.y - u0.y);
		float const b	= r * (u0.x - v0.x);

		ut	= vdx*a + vdy*b;
		vt	= ud.x*a + ud.y*b;

		return (ut >= 0.0f && vt >= 0.0f && vt <= 1.0f);
	}
	else
	{
		return false;
	}

}

/*
	trace
*/
inline void trace(Node * node, Vec2 pos, Vec2 dir, float dMax, float & dOut, Geom * & geom)
{
	float d = 0.0f;
	float t;

	// loop until the maximum distance has been reached, or no more nodes
	while ((d < dMax) && node != NULL)
	{
		// find nearest intersection with local geometry
		//TODO

		// if unsuccessful, then find intersection with local boundary
		//TODO

		// move to next node
		//TODO

		// increment distance travelled
		d += t;
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
	glfwInit();
	glfwSwapInterval(1);// vsync
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindow(WINW, WINH, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("pixellight");

	game();

	glfwCloseWindow();

	return 0;
}