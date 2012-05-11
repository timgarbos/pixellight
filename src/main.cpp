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
#define RAYSFRAME	1000
#define TRACEDEBUG	0

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

		ut = vdx*a + vdy*b;
		vt = ud.x*a + ud.y*b;

		return (ut > 0.0f && vt >= 0.0f && vt <= 1.0f);
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
	traceres_t
*/
typedef struct traceres
{
	LevelNode	*	node;
	Geom *			geom;
	Vec2			pos;
	Vec2			dir;
	float			d;
} traceres_t;

/*
	trace
*/
inline void trace(LevelNode * node, Vec2 pos, Vec2 dir, float dMax, traceres_t & out)
{
	LevelEdge *	ce	= NULL;
	Geom *		ge	= NULL;
	float		r	= dMax;
	bool		f	= false;
	int			c	= 0;

	float		ut;	// distance to intersection in trace direction
	float		vt;	// distance to intersection in edge
	float		sx;	// sign of x-component after moving node
	float		sy; // sign of y-component after moving node

#if TRACEDEBUG
	std::cout << "--- new trace" << std::endl;
#endif

	// loop until the maximum distance has been reached, or no more nodes
	while (r > 0.0f && node != NULL)
	{
#if TRACEDEBUG
		std::cout << "node " << node << std::endl;
#endif

		// find nearest intersection with local geometry
		//TODO

		if (++c > 10)
		{
			break;
		}

		// if unsuccessful, then move towards node boundary
		if (!f)
		{
			// pick connecting edge
			if (rayline(pos, dir, v00, v10, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going south" << std::endl;
#endif
				// south
				ce = node->s;
				sx = 1.0f;
				sy = -1.0f;
			}
			else if (rayline(pos, dir, v10, v11, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going east" << std::endl;
#endif
				// east
				ce = node->e;
				sx = -1.0f;
				sy = 1.0f;
			}
			else if (rayline(pos, dir, v11, v01, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going north" << std::endl;
#endif
				// north
				ce = node->n;
				sx = 1.0f;
				sy = -1.0f;
			}
			else if (rayline(pos, dir, v01, v00, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going west" << std::endl;
#endif
				// west
				ce = node->w;
				sx = -1.0f;
				sy = 1.0f;
			}
			else
			{
#if TRACEDEBUG
				std::cout << "going nowhere" << std::endl;
#endif
				// halt if trying to cross edge from whence we came
				break;
			}

#if TRACEDEBUG
			std::cout << "   r   = " << r << std::endl;
			std::cout << "   ut  = " << ut << std::endl;
			std::cout << "   pos = " << pos.x << ", " << pos.y << std::endl;
			std::cout << "   dir = " << dir.x << ", " << dir.y << std::endl;
#endif

			// ray transfer
			if (ut > r)
			{
				// did not reach edge
				pos.x = pos.x + r*dir.x;
				pos.y = pos.y + r*dir.y;
			}
			else
			{
				// reached edge
				pos.x = pos.x + ut*dir.x;
				pos.y = pos.y + ut*dir.y;

				// decrement reach
				r -= ut;

				// check for connecting node
				if (ce->Node != NULL)
				{
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

					// transform direction
					rot90(dir, ce->ccwSteps);

					// move to connecting node
					node = ce->Node;
				}
				else
				{
					// halt if no connecting node
					break;
				}
			}

			// moved towards node boundary
			;
		}

		// next node
		;
	}

	// write result
	out.node	= node;
	out.geom	= ge;
	out.pos		= pos;
	out.dir		= dir;
	out.d		= dMax - r;

	// done
	;
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
	float		len = std::sqrt(v.x*v.x + v.y*v.y);
	Vec2		dir = Vec2(v.x / len, v.y / len);
	traceres_t	tr;

	trace(root, pos, dir, len, tr);

	root	= tr.node;
	pos.x	= tr.pos.x;
	pos.y	= tr.pos.y;

	std::cout << "pos = " << tr.pos.x << ", " << tr.pos.y << std::endl;
}

/*
	game
*/
void game()
{
	float		step = (2.0f * 3.14f) / static_cast<float>(RAYSFRAME);
	Vec2		dir;
	traceres_t	tr;

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
			move_view(Vec2(-0.01f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			move_view(Vec2(0.01f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
		{
			move_view(Vec2(0.0f, 0.01f));
		}
		else if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			move_view(Vec2(0.0f, -0.01f));
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

			trace(root, pos, dir, 20.0f, tr);

			glBegin(GL_LINES);
			{
				glVertex2f(0.0f, 0.0f);
				glVertex2f(tr.d*dir.x, tr.d*dir.y);
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
