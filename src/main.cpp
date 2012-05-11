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
#include "LevelLoader.h"
#include "f3x5.h"

/*
	MACROS
*/
#define WINW		512
#define WINH		512
#define TEXW		256
#define TEXH		256

#define TX(x,y)		(texdata + (x) + (y)*TEXW)
#define FF(n)		(n & 0xff)
#define RGB(r,g,b)	(0xff000000 | (FF(b)<<16) | (FF(g)<<8) | (FF(r)))
#define RANDRGB		(RGB(100+rand()%156,100+rand()%156,100+rand()%156))

#define DT			0.01666667f
#define PI			3.14159265f
#define RAYSFRAME	500
#define TRACEDEBUG	0

#define EDGE_S		0
#define EDGE_E		1
#define EDGE_N		2
#define EDGE_W		3
#define EDGE_NONE	4

#define PXPLIMIT	65535

/*
	typedefs
*/
typedef struct traceres
{
	LevelNode	*	node;
	Geom *			geom;
	Vec2			pos;
	Vec2			dir;
	float			d;
	unsigned int	ccw;
} traceres_t;

typedef struct pxp
{
	unsigned int	ttl;
	unsigned int	idx;
	float			vx;
	float			vy;
	float			xx;
	float			xy;
	unsigned int	color;
} pxp_t;

/*
	globals
*/
unsigned int	frame;
float			frametime;

Vec2			v00 = Vec2(-1.0f, -1.0f);	// lower left
Vec2			v10 = Vec2(1.0f, -1.0f);	// lower right
Vec2			v01 = Vec2(-1.0f, 1.0f);	// upper left
Vec2			v11 = Vec2(1.0f, 1.0f);		// upper right

LevelNode *		root;
Vec2			pos;
unsigned int	ccw;

pxp_t *			pxpdata;
unsigned int *	pxppool;
unsigned int	pxppoolcursor;

unsigned int *	texdata;
unsigned int	texid;

float			texv[] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
float			texc[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

/*
	dchr
*/
inline void dchr(unsigned int x0, unsigned int y0, unsigned char c, unsigned int color)
{
	unsigned short s = f3x5[f3x5t[c]];

	for (unsigned char y = 0; y < 5; y++)
	{
		for (unsigned char x = 0; x < 3; x++)
		{
			if ((s & 1) != 0)
			{
				*TX(x0+2-x, TEXH-1-y0-y) = color;
			}

			s >>= 1;
		}
	}
}

/*
	dstr
*/
void dstr(unsigned int x0, unsigned int y0, char const * s, unsigned int color)
{
	unsigned int	x = x0;
	unsigned int	y = y0;
	unsigned int	i = 0;
	unsigned int	r;
	unsigned char	c;

	while (true)
	{
		if ((c = s[i++]) == '\0')
		{
			break;
		}
		else
		{
			switch (c)
			{
			case ' ':
				x += 4;
				break;

			case '\n':
				x = x0;
				y += 6;
				break;

			case '\t':
				x += ((r = (x-x0)%16) == 0) ? 16 : r;
				break;

			default:
				dchr(x, y, c, color);
				x += 4;
			}
		}
	}
}

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

		return (ut >= 0.0f && vt >= 0.0f && vt <= 1.0f);
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
inline void trace(LevelNode * node, Vec2 pos, Vec2 dir, float dMax, traceres_t & out)
{
	Geom *		ge	= NULL;
	LevelEdge *	ce	= NULL;
	float		r	= dMax;
	int			c	= 0;

	float		ut;	// distance to intersection in trace direction
	float		vt;	// distance to intersection in edge
	float		sx;	// sign of x-component after moving node
	float		sy; // sign of y-component after moving node

	int			ci	= EDGE_NONE;
	int			co	= EDGE_NONE;
	int			ccw = 0;

#if TRACEDEBUG
	std::cout << "--- new trace" << std::endl;
#endif

	// hack hack
	if (pos.x == 1.0f)
	{
		pos.x -= static_cast<float>(1e-7);
	}
	else if (pos.x == -1.0f)
	{
		pos.x += static_cast<float>(1e-7);
	}

	if (pos.y == 1.0f)
	{
		pos.y -= static_cast<float>(1e-7);
	}
	else if (pos.y == -1.0f)
	{
		pos.y += static_cast<float>(1e-7);
	}

	// loop until the maximum distance has been reached, or no more nodes
	while (r > 0.0f && node != NULL)
	{
		float um = 0.0f;

#if TRACEDEBUG
		std::cout << "node " << node << std::endl;
#endif

		if (++c > 10)
		{
			break;
		}

		// find nearest intersection with local geometry
		for (std::list<GeomInstance *>::iterator it = node->objs.begin(); it != node->objs.end(); it++)
		{
			GeomInstance *	g		= *it;
			Vec2 const &	gpos	= g->pos;
			Geom *			gbase	= g->masterGeom;
			Vec2 const &	gext	= gbase->extends;

			//std::cout << "g pos = "<<gpos.x<<", " << gpos.y<< ", ext = "<<gext.x<<", " << gext.y<< std::endl;

			// we only care about region inside current node
			float x0 = std::max(gpos.x-gext.x, -1.0f);
			float x1 = std::min(gpos.x+gext.x, 1.0f);
			float y0 = std::max(gpos.y-gext.y, -1.0f);
			float y1 = std::min(gpos.y+gext.y, 1.0f);

			// assemble into vertices
			Vec2 g00(x0, y0);
			Vec2 g10(x1, y0);
			Vec2 g01(x0, y1);
			Vec2 g11(x1, y1);

			//std::cout << "geom ["<<x0<<","<<y0<<"] , ["<<x1<<","<<y1<<"]" << std::endl;

			// test faces
			if ((dir.y > 0.0f && rayline(pos, dir, g00, g10, ut, vt)) ||
				(dir.x < 0.0f && rayline(pos, dir, g10, g11, ut, vt)) ||
				(dir.y < 0.0f && rayline(pos, dir, g11, g01, ut, vt)) ||
				(dir.x > 0.0f && rayline(pos, dir, g01, g00, ut, vt)))
			{
				if (ut <= r && (ge == NULL || ut < um))
				{
					ge = gbase;
					um = ut;
				}
			}
		}

		// if an intersection was found with local geometry, then move there
		if (ge != NULL)
		{
			// ray transfer to intersection with geometry
			pos.x += (um - static_cast<float>(1e-4))*dir.x;
			pos.y += (um - static_cast<float>(1e-4))*dir.y;

			// decrement reach
			r -= um;

			// halt
			break;
		}
		else// if not intersecting local geometry, then move towards node boundary
		{
			// pick connecting edge
			if (ci != EDGE_S && rayline(pos, dir, v00, v10, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going south" << std::endl;
#endif
				// south
				ce = node->s;
				sx = 1.0f;
				sy = -1.0f;
				co = EDGE_S;
			}
			else if (ci != EDGE_E && rayline(pos, dir, v10, v11, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going east" << std::endl;
#endif
				// east
				ce = node->e;
				sx = -1.0f;
				sy = 1.0f;
				co = EDGE_E;
			}
			else if (ci != EDGE_N && rayline(pos, dir, v11, v01, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going north" << std::endl;
#endif
				// north
				ce = node->n;
				sx = 1.0f;
				sy = -1.0f;
				co = EDGE_N;
			}
			else if (ci != EDGE_W && rayline(pos, dir, v01, v00, ut, vt))
			{
#if TRACEDEBUG
				std::cout << "going west" << std::endl;
#endif
				// west
				ce = node->w;
				sx = -1.0f;
				sy = 1.0f;
				co = EDGE_W;
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

			// test if we reached node boundary
			if (ut > r)
			{
				// ray transfer to point of reach
				pos.x += r*dir.x;
				pos.y += r*dir.y;

				// exhaust reach
				r = 0.0f;
			}
			else
			{
				// ray transfer to intersection with node boundary
				pos.x += ut*dir.x;
				pos.y += ut*dir.y;

				// decrement reach
				r -= ut;

				// check for connecting node
				if (ce->Node != NULL)
				{
					// update edge-in
					switch (ce->ccwSteps)
					{
					case 0:
						ci = (2 + co) % 4;
						break;
					
					case 1:
						ci = (3 + co) % 4;
						break;

					case 2:
						ci = co;
						break;

					case 3:
						ci = (1 + co) % 4;
						break;
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

					// transform direction
					rot90(dir, ce->ccwSteps);

					// move to connecting node
					node = ce->Node;

					// accumulate ccw steps
					ccw += ce->ccwSteps;
				}
				else
				{
					// halt if no connecting node
					break;
				}
			}

			// crossed node boundary
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
	out.ccw		= ccw % 4;

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
	pxp_emit
*/
void pxp_emit(unsigned int ttl, float vx, float vy, float xx, float xy, unsigned int color)
{
	if (ttl != 0 && pxppoolcursor < PXPLIMIT)
	{
		pxp & p = pxpdata[pxppool[pxppoolcursor++]];

		p.ttl	= ttl;
		p.vx	= vx;
		p.vy	= vy;
		p.xx	= xx;
		p.xy	= xy;
		p.color	= color;
	}
}

/*
	pxp_step
*/
void pxp_step(float dt)
{
	//#pragma omp parallel for
	for (int i = 0; i < PXPLIMIT; i++)
	{
		pxp & p = pxpdata[i];
			
		if (p.ttl != 0)
		{
			if ((--p.ttl) == 0)
			{
				pxppool[--pxppoolcursor] = p.idx;
			}
			else
			{
				p.xx += dt * p.vx;
				p.xy += dt * p.vy;
			}
		}
	}
}

/*
	pxp_plot
*/
void pxp_plot()
{
	int		x;
	int		y;

	float	ex = static_cast<float>(TEXW>>1);
	float	ey = static_cast<float>(TEXH>>1);

	memset(texdata, 0, (TEXW*TEXH)<<2);

	//#pragma omp parallel for
	for (int i = 0; i < PXPLIMIT; i++)
	{
		pxp const & p = pxpdata[i];
		
		if (p.ttl != 0)
		{
			x = static_cast<int>(p.xx * ex + ex);
			y = static_cast<int>(p.xy * ey + ey);
			
			if (x >= 0 && x <= TEXW-1 &&
				y >= 0 && y <= TEXH-1)
			{
				*TX(x,y) = p.color;
			}
		}
	}
}

/*
	move_view
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
	ccw		= (ccw + tr.ccw) % 4;
}

Vec2	jumpVel;
/*
	game
*/
void game()
{
	float		frametime0;
	float		scale = 0.3f;
	float		theta = (2.0f * PI) / static_cast<float>(RAYSFRAME);
	char		txt[100];
	Vec2		mov;
	Vec2		dir;
	traceres_t	tr;

	root	= LevelLoader::LoadXml(0);
	pos.x	= 0.0f;
	pos.y	= 0.0f;
	ccw		= 0;

	while (true)
	{
		frame++;
		frametime0 = static_cast<float>(glfwGetTime());

		// poll input
		glfwPollEvents();

		// quit on escape
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			break;
		}

		// handle input
		mov.x = 0.0f;
		mov.y = 0.0f;

		if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			mov.x -= 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			mov.x += 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
		{
			mov.y += 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			mov.y -= 0.01f;
		}

		

		if(jumpVel.y>=0)
		{
			mov.y +=0.01f;
			jumpVel.y -= 0.01f;
			
			
		}
		else
		{
			mov.y -=0.01f;
		}
		if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS && jumpVel.y<=0)
		{
			jumpVel.y = 1.0f;
		}

		if (mov.x != 0.0f || mov.y != 0.0f)
		{
			move_view(mov);
		}

		// prep
		glClear(GL_COLOR_BUFFER_BIT);

		// move particles
		pxp_step(DT);

		// emit particles
		for (unsigned int i = 0; i < RAYSFRAME; i++)
		{
			float a = theta*i + 0.6f*sin(15.0f*glfwGetTime());

			dir.x = cos(a);
			dir.y = sin(a);

			trace(root, pos, dir, 15.0f, tr);
			rot90(dir, ccw);

			float vel = 0.8f + 0.1f * (rand() % 50);
			float ttl = (60.0f/vel) * tr.d;

			pxp_emit(ttl, vel*scale*dir.x, vel*scale*dir.y, 0.0f, 0.0f, RANDRGB);
		}

		// plot particles
		pxp_plot();

		// read frametime
		frametime = static_cast<float>(glfwGetTime()) - frametime0;

		// plot some text
		sprintf(txt, "frame %d\nf/sec %d\n#free %d\n#live %d", frame, static_cast<unsigned int>(1.0f / frametime),
			PXPLIMIT-pxppoolcursor, pxppoolcursor);
		
		dstr(10, 10, txt, RGB(255,255,255));

		// draw plot
		glEnable(GL_TEXTURE_2D);
		{
			glBindTexture(GL_TEXTURE_2D, texid);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXW, TEXH, GL_RGBA, GL_UNSIGNED_BYTE, texdata);

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, texv);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, texc);

			glDrawArrays(GL_QUADS, 0, 4);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDisable(GL_TEXTURE_2D);

		/*
		// draw some rays
		for (unsigned int i = 0; i < RAYSFRAME; i++)
		{
			dir.x = cos(theta * i);
			dir.y = sin(theta * i);

			trace(root, pos, dir, 15.0f, tr);

			rot90(dir, ccw);

			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_LINES);
			{
				glVertex2f(0.0f, 0.0f);
				glVertex2f(scale*tr.d*dir.x, scale*tr.d*dir.y);
			}
			glEnd();
		}
		*/

		/*
		// blend in some debug stuff
		glPushMatrix();
		{
			glScalef(scale, scale, 1.0f);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			root->Draw(pos);
			glDisable(GL_BLEND);
		}
		glPopMatrix();
		*/

		/*
		// draw an avatar
		glColor3f(1.0f, 0.0f, 0.0f);
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		{
			glVertex2f(0.0f, 0.0f);
		}
		glEnd();
		*/

		
		// wait a bit if not in sync
		/*while (glfwGetTime() - frametime0 < DT - 0.00001f)
		{
			glfwSleep(0.0001f);// sleep 0.1ms
		}*/

		// swap
		glfwSwapBuffers();


		// next frame
		;
	}
}


void editor()
{
	float		step = (2.0f * 3.14f) / static_cast<float>(RAYSFRAME);
	Vec2		dir;
	Vec2		jumpVel;
	char		str[100];
	traceres_t	tr;

	root = LevelLoader::LoadXml(0);
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
			move_view(Vec2(-0.02f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			move_view(Vec2(0.02f, 0.0f));
		}
		else if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			move_view(Vec2(0.0f, -0.02f));
		}
		if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
		{
			jumpVel.y = 100.0f;
		}
		if(jumpVel.y>=0)
		{
			move_view(Vec2(0.0f, 0.02f));
			jumpVel.y -= 0.02f;
		}
		else
		{
			move_view(Vec2(0.0f, -0.02f));
		}

		// prep
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glScalef(0.3f, 0.3f, 0.3f);
		glColor3f(1.0f, 1.0f, 1.0f);

		root->Draw(pos);

		// draw some rays
		/*for (unsigned int i = 0; i < RAYSFRAME; i++)
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
		}*/

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
	// init glfw
	glfwInit();
	glfwSwapInterval(1);// vsync
	glfwEnable(GLFW_STICKY_KEYS);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindow(WINW, WINH, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("pixellight");

	// init buffers
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXW, TEXH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	texdata	= new unsigned int[TEXW*TEXH];
	pxpdata	= new pxp_t[PXPLIMIT];
	pxppool	= new unsigned int[PXPLIMIT];

	//#pragma omp parallel for
	for (int i = 0; i < PXPLIMIT; i++)
	{
		pxpdata[i].ttl	= 0;
		pxpdata[i].idx	= i;
		pxppool[i]		= i;
	}

	pxppoolcursor = 0;

	// loop
	game();
	//editor();

	// nuke buffers
	delete[] texdata;
	delete[] pxpdata;
	delete[] pxppool;

	// nuke glfw
	glfwCloseWindow();

	// done
	return 0;
}
