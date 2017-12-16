/* 
 * File:   Cloth.h
 * Author: tanmaya
 *
 * Created on 25 November, 2017, 12:58 PM
 */

#ifndef CLOTH_H
#define CLOTH_H
#define PI 3.14159265
#include <bits/stdc++.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <cstdlib>

using namespace std;
using namespace glm;
typedef dvec3 point;
typedef dvec2 UVpoint;
typedef tuple<int, int, int> triangle;

#define GRAVITY 0.000002
#define DEL 0.0001
#define STRX 1.0
#define STRY 1.0
#define KSTRX 0.6
#define KSTRY 0.6
#define KSH 0.01
#define KBEND 0.01
#define KDAMP 0.1
#define MAX_BEND 0.0000001
#define MAX_BEND_DAMP 0.0000001
#define MAX_SHEAR 0.01
#define MAX_SHEAR_DAMP 0.01
#define MAX_STRETCH 0.01
#define MAX_STRETCH_DAMP 0.01

class Cloth
{
public:
    int numX; //resolution on the X axis
    int numY; //resolution on the Y axis
    double mass = 20; //mass of the entire cloth
    double imass; //inverse of the mass per particle
    vector<point> points; //all the points of the cloth
    vector<UVpoint> uvpoints; //the uv coordinates of all the points
    vector<triangle> triangles; //all the triangles as tuples
    vector<dvec3> pointNorms; //normals of all points
    vector<dvec3> triNorms; //normals of all triangles (required for bending)
    vector<dvec3> forces; //all the forces calculated for each point
    vector<dvec3> velocities; //the velocities for each point
    vector<bool> movable; //whether the point is movable or not
    double UVarea; //the area of the UV triangle
    /**
     * Constructor. Generates a cloth of the given resolution
     * @param X The resolution on the X axis
     * @param Y The resolution on the Y axis
     */
    Cloth(int X, int Y);
    /**
     * updates all the points, forces, velocities and normals
     */
    void update();
    /**
     * Integrates the calculated forces
     */
    void integrate();
    /**
     * Makes both the poin and the triangle normals
     */
    void makeNorms();
    /**
     * Gets the W_u and W_v for the triangle
     * @param t The triangle for which the grads are required
     * @return A pair of the two vectors
     */
    pair<dvec3, dvec3> getWUV(triangle t);
    /**
     * Adds the stretch forces due to the X axis, as well as the damping components
     * @param str The stretchiness for X axis
     */
    void addStretchXForces(double str);
    /**
     * Adds the stretch forces due to the Y axis, as well as the damping components
     * @param str The stretchiness for Y axis
     */
    void addStretchYForces(double str);
    /**
     * Adds the shear forces as well as the damping components
     */
    void addShearForces();
    /**
     * Adds the bending forces as well as the damping components
     */
    void addBendForces();
    /**
     * Perturbs each particle by a small amount
     */
    void perturb();
    /**
     * Checks if the two triangles are in bounds and then adds their bending components
     * @param t1 Triangle 1 index
     * @param t2 Triangle 2 index
     */
    void checkAndBend(int t1, int t2);
    /**
     * Changes the configuration of the entire system.
     * @param points The new point locations
     * @param velocities The new velocities
     * @param movable The new movable vector
     * @param pert Whether to perturb after applying the new config or not
     */
    void changeState(vector<point> points, vector<dvec3> velocities, vector<bool> movable, bool pert);
    /**
     * Given a triangle and two points, returns the index of the remaning point
     * @param shared A pair of two points
     * @param t The index of the triangle
     * @return The index(in the points vector) of the remaining point
     */
    int getRemaining(pair<int, int> shared, int t);
    /**
     * Returns the normal of a triangle
     * @param t The triangle
     * @return The normal vector
     */
    dvec3 getNormTriangle(triangle t);
    /**
     * Calculates the Stretch condition along the X axis
     * @param t The triangle
     * @param stretchiness The stretchiness along X
     * @return The value of the condition along X
     */
    double condStretchX(triangle t, double stretchiness);
    /**
     * Calculates the Stretch condition along the YX axis
     * @param t The triangle
     * @param stretchiness The stretchiness along Y
     * @return The value of the condition along Y
     */
    double condStretchY(triangle t, double stretchiness);
    /**
     * Calculates the Shear condition
     * @param t The triangle
     * @return The shear condition value
     */
    double condShear(triangle t);
    /**
     * Calculates the bend condition between the two given triangles
     * @param t1 The index of the first triangle
     * @param t2 The index of the second triangle
     * @return The bend condition value
     */
    double condBend(int t1, int t2);
    /**
     * Calculates the shared edge between two adjacent triangles
     * @param t1 The first triangle index
     * @param t2 The second triangle index
     * @return A pair of point indices representing the shared edge
     */
    pair<int, int> sharedEdge(int t1, int t2);
    /**
     * Calculates the derivative of the StretchX condition
     * @param t The triangle
     * @param stretchiness The stretchiness value in the X direction
     * @return A tuple of vectors for derivatives wrt all points involved
     */
    tuple<dvec3, dvec3, dvec3> derivativeStretchX(triangle t, double stretchiness);
    /**
     * Calculates the derivative of the StretchY condition
     * @param t The triangle
     * @param stretchiness The stretchiness value in the Y direction
     * @return A tuple of vectors for derivatives wrt all points involved
     */
    tuple<dvec3, dvec3, dvec3> derivativeStretchY(triangle t, double stretchiness);
    /**
     * Calculates the derivative of the Shear condition
     * @param t The triangle
     * @return A tuple of vectors for derivatives wrt all points involved
     */
    tuple<dvec3, dvec3, dvec3> derivativeShear(triangle t);
    /**
     * Calculates the derivative of the Bend condition for all points involved
     * @param t1 The first triangle index
     * @param t2 The second triangle index
     * @return A tuple of vectors for derivatives wrt all points involved (first all the three points of t1, then the remaining one)
     */
    tuple<dvec3, dvec3, dvec3, dvec3>derivativeBend(int t1, int t2);    
    
};

#endif /* CLOTH_H */

