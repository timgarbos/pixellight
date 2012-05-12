/* main.cpp

pixellight!

*/

#include <cmath>

// libc++
#include <iostream>
#include <map>

// GL
#include <GL/glfw.h>

// local
#include "Vec2.h"
#include "LevelNode.h"
#include "LevelEdge.h"
#include "Geom.h"
#include "LevelLoader.h"
#include "AudioManager.h"
#include "f3x5.h"

/*
	MACROS
*/
#define WINW		512
#define WINH		512
#define TEXW		512
#define TEXH		512

#define TX(x,y)		(texdata + (x) + (y)*TEXW)
#define FF(n)		(n & 0xff)
#define RGB(r,g,b)	(0xff000000 | (FF(b)<<16) | (FF(g)<<8) | (FF(r)))
#define RGBA(r,g,b,a)	((FF(a)<<24) | (FF(b)<<16) | (FF(g)<<8) | (FF(r)))
#define RANDRGB_		(RGB(rand()%256,rand()%256,rand()%256))
#define RANDRGB		(RGBA(rand()%256,rand()%256,rand()%256,120))
#define RANDRGB2_(r,g,b,mod)	(RGB(max(0,min(255,r+rand()%mod)),max(0,min(255,g+rand()%mod)),max(0,min(255,b+rand()%mod))))
#define RANDRGB2(r,g,b,mod)	(RGBA(max(0,min(255,r+rand()%mod)),max(0,min(255,g+rand()%mod)),max(0,min(255,b+rand()%mod)),120))

#define DT			0.01666667f
#define PI			3.14159265f

#define RAYSFRAME			3100
#define RAYSFRAMEDEV		1500

#define PARTICLESFRAME		1200
#define PARTICLESFRAMEDEV	1200

#define TRACEDEBUG	0

#define EDGE_S		0
#define EDGE_E		1
#define EDGE_N		2
#define EDGE_W		3
#define EDGE_NONE	4

#define NORM_N		1
#define NORM_W		2
#define NORM_S		3
#define NORM_E		4

#define PXPLIMIT	65535

int RaysPerFrame = RAYSFRAME;
int ParticlesPerFrame = PARTICLESFRAME;
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
	unsigned int	norm;
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
Vec2			vel;
unsigned int	ccw;

pxp_t *			pxpdata;
unsigned int *	pxppool;
unsigned int	pxppoolcursor;

unsigned int *	texdata;
unsigned int	texid;

float			texv[] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
float			texc[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

unsigned int	normpre	= 0;
int				lvlcurr	= 0;
int				lvlnext	= 0;

unsigned int	wait	= 0;
bool			died	= false;

float off_ground_ratio = 0.0f;
std::map<Geom *, Vec2> goals;

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

AudioManager *audioManager;

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
	int			tn	= 0;
	int			hn	= 0;
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
			if ((dir.y > 0.0f && rayline(pos, dir, g00, g10, ut, vt) && (tn = NORM_S) != 0) ||
				(dir.x < 0.0f && rayline(pos, dir, g10, g11, ut, vt) && (tn = NORM_E) != 0) ||
				(dir.y < 0.0f && rayline(pos, dir, g11, g01, ut, vt) && (tn = NORM_N) != 0) ||
				(dir.x > 0.0f && rayline(pos, dir, g01, g00, ut, vt) && (tn = NORM_W) != 0))
			{
				if (ut <= r && (ge == NULL || ut < um))
				{
					ge = gbase;
					hn = tn;
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

				// hack hack, norm norm
				if (ce->Node == NULL)
				{
					hn = co + 1;
				}

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
	out.norm	= hn;
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
	grav
*/
inline float grav(float d)
{
	//if (d < 0.0001f)
	//{
	//	return 0.0f;
	//}
	//else
	//{
	//	return 0.1f * std::pow(std::min(1.0f, d)-1.0f, 4);
	//}
	return 5.0f * std::pow(1.0f / (1.0f + 20.0f * d), 1);
}

/*
	fisr
*/
inline float fisr(float x)
{
	float h = 0.5f*x;
	unsigned int i = *(unsigned int*)&x;
	
	i = 0x5f3759df - (i >> 1);
	
	x = *(float*)&i;
	x = x*(1.5f - h*x*x);
	
	return x;
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
	int		gnum = 0;
	int		gcnt = goals.size();
	Vec2 *	gpos = new Vec2[gcnt];

	for (std::map<Geom *, Vec2>::iterator it = goals.begin(); it != goals.end(); it++)
	{
		gpos[gnum++] = it->second;
	}

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
				for (int j = 0; j < gcnt; j++)
				{
					float dx = p.xx - gpos[j].x;
					float dy = p.xy - gpos[j].y;

					float r = fisr(dx*dx + dy*dy);

					if (r > 10.0f)
					{
						p.vx -= dt * 5.0f * dx * r;
						p.vy -= dt * 5.0f * dy * r;
					}
				}

				p.xx += dt * p.vx;
				p.xy += dt * p.vy;
			}
		}
	}

	delete[] gpos;
}

/*
	pxp_plot
*/
void pxp_plot()
{
	int x;
	int y;

	float ex = static_cast<float>(TEXW>>1);
	float ey = static_cast<float>(TEXH>>1);

	memset(texdata, 0, (TEXW*TEXH)<<2);

	//#pragma omp parallel for
	int pixelSize = 1;

	for (int i = 0; i < PXPLIMIT; i++)
	{
		//pixelSize = i%120==0?2:1;
		//pixelSize = i%401==0?4:pixelSize;

		pxp const & p = pxpdata[i];
		
		if (p.ttl != 0)
		{
			x = static_cast<int>(p.xx * ex + ex);
			y = static_cast<int>(p.xy * ey + ey);
			
				for(int ix=x-pixelSize/2;ix<x+pixelSize;ix++)
				{
					for(int iy=y-pixelSize/2;iy<y+pixelSize;iy++)
					{
						if (ix >= 0 && ix <= TEXW-1 &&
								iy >= 0 && iy <= TEXH-1)
						{
							*TX(ix,iy) = p.color;
						}
					}
				}

			}
		}
	}


/*
	move_player
*/
void move_player()
{
	Vec2		acc;
	float		len;
	Vec2		dir;
	float		dd;
	traceres_t	trtmp;
	traceres_t	tr;

	// allow left/right acceleration
	if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS && normpre != NORM_E)
	{
		acc.x -= (normpre == NORM_N) ? 30.0f : 15.0f;
	}
	if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS && normpre != NORM_W)
	{
		acc.x += (normpre == NORM_N) ? 30.0f : 15.0f;
	}

	// if player hit ground in last move
	if (normpre == NORM_N)
	{
		// allow jump impulse
		if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
		{
			acc.y += (6.0f / DT);
		}
	}
	else
	{
		// apply some gravity
		acc.y -= 7.0f;
	}

	// velocity intergration step
	vel.x += DT * acc.x;
	vel.y += DT * acc.y;

	// apply some damping
	vel.x *= 0.9f;

	// trace prep!
	if ((len = std::sqrt(vel.x*vel.x + vel.y*vel.y)) == 0.0f)
	{
		return;
	}

	dir.x	= vel.x/len;
	dir.y	= vel.y/len;
	dd		= DT * len;

	rot90(dir, ccw);

	// trace execute!
	trace(root, pos, dir, dd, tr);

	///* don't have time to make this work before the deadline*/
	//// trace scout!
	//float x0 = pos.x-0.15f;
	//float x1 = pos.x+0.15f;
	//float y0 = pos.y-0.15f;
	//float y1 = pos.y+0.15f;

	//trtmp.d = dd;
	//trace(root, Vec2(x0, y0), dir, dd, tr);
	//if (tr.d != 0.0f && tr.d < trtmp.d)
	//{
	//	trtmp = tr;
	//}
	//trace(root, Vec2(x1, y0), dir, dd, tr);
	//if (tr.d != 0.0f && tr.d < trtmp.d)
	//{
	//	trtmp = tr;
	//}
	//trace(root, Vec2(x0, y1), dir, dd, tr);
	//if (tr.d != 0.0f && tr.d < trtmp.d)
	//{
	//	trtmp = tr;
	//}
	//trace(root, Vec2(x1, y1), dir, dd, tr);
	//if (tr.d != 0.0f && tr.d < trtmp.d)
	//{
	//	trtmp = tr;
	//}

	//// trace execute!
	//trace(root, pos, dir, trtmp.d, tr);
	//if (trtmp.d != 0.0f && tr.norm == 0 && trtmp.norm != 0)
	//{
	//	tr.norm = ((4 + (trtmp.norm-1) - ccw) % 4) + 1;
	//}
	//else
	//{
	//	tr.norm = 0;
	//}

	// did we change root?
	if (root != tr.node)
	{
        
        // in that case, space may have rotated
		ccw = (ccw + tr.ccw) % 4;
 
        // fade out
        if(root->audio < audioManager->channels.size())
            audioManager->channels[root->audio]->setVolumeTarget(0.0f, 1.0f/4.0f);

        // fade in
        if(tr.node->audio < audioManager->channels.size())
            audioManager->channels[tr.node->audio]->setVolumeTarget(0.5f, 1.0f/4.0f);
 	}

	// if player was obstructed, then respond to the contact
	if (tr.norm != 0)
	{
		// transform contact normal
		tr.norm = ((4 + (tr.norm-1) - ccw) % 4) + 1;

		// clip velocity by tangent
		switch (tr.norm)
		{
		case NORM_N:
		case NORM_S:
			vel.y = 0.0f;
			vel.x *= ((dd - tr.d) / dd);
			break;

		case NORM_W:
		case NORM_E:
			vel.x = 0.0f;
			vel.y *= ((dd - tr.d) / dd);
			break;
		}
	}

	// store contact normal for next move (may be zero)
	normpre = tr.norm;

	// move player
	root	= tr.node;
	pos.x	= tr.pos.x;
	pos.y	= tr.pos.y;

	// maybe we hit the goal or something bad
	if (tr.geom != NULL && tr.geom->isGoal)
	{
		died = false;
		wait = 120;

		lvlnext = (lvlcurr + 1) % 4;
	}
	else if ((tr.geom != NULL && tr.geom->isBad) || pos.x < -1.0f || pos.x > 1.0f || pos.y < -1.0f || pos.y > 1.0f)
	{
		died = true;
		wait = 120;

		//TODO: emit some death particles, maybe
	}

	// done
	;
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

	if (root != tr.node)
	{
		ccw = (ccw + tr.ccw) % 4;
	}

	root	= tr.node;
	pos.x	= tr.pos.x;
	pos.y	= tr.pos.y;
}

void capFramerate(double fps) {
    static double start = 0, diff, fwait;
    fwait = 1 / fps;
    diff = glfwGetTime() - start;
    if (diff < fwait) {
        glfwSleep(fwait - diff);
    }
    start = glfwGetTime();
}

/*
	game
*/
void game()
{
	float		frametime0;
	float		scale = 0.3f;
	float		theta = (2.0f * PI) / static_cast<float>(RaysPerFrame);
	float		particleTheta = (2.0f * PI) / static_cast<float>(PARTICLESFRAME);
	char		txt[100];
	Vec2		mov;
	Vec2		dir;
	traceres_t	tr;

	root	= LevelLoader::LoadXml(0);
	pos.x	= 0.0f;
	pos.y	= 0.0f;
	ccw		= 0;

    float frame_time_last = glfwGetTime();
    float frame_time;
    float delta_time;

    int frames_off_ground = 0;
    
	while (true)
	{
        // calc delta time.
        frame_time = glfwGetTime();
        delta_time = frame_time - frame_time_last;
        frame_time_last = frame_time;
        
        audioManager->update_channels(delta_time);
 
        if(normpre != NORM_N)
        {
            frames_off_ground++;
        }
        else 
        {
            frames_off_ground = 0;
        }
        
        off_ground_ratio += (frames_off_ground > 3 ? 1.0f : -0.5f) * 2.0f * delta_time; 
        off_ground_ratio = min(off_ground_ratio, 1.0f);
        off_ground_ratio = max(off_ground_ratio, 0.0f);
 
        audioManager->off_ground_ratio = off_ground_ratio;
        
        frame++;
		frametime0 = static_cast<float>(glfwGetTime());

        RaysPerFrame = RAYSFRAME+RAYSFRAMEDEV*sin(glfwGetTime()*5.0f);
		ParticlesPerFrame = PARTICLESFRAME+PARTICLESFRAMEDEV*sin(PI+glfwGetTime()*5.0f);
		theta = (2.0f * PI) / static_cast<float>(RaysPerFrame);
		particleTheta = (2.0f * PI) / static_cast<float>(ParticlesPerFrame);

		// clear goals
		goals.clear();

		// poll input
		glfwPollEvents();

		// handle input
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			break;
		}
		if (glfwGetKey('1') == GLFW_PRESS)
		{
			lvlnext = 0;
			wait	= 1;
		}
		if (glfwGetKey('2') == GLFW_PRESS)
		{
			lvlnext = 1;
			wait	= 1;
		}
		if (glfwGetKey('3') == GLFW_PRESS)
		{
			lvlnext = 2;
			wait	= 1;
		}
		if (glfwGetKey('4') == GLFW_PRESS)
		{
			lvlnext = 3;
			wait	= 1;
		}

		// prep draw
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0f, 1.0f, 1.0f);

		// waiting on level reset?
		if (wait > 0)
		{
			if (--wait == 0)
			{
				if (lvlnext != lvlcurr)
				{
					root	= LevelLoader::LoadXml(lvlnext);
					lvlcurr	= lvlnext;
				}

				pos.x	= 0;
				pos.y	= 0;
				normpre	= 0;
				died	= false;
				ccw		= 0;
			}
		}

		// move the player if not waiting
		if (wait == 0)
		{
			move_player();
		}

		// emit particles
		if (wait == 0)
		{
			for (unsigned int i = 0; i < ParticlesPerFrame; i++)
			{
				float	a = particleTheta*i + 0.6f*sin(10.0f*glfwGetTime());
				Geom *	g = NULL;

				dir.x = cos(a);
				dir.y = sin(a);

				trace(root, pos, dir, 10.0f, tr);
				rot90(dir, 4-ccw);

				float spd = 0.9f + 0.15f * (rand() % 100);
				float ttl = 1.0f + (60.0f/spd) * tr.d;

				pxp_emit(static_cast<unsigned int>(ttl), spd*scale*dir.x, spd*scale*dir.y, 0.0f, 0.0f, i%root->colorMod==0?RANDRGB2(root->colorR,root->colorG,root->colorB,80):RANDRGB);

				if ((g = tr.geom) != NULL && (g->colorR | g->colorG | g->colorB) != 0)
				{
					pxp_emit(60, spd*0.3f*dir.x, spd*0.3f*dir.y, scale*dir.x*tr.d, scale*dir.y*tr.d, RGB(g->colorR,g->colorG,g->colorB));

					/* gravity goals doesn't look good, scrapped
					std::map<Geom *, Vec2>::iterator it = goals.find(tr.geom);

					if (it == goals.end())
					{
						goals[tr.geom] = Vec2(scale*tr.d*dir.x, scale*tr.d*dir.y);
					}
					*/
				}
			}
		}

		// move particles
		pxp_step(DT);

		// plot particles
		pxp_plot();

		// read frametime
		frametime = static_cast<float>(glfwGetTime()) - frametime0;

		// plot some text
		sprintf(txt, "photon boy\n----------\nframe %d\nf/sec %d\n#free %d\n#live %d",
			frame, static_cast<unsigned int>(1.0f / frametime),
			PXPLIMIT-pxppoolcursor, pxppoolcursor);
		
		dstr(10, 10, txt, RGB(255,255,255));

		// enable additive blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

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
		
		// draw some rays
		if (wait == 0)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 0.005f);
			glLineWidth(25.0f);

			for (unsigned int i = 0; i < RaysPerFrame; i++)
			{
				dir.x = cos(theta * i);
				dir.y = sin(theta * i);

				trace(root, pos, dir, 15.0f, tr);

				rot90(dir, 4-ccw);

				glBegin(GL_LINES);
				{
					glVertex2f(0.0f, 0.0f);
					glVertex2f(scale*tr.d*dir.x, scale*tr.d*dir.y);
				}
				glEnd();
			}
		}

		// disable additive blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		// swap, gogogo!
		glfwSwapBuffers();
		
		// next frame
		capFramerate(60.0);
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
	glfwSetWindowTitle("Photon Boy 1e-7");
    
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

	// init audio
    audioManager = new AudioManager();
    
	// loop game
	game();

	// nuke audio
    delete audioManager;
    
	// nuke buffers
	delete[] texdata;
	delete[] pxpdata;
	delete[] pxppool;

	// nuke glfw
	glfwCloseWindow();
    
	// done
	return 0;
}
