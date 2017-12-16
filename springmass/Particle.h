//
// Created by anikethjr on 24/11/17.
//
#ifndef CLOTH_SIMULATION_PARTICLE_H
#define CLOTH_SIMULATION_PARTICLE_H

#include <bits/stdc++.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace std;
using namespace glm;

#define DAMPING_FACTOR 0.01 // refers to the damping of the cloth for each frame
#define TIME_STEP 0.5 // refers to the timestep taken by each particle in each frame

/**
 * Class which is used to define the particles of the cloth
 */
class Particle {
    bool is_movable; // to check whether the particle is movable or not
    double mass; // defines the mass of the particle (default = 1)
    dvec3 current_pos; // defines the current position of the particle
    dvec3 old_pos; // defines the old position of the particle
    dvec3 acceleration; // defines the acceleration of the particle
    dvec3 normal; // defines the normal to the cloth at the position of the particle - used for shading

public:
    /**
     * Constructor to initialize the particle
     * @param current_pos position of the particle
     * @param mass mass of the particle
     */
    Particle(dvec3 current_pos, double mass) {
        is_movable = true;
        this->mass = mass;
        this->current_pos = current_pos;
        old_pos = current_pos;
        acceleration = dvec3(0, 0, 0);
        normal = dvec3(0, 0, 0);
    }

    /**
     * Update the normal by adding the given update vector
     * @param update update vector
     */
    void updateNormal(dvec3 update) {
        normal = normal + normalize(update);
    }

    /**
     * Resets the normal to 0
     */
    void resetNormal() {
        normal = dvec3(0, 0, 0);
    }

    /**
     * Makes the length of the normal 1
     */
    void normalizeNormal() {
        normal = normalize(normal);
    }

    /**
     * Function to apply a force vector on a particle (to change the acceleration of the particle)
     * @param force refers to the force vector to be applied on the particle
     */
    void applyForce(dvec3 force) {
        acceleration += force / (double) mass;
    }

    /**
     * Function to get the current position of the particle
     * @return returns the current position of the particle
     */
    dvec3 getCurrentPos() {
        return current_pos;
    }

    /**
     * Function to reset the value of the acceleration
     */
    void resetAcceleration() {
        acceleration = dvec3(0.0, 0.0, 0.0);
    }

    /**
     * Function to progress the time by one time step define by TIME_STEP
	 * Uses Verlet integration to find the new pos of the particle
     */
    void timeStep() {
        if (is_movable) {
            dvec3 temp = current_pos;
            current_pos = current_pos + (current_pos - old_pos) * (1.0 - DAMPING_FACTOR) +
                          acceleration * pow(TIME_STEP, 2); // gives the new position of the particle
            old_pos = temp;
            resetAcceleration();  // changing the position resets the acceleration of the particle
        }

    }

    /**
     * Function to make the particle immovable
     */
    void makeImmovable() {
        is_movable = false;
    }

    /**
     * Function to offset the position of a particle wrt given update vector
     * @param update refers to the update vector
     */
    void updatePosition(dvec3 update) {
        if (is_movable) {
            current_pos += update;
        }
    }

    /**
     * Returns the normal
     * @return the normal
     */
    dvec3 getNormal() {
        return normal;
    }
};

#endif //CLOTH_SIMULATION_PARTICLE_H
