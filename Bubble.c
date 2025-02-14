

#define F_ERR 1e-9

#include "grid/quadtree.h"
#include "axi.h"
#include "navier-stokes/centered.h"
#include "two-phase.h"
#include "tension.h"
#include "curvature.h"
#include "solver/interface/adapt_wavelet_leave_interface.h"

// htg head file
#include "output_htg.h"

/*
  Two-phase system properties
*/

// Reference O2
#define RHOR 710.43 // density ratio
#define MUR 71.9178 // viscosity ratio

// #define TEND 5.0 //End time
#define Rb 10e-5 // initial bubble1 Radius in "m" unit
#define relax_tt 0     // 1: switch on the relaxation time
#define laplace_term 0 // 1 switch on the effect of laplace pressure on saturation concentration

double WIDTH = 15. * Rb;
double TEND = 1.;

int LEVEL = 9; // max. refinement level

int main()
{
  size(WIDTH); // length of domain;
  init_grid(32);

  rho1 = 1000.;       // liquid phase
  rho2 = rho1 / RHOR; // gas phase
  mu1 = 1.05e-3;      // liquid phase
  mu2 = mu1 / MUR;    // gas phase
  f.sigma = 0.072;

  // solution setup
  TOLERANCE = 1e-7;
  DT = 1e-8;
  CFL = 0.5;
  NITERMAX = 500;
  run();
}

/* in htg file the domain is like this
                right
      -------------------------
      |                       |
      |                       |
      |                       |
      |                       |
bottom|                       |top
 axi  |                       |
      |                       |
      |                       |
      |                       |
      |                       |
      -------------------------
                left

// Bcs, allow outflow at right and top
u.n[right] = neumann(0.); // zeroGradient of normal velocity
u.n[top] = neumann(0.);
// u.t[right]  = dirichlet(0.);
// u.n[right]  = dirichlet(0.);
p[right] = dirichlet(0.);                       // zeroPressre at cell center
pf[right] = dirichlet(0.);                      // zeroPressure at cell face (?)


/**
  We initialize the bubble with a radius $r=0.5$ and set the initial concentration in the liquid domain (according to the saturation ratio $\zeta$).
*/

//**************2D coordinates**************//
// x_simu =y;
// y_simu =x;
//******************************************//

event init (t=0){
  scalar f1[],f2[];
  refine(level<LEVEL && sq(x - 10.0*Rb)+sq(y)<sq(4.5 * Rb)&&sq(x)+sq(y)>sq(3.5 * Rb));
  refine(level<LEVEL && sq(x - 4.0*Rb)+sq(y)<sq(2.5 * Rb)&&sq(x - 4.0*Rb)+sq(y)>sq(1.5 * Rb));
  fraction (f1,sq(4.0 * Rb)-sq(x - 10.0*Rb)-sq(y));
  fraction (f2,sq(2.0 * Rb)-sq(x - 4.0*Rb)-sq(y));
  foreach()
  {
	f[]=0.;
    f[]=1-(f1[]+f2[]);
  }
  boundary(all);
}

event acceleration(i++)
{
  face vector av = a;
  foreach_face(x)
      av.x[] -= 9.81; //gravity closed here
      // av.x[] -= 0.0;
}


event stability (i++) 
{
      DT = t < 1.0 ? 1e-9 : 1e-8; //this helps for the stability
}

//event interface(t = 0; t <= TEND; t = t +1e-6)
event interface(i = i + 100)
{
  // output_facets (f, stderr);
  // The Following Code is important for parallel calculation!
  // Otherwise only get interface coordinate calculated by one of the cores
  char names[80];
  sprintf(names, "interface%d", pid());
  FILE *fp = fopen(names, "w");
  output_facets(f, fp);
  fclose(fp);
  char command[80];
  sprintf(command, "LC_ALL=C cat interfa* > facets/facets-%.6f", t);
  system(command);
}


event adapt(i++)
{
  double uemax = 1e-2;
  adapt_wavelet_leave_interface({u.x, u.y}, {f}, (double[]){ uemax, uemax, 1e-3}, LEVEL, 5, 1);
}

//event snapshot1(t = 0; t <= TEND; t = t + 1e-6)
event snapshot1(i = i + 100)
{
  char name[80];
  sprintf(name, "dump/dump-%.6f", t);
  dump(file = name);
}

//event snapshot2(t = 0; t <= TEND; t = t + 1e-6)
event snapshot2(i = i + 100)
{
  char path[] = "htg"; // no slash at the end!!
  char prefix[80];

  sprintf(prefix, "data-%.6f", t);
  output_htg((scalar *){f,u.x, u.y, p}, (vector *){uf}, path, prefix, i, t);
}

event stop_simulation(t = TEND)
{
  return 1;
}

