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
//#include "Node.h"
//#include "Edge.h"
//#include "Geom.h"

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
	trace
*/
bool trace(Node * node, Vec2 const & pos, Vec2 const & dir, float dMax, float * dOut, Geom ** geom)
{

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