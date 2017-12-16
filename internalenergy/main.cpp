/* 
 * File:   main.cpp
 * Author: tanmaya
 *
 * Created on 25 November, 2017, 9:59 AM
 */

#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "Camera.h"
#include "Cloth.h"
#define FPS 200

using namespace std;
using namespace glm;
/*
 * 
 */

int width = 500; //width of the window
int height = 500; //height of the window
GLdouble backColor[3] = {0.0, 0.0, 0.0}; //color of the background
GLdouble clothColor1[3] = {0.5, 0.5, 0.9};
GLdouble clothColor2[3] = {0.8, 0.2, 0.5};
Camera* cam;
Cloth* c;

void keyPress(unsigned char key,int x,int y)
{
    switch(key)
    {
        //Move forward
        case 'w':
            cam->move(dvec3(0,0,-0.2));
            break;
        //Move back
        case 's':
            cam->move(dvec3(0,0,0.2));
            break;
        //Move left
        case 'a':
            cam->move(dvec3(-0.2,0,0));
            break;
        //Move right
        case 'd':
            cam->move(dvec3(0.2,0,0));
            break;
        //Move up
        case 'r':
            cam->move(dvec3(0,0.2,0));
            break;
        //Move down
        case 'f':
            cam->move(dvec3(0,-0.2,0));
            break;
        //Look up
        case 'i':
            cam->roll(1.0);
            break;
        //Look down
        case 'k':
            cam->roll(-1.0);
            break;
        //Look right
        case 'l':            
            cam->pitch(-1.0);
            break;
        //Look left
        case 'j':            
            cam->pitch(1.0);
            break;
        //Rotate clockwise
        case 'z':
            cam->yaw(-1.0);
            break;
        //Rotate anti-clockwise
        case 'x':
            cam->yaw(1.0);
            break;
        default:
            return;
    }
    glutPostRedisplay();
}

void draw()
{
    glClear  (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3dv(clothColor1);
    glBegin(GL_TRIANGLES);
    for(auto x: c->triangles) //drawing each triangle individually
    {
        glNormal3dv(value_ptr(c->pointNorms[get<0>(x)]));
        glVertex3dv(value_ptr(c->points[get<0>(x)]));
        glNormal3dv(value_ptr(c->pointNorms[get<1>(x)]));
        glVertex3dv(value_ptr(c->points[get<1>(x)]));
        glNormal3dv(value_ptr(c->pointNorms[get<2>(x)]));
        glVertex3dv(value_ptr(c->points[get<2>(x)]));
    }
    glEnd();
    glutSwapBuffers();
}
void timer(int t)
{
    c->update();
    glutPostRedisplay();
    glutTimerFunc(1000/FPS, timer, 0);
}

void light()
{
    GLfloat mat_specular[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat mat_emission[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat light_position[] = {-1, -0.8, 0.0, 0.0};
    GLfloat light_ambient[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat light_diffuse[] = {0.6, 0.6, 0.6, 1.0};
    GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);	
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void initGlut()
{
    int x = 0;
    srand(time(NULL));
    glutInit(&x, nullptr);    
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(0,0);
    glutInitWindowSize(width, height);
    glutCreateWindow("Cloth");
    glEnable(GL_DEPTH_TEST);
    cam = new Camera(width, height);
    cam->to3D();
    light();
    c = new Cloth(10, 30);    
    glClearColor(backColor[0], backColor[1], backColor[2], 0);
    glutKeyboardFunc(keyPress);
    glutTimerFunc(1000/FPS, timer, 0);
    glutDisplayFunc(draw);
        
}

int main(int argc, char** argv) {
    
    initGlut();
    glutMainLoop();
    return 0;
}

