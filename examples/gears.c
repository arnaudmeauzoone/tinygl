/* gears.c */

/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * Brian Paul
 */


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/gl.h> 
#include "ui.h"

#ifndef M_PI
#  define M_PI 3.14159265
#endif


/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 *         outer_radius - radius at center of teeth
 *         width - width of gear
 *         teeth - number of teeth
 *         tooth_depth - depth of tooth
 */
static void gear( GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
		  GLint teeth, GLfloat tooth_depth )
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat angle, da;
   GLfloat u, v, len;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth/2.0;
   r2 = outer_radius + tooth_depth/2.0;

   da = 2.0*M_PI / teeth / 4.0;

   glShadeModel( GL_FLAT );

   glNormal3f( 0.0, 0.0, 1.0 );

   /* draw front face */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
      glVertex3f( r1*cos(angle), r1*sin(angle), width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5 );
   }
   glEnd();

   /* draw front sides of teeth */
   glBegin( GL_QUADS );
   da = 2.0*M_PI / teeth / 4.0;
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle),      r1*sin(angle),      width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5 );
   }
   glEnd();


   glNormal3f( 0.0, 0.0, -1.0 );

   /* draw back face */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glVertex3f( r1*cos(angle), r1*sin(angle), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
   }
   glEnd();

   /* draw back sides of teeth */
   glBegin( GL_QUADS );
   da = 2.0*M_PI / teeth / 4.0;
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5 );
      glVertex3f( r1*cos(angle),      r1*sin(angle),      -width*0.5 );
   }
   glEnd();


   /* draw outward faces of teeth */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle),      r1*sin(angle),       width*0.5 );
      glVertex3f( r1*cos(angle),      r1*sin(angle),      -width*0.5 );
      u = r2*cos(angle+da) - r1*cos(angle);
      v = r2*sin(angle+da) - r1*sin(angle);
      len = sqrt( u*u + v*v );
      u /= len;
      v /= len;
      glNormal3f( v, -u, 0.0 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),    width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5 );
      glNormal3f( cos(angle), sin(angle), 0.0 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da),  width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5 );
      u = r1*cos(angle+3*da) - r2*cos(angle+2*da);
      v = r1*sin(angle+3*da) - r2*sin(angle+2*da);
      glNormal3f( v, -u, 0.0 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da),  width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glNormal3f( cos(angle), sin(angle), 0.0 );
   }

   glVertex3f( r1*cos(0), r1*sin(0), width*0.5 );
   glVertex3f( r1*cos(0), r1*sin(0), -width*0.5 );

   glEnd();


   glShadeModel( GL_SMOOTH );

   /* draw inside radius cylinder */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glNormal3f( -cos(angle), -sin(angle), 0.0 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
   }
   glEnd();
      
}


static GLfloat view_rotx=20.0, view_roty=30.0, view_rotz=0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static GLuint limit;
static GLuint count = 1;


void draw( void )
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef( view_rotx, 1.0, 0.0, 0.0 );
   glRotatef( view_roty, 0.0, 1.0, 0.0 );
   glRotatef( view_rotz, 0.0, 0.0, 1.0 );

   glPushMatrix();
   glTranslatef( -3.0, -2.0, 0.0 );
   glRotatef( angle, 0.0, 0.0, 1.0 );
   glCallList(gear1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef( 3.1, -2.0, 0.0 );
   glRotatef( -2.0*angle-9.0, 0.0, 0.0, 1.0 );
   glCallList(gear2);
   glPopMatrix();

   glPushMatrix();
   glTranslatef( -3.1, 4.2, 0.0 );
   glRotatef( -2.0*angle-25.0, 0.0, 0.0, 1.0 );
   glCallList(gear3);
   glPopMatrix();

   glPopMatrix();

   tkSwapBuffers();

   count++;
   if (count==limit) {
       exit(0);
   }
}



void idle( void )
{
   angle += 2.0;
   draw();
}

/* new window size or exposure */
void reshape( int width, int height )
{
   GLfloat  h = (GLfloat) height / (GLfloat) width;

   glViewport(0, 0, (GLint)width, (GLint)height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum( -1.0, 1.0, -h, h, 5.0, 60.0 );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -40.0 );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void init( void )
{
   static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0 };
   static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0 };
   static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0 };
   static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0 };

   glLightfv( GL_LIGHT0, GL_POSITION, pos );
   glEnable( GL_CULL_FACE );
   glEnable( GL_LIGHTING );
   glEnable( GL_LIGHT0 );
   glEnable( GL_DEPTH_TEST );

   /* make the gears */
   gear1 = glGenLists(1);
   glNewList(gear1, GL_COMPILE);
   glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red );
   gear( 1.0, 4.0, 1.0, 20, 0.7 );
   glEndList();

   gear2 = glGenLists(1);
   glNewList(gear2, GL_COMPILE);
   glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green );
   gear( 0.5, 2.0, 2.0, 10, 0.7 );
   glEndList();

   gear3 = glGenLists(1);
   glNewList(gear3, GL_COMPILE);
   glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue );
   gear( 1.3, 2.0, 0.5, 10, 0.7 );
   glEndList();

   glEnable( GL_NORMALIZE );
}

int main(int argc, char **argv) 
{
   if (argc>1) {
      /* do 'n' frames then exit */
      limit = atoi( argv[1] ) + 1;
   }
   else {
      limit = 0;
   }

   return ui_loop(argc, argv, "gears");
}


