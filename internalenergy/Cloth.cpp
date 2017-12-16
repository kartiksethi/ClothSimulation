/* 
 * File:   Cloth.cpp
 * Author: tanmaya
 *
 * Created on 25 November, 2017, 12:58 PM
 */
#include "Cloth.h"
/**
 * Generates a random number between 0.0 and 1.0
 * @return The generated random number
 */
double uniformRandom()
{
    return rand()/(double)RAND_MAX;
}
/**
 * Clamps each dimension of a dvec3 to a range of -lim to +lim
 * @param x The dvec3
 * @param lim The clamping value
 * @return The clamped vector
 */
dvec3 clamp(dvec3 x, double lim)
{
    dvec3 result(0.0);
    result[0] = std::max(std::min(x[0], lim), -lim);//clamping each dimension
    result[1] = std::max(std::min(x[1], lim), -lim);
    result[2] = std::max(std::min(x[2], lim), -lim);
    return result;
}

Cloth::Cloth(int X, int Y)
{
    numX = X;
    numY = Y;
    UVarea = 1.0/(2*(X - 1)*(Y - 1)); //area of a triangle
    imass = (X*Y) / (double)mass;
    points.reserve(X*Y); //reserving memory for each buffer
    uvpoints.reserve(X*Y);
    velocities.reserve(X*Y);
    forces.reserve(X*Y);
    pointNorms.reserve(X*Y);
    movable.reserve(X*Y);
    triangles.reserve(2*(X - 1)*(Y - 1));
    triNorms.reserve(2*(X - 1)*(Y - 1));
    for(int i = 0; i < X*Y; i++) //init all points
    {
        uvpoints.push_back(UVpoint((i%X)/((double)X - 1), (i/X) / ((double)Y - 1)));
        points.push_back(point((i%X)/((double)X - 1), (i/X)/((double)Y - 1), 0.0));
        velocities.push_back(dvec3(0.0, 0.0, 0.0));
        forces.push_back(dvec3(0.0, 0.0, 0.0));
        pointNorms.push_back(dvec3(0.0, 0.0, 0.0));
        movable.push_back(true);
    }
    movable[movable.size() - 1] = false;
    movable[movable.size() - numX] = false;
    for(int i = 0; i < 2*(X - 1)*(Y -1); i++) //init all triangles
    {
        int x = (i / 2)%(X - 1);
        int y = (i / 2)/(X - 1);
        if(i % 2 == 0)
        {
            triangles.push_back(make_tuple(x + y*X, x + 1 + (y + 1)*X, x + (y + 1)*X));
        }
        else
        {
            triangles.push_back(make_tuple(x + y*X, x + 1 + y*X, x + 1 + (y + 1)*X));
        }
        triNorms.push_back(dvec3(0.0, 0.0, 0.0));
    }
    perturb();
    makeNorms();
    
}

void Cloth::changeState(vector<point> points, vector<dvec3> velocities, vector<bool> movable, bool pert)
{
    this->points = points;
    this->velocities = velocities;
    this->movable = movable;
    if(pert) //perturb if required
        perturb();
    makeNorms();
}

void Cloth::perturb()
{
    for(int i = 0; i < points.size() - numX; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            if(movable[i])
                points[i][j] += uniformRandom() / 50; //perturbing by a small random amount
        }
    }
}

void Cloth::integrate() 
{
    for(int i = 0; i < forces.size(); i++)
    {
        for(int j = 0; j < 3; j++)
            velocities[i][j] += forces[i][j] * imass;
        velocities[i][1] -= GRAVITY;
        
        
    }
    for(int i = 0; i < points.size(); i++)
    {
        for(int j = 0; j < 3; j++)
        {
            if(movable[i])
                points[i][j] += velocities[i][j];
        }
    }
}

int Cloth::getRemaining(pair<int, int> shared, int t)
{
    if(get<0>(triangles[t]) != shared.first && get<0>(triangles[t]) != shared.second)
        return get<0>(triangles[t]);
    else if(get<1>(triangles[t]) != shared.first && get<1>(triangles[t]) != shared.second)
        return get<1>(triangles[t]);
    else
        return get<2>(triangles[t]);
}

pair<dvec3, dvec3> Cloth::getWUV(triangle t)
{
    auto uvp0 = uvpoints[get<0>(t)];
    auto uvp1 = uvpoints[get<1>(t)];
    auto uvp2 = uvpoints[get<2>(t)];
    auto p0 = points[get<0>(t)];
    auto p1 = points[get<1>(t)];
    auto p2 = points[get<2>(t)];
    auto dUV1 = uvp1 - uvp0;
    auto dUV2 = uvp2 - uvp0;
    auto dP1 = p1 - p0;
    auto dP2 = p2 - p0;
    double delta = dUV1[0]*dUV2[1] - dUV2[0]*dUV1[1]; //the discriminant for the UV matrix
    dvec3 Wu, Wv;
    for(int i = 0; i < 3; i++) //calculating the WUV
    {
        Wu[i] = (dP1[i]*dUV2[1] - dP2[i]*dUV1[1])/delta;
        Wv[i] = (-dP1[i]*dUV2[0] + dP2[i]*dUV1[0])/delta;
    }
    return make_pair(Wu, Wv);    
}

pair<int, int> Cloth::sharedEdge(int t1, int t2)
{
    if(t2 - t1 == 1)
        return make_pair(get<1>(triangles[t1]), get<0>(triangles[t1]));
    else if(t2 - t1 == -1)
        return make_pair(get<0>(triangles[t1]), get<2>(triangles[t1]));
    else
        return make_pair(get<2>(triangles[t1]), get<1>(triangles[t1]));
}

double Cloth::condStretchX(triangle t, double stretchiness)
{
    auto wuv = getWUV(t);
    return UVarea * (length(wuv.first) - stretchiness); //scaling the condition by the area
}

double Cloth::condStretchY(triangle t, double stretchiness)
{
    auto wuv = getWUV(t);
    return UVarea * (length(wuv.second) - stretchiness); //scaling the condition by the area
}

double Cloth::condShear(triangle t)
{
    auto wuv = getWUV(t);
    return UVarea * dot(wuv.first, wuv.second);
}

double Cloth::condBend(int t1, int t2)
{
    dvec3 n1 = triNorms[t1];
    dvec3 n2 = triNorms[t2];
    auto shared = sharedEdge(t1, t2);
    auto e = (points[shared.first] - points[shared.second]);
    double sin = dot(cross(n1, n2), e); //getting sin and cos to maintain numerical stability
    double cos = dot(n1, n2);
    return atan(sin, cos);//, cos);
}

tuple<dvec3, dvec3, dvec3> Cloth::derivativeStretchX(triangle t, double stretchiness)
{
    auto result = make_tuple(dvec3(0.0), dvec3(0.0), dvec3(0.0)); //for each of the three points
    for(int i = 0; i < 3; i++)
    {
        points[get<0>(t)][i] -= DEL;
        double f1 = condStretchX(t, stretchiness);
        points[get<0>(t)][i] += 2*DEL;
        double f2 = condStretchX(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL); //numerical gradient
        points[get<0>(t)][i] -= DEL;
        get<0>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<1>(t)][i] -= DEL;
        double f1 = condStretchX(t, stretchiness);
        points[get<1>(t)][i] += 2*DEL;
        double f2 = condStretchX(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL);
        points[get<1>(t)][i] -= DEL;
        get<1>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<2>(t)][i] -= DEL;
        double f1 = condStretchX(t, stretchiness);
        points[get<2>(t)][i] += 2*DEL;
        double f2 = condStretchX(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL);
        points[get<2>(t)][i] -= DEL;
        get<2>(result)[i] = grad;
    }
    return result;
}

tuple<dvec3, dvec3, dvec3> Cloth::derivativeStretchY(triangle t, double stretchiness)
{
    auto result = make_tuple(dvec3(0.0), dvec3(0.0), dvec3(0.0)); //for each of the three points
    for(int i = 0; i < 3; i++)
    {
        points[get<0>(t)][i] -= DEL;
        double f1 = condStretchY(t, stretchiness);
        points[get<0>(t)][i] += 2*DEL;
        double f2 = condStretchY(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL); //numerical gradient
        points[get<0>(t)][i] -= DEL;
        get<0>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<1>(t)][i] -= DEL;
        double f1 = condStretchY(t, stretchiness);
        points[get<1>(t)][i] += 2*DEL;
        double f2 = condStretchY(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL);
        points[get<1>(t)][i] -= DEL;
        get<1>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<2>(t)][i] -= DEL;
        double f1 = condStretchY(t, stretchiness);
        points[get<2>(t)][i] += 2*DEL;
        double f2 = condStretchY(t, stretchiness);
        double grad = (f2 - f1) / (2*DEL);
        points[get<2>(t)][i] -= DEL;
        get<2>(result)[i] = grad;
    }
    return result;
}

tuple<dvec3, dvec3, dvec3> Cloth::derivativeShear(triangle t) 
{
    auto result = make_tuple(dvec3(0.0), dvec3(0.0), dvec3(0.0)); //for each of the three points
    for(int i = 0; i < 3; i++)
    {
        points[get<0>(t)][i] -= DEL;
        double f1 = condShear(t);
        points[get<0>(t)][i] += 2*DEL;
        double f2 = condShear(t);
        double grad = (f2 - f1) / (2*DEL); //numerical gradient
        points[get<0>(t)][i] -= DEL;
        get<0>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<1>(t)][i] -= DEL;
        double f1 = condShear(t);
        points[get<1>(t)][i] += 2*DEL;
        double f2 = condShear(t);
        double grad = (f2 - f1) / (2*DEL);
        points[get<1>(t)][i] -= DEL;
        get<1>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<2>(t)][i] -= DEL;
        double f1 = condShear(t);
        points[get<2>(t)][i] += 2*DEL;
        double f2 = condShear(t);
        double grad = (f2 - f1) / (2*DEL);
        points[get<2>(t)][i] -= DEL;
        get<2>(result)[i] = grad;
    }
    return result;
}

tuple<dvec3, dvec3, dvec3, dvec3> Cloth::derivativeBend(int t1, int t2)
{
    auto result = make_tuple(dvec3(0.0), dvec3(0.0), dvec3(0.0), dvec3(0.0)); //for each of the four points
    for(int i = 0; i < 3; i++)
    {
        points[get<0>(triangles[t1])][i] -= DEL;
        double f1 = condBend(t1, t2);
        points[get<0>(triangles[t1])][i] += 2*DEL;
        double f2 = condBend(t1, t2);
        double grad = (f2 - f1) / (2*DEL); //numerical gradient        
        points[get<0>(triangles[t1])][i] -= DEL;
        get<0>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<1>(triangles[t1])][i] -= DEL;
        double f1 = condBend(t1, t2);
        points[get<1>(triangles[t1])][i] += 2*DEL;
        double f2 = condBend(t1, t2);
        double grad = (f2 - f1) / (2*DEL);
        points[get<1>(triangles[t1])][i] -= DEL;
        get<1>(result)[i] = grad;
    }
    for(int i = 0; i < 3; i++)
    {
        points[get<2>(triangles[t1])][i] -= DEL;
        double f1 = condBend(t1, t2);
        points[get<2>(triangles[t1])][i] += 2*DEL;
        double f2 = condBend(t1, t2);
        double grad = (f2 - f1) / (2*DEL);
        points[get<2>(triangles[t1])][i] -= DEL;
        get<2>(result)[i] = grad;
    }
    auto shared = sharedEdge(t1, t2);
    int rem = getRemaining(shared, t2);
    for(int i = 0; i < 3; i++)
    {
        points[rem][i] -= DEL;
        double f1 = condBend(t1, t2);
        points[rem][i] += 2*DEL;
        double f2 = condBend(t1, t2);        
        double grad = (f2 - f1) / (2*DEL);
        points[rem][i] -= DEL;
        get<3>(result)[i] = grad;
    }
    return result;    
}

void Cloth::addStretchXForces(double str)
{
    for(int i = 0; i < triangles.size(); i++)
    {
        auto gradx = derivativeStretchX(triangles[i], str);
        double condx = condStretchX(triangles[i], str);
        forces[get<0>(triangles[i])] -= clamp(get<0>(gradx)*condx*KSTRX, MAX_STRETCH); //adding forces
        forces[get<1>(triangles[i])] -= clamp(get<1>(gradx)*condx*KSTRX, MAX_STRETCH);
        forces[get<2>(triangles[i])] -= clamp(get<2>(gradx)*condx*KSTRX, MAX_STRETCH);
        double timederivative = dot(get<0>(gradx), velocities[get<0>(triangles[i])]); //time derivative of the condition
        timederivative +=  dot(get<1>(gradx), velocities[get<1>(triangles[i])]);
        timederivative += dot(get<2>(gradx), velocities[get<2>(triangles[i])]);
        forces[get<0>(triangles[i])] -= clamp(get<0>(gradx)*timederivative*KSTRX*KDAMP, MAX_STRETCH_DAMP); //damping
        forces[get<1>(triangles[i])] -= clamp(get<1>(gradx)*timederivative*KSTRX*KDAMP, MAX_STRETCH_DAMP);
        forces[get<2>(triangles[i])] -= clamp(get<2>(gradx)*timederivative*KSTRX*KDAMP, MAX_STRETCH_DAMP);
    }
}

void Cloth::addStretchYForces(double str)
{
    for(int i = 0; i < triangles.size(); i++)
    {
        auto grady = derivativeStretchY(triangles[i], str);
        double condy = condStretchY(triangles[i], str);
        forces[get<0>(triangles[i])] -= clamp(get<0>(grady)*condy*KSTRY, MAX_STRETCH); //adding forces
        forces[get<1>(triangles[i])] -= clamp(get<1>(grady)*condy*KSTRY, MAX_STRETCH);
        forces[get<2>(triangles[i])] -= clamp(get<2>(grady)*condy*KSTRY, MAX_STRETCH);
        double timederivative = dot(get<0>(grady), velocities[get<0>(triangles[i])]); //time derivative of the condition
        timederivative +=  dot(get<1>(grady), velocities[get<1>(triangles[i])]);
        timederivative += dot(get<2>(grady), velocities[get<2>(triangles[i])]);
        forces[get<0>(triangles[i])] -= clamp(get<0>(grady)*timederivative*KSTRY*KDAMP, MAX_STRETCH_DAMP); //damping
        forces[get<1>(triangles[i])] -= clamp(get<1>(grady)*timederivative*KSTRY*KDAMP, MAX_STRETCH_DAMP);
        forces[get<2>(triangles[i])] -= clamp(get<2>(grady)*timederivative*KSTRY*KDAMP, MAX_STRETCH_DAMP);
    }
}

void Cloth::addShearForces()
{
    for(int i = 0; i < triangles.size(); i++)
    {
        auto gradsh = derivativeShear(triangles[i]);
        double condsh = condShear(triangles[i]);
        forces[get<0>(triangles[i])] -= clamp(get<0>(gradsh)*condsh*KSH, MAX_SHEAR); //adding normal forces
        forces[get<1>(triangles[i])] -= clamp(get<1>(gradsh)*condsh*KSH, MAX_SHEAR);
        forces[get<2>(triangles[i])] -= clamp(get<2>(gradsh)*condsh*KSH, MAX_SHEAR);
        double timederivative = dot(get<0>(gradsh), velocities[get<0>(triangles[i])]); //time derivative of the condition
        timederivative +=  dot(get<1>(gradsh), velocities[get<1>(triangles[i])]);
        timederivative += dot(get<2>(gradsh), velocities[get<2>(triangles[i])]);
        forces[get<0>(triangles[i])] -= clamp(get<0>(gradsh)*timederivative*KSH*KDAMP, MAX_SHEAR_DAMP); //damping
        forces[get<1>(triangles[i])] -= clamp(get<1>(gradsh)*timederivative*KSH*KDAMP, MAX_SHEAR_DAMP);
        forces[get<2>(triangles[i])] -= clamp(get<2>(gradsh)*timederivative*KSH*KDAMP, MAX_SHEAR_DAMP);
    }
}

void Cloth::checkAndBend(int t1, int t2)
{
    
    if(t2 < 0 || t2 >= triangles.size()) //rejecting if out of bounds
        return;
    double condb = condBend(t1, t2);
    auto gradb = derivativeBend(t1, t2);
    forces[get<0>(triangles[t1])] -= clamp(get<0>(gradb)*condb*KBEND, MAX_BEND); //adding normal forces
    forces[get<1>(triangles[t1])] -= clamp(get<1>(gradb)*condb*KBEND, MAX_BEND);
    forces[get<2>(triangles[t1])] -= clamp(get<2>(gradb)*condb*KBEND, MAX_BEND);
    int rem = getRemaining(sharedEdge(t1, t2), t2);
    forces[rem] -= get<3>(gradb)*condb*KBEND;
    double timederivative = dot(get<0>(gradb), velocities[get<0>(triangles[t1])]); //time derivative of the condition
    timederivative +=  dot(get<1>(gradb), velocities[get<1>(triangles[t1])]);
    timederivative += dot(get<2>(gradb), velocities[get<2>(triangles[t1])]);
    timederivative += dot(get<3>(gradb), velocities[rem]);
    forces[get<0>(triangles[t1])] -= clamp(get<0>(gradb)*timederivative*KBEND*KDAMP, MAX_BEND_DAMP); //damping
    forces[get<1>(triangles[t1])] -= clamp(get<1>(gradb)*timederivative*KBEND*KDAMP, MAX_BEND_DAMP);
    forces[get<2>(triangles[t1])] -= clamp(get<2>(gradb)*timederivative*KBEND*KDAMP, MAX_BEND_DAMP);
    forces[rem] -= get<3>(gradb)*timederivative*KBEND*KDAMP;
}

void Cloth::addBendForces()
{
    for(int i = 0; i < triangles.size(); i+=2)
    {
        int c1 = i-1;
        int c2 = i+1;
        int c3 = i+numX+1;
        checkAndBend(i, c1); //trying for all three possible triangles
        checkAndBend(i, c2);
        checkAndBend(i, c3);
    }
}

void Cloth::update()
{
    for(int i = 0; i < forces.size(); i++)
    {
        forces[i] = dvec3(0.0);
    }
    addStretchXForces(STRX);
    addStretchYForces(STRY);
    addShearForces();
    addBendForces();
    integrate();
    makeNorms();
}

dvec3 Cloth::getNormTriangle(triangle t)
{
    auto p0 = points[get<0>(t)];
    auto p1 = points[get<1>(t)];
    auto p2 = points[get<2>(t)];
    auto d1 = p1 - p0;
    auto d2 = p2 - p0;
    auto norm = cross(d1, d2);
    return normalize(norm);
}


void Cloth::makeNorms()
{
    for(int i = 0; i < pointNorms.size(); i++)
    {
        pointNorms[i] = dvec3(0, 0, 0);
    }
    for(int i = 0; i < triangles.size(); i++)
    {
        auto t = triangles[i];
        auto p0 = points[get<0>(t)];
        auto p1 = points[get<1>(t)];
        auto p2 = points[get<2>(t)];
        auto d1 = p1 - p0;
        auto d2 = p2 - p0;
        auto norm = cross(d1, d2);
        triNorms[i] = normalize(norm);
        pointNorms[get<0>(t)] += norm; //additive normal generation for smooth shading
        pointNorms[get<1>(t)] += norm;
        pointNorms[get<2>(t)] += norm;
            
    }
    for(int i = 0; i < pointNorms.size(); i++)
    {
        pointNorms[i] = normalize(pointNorms[i]); //normalizing all the point normals
    }
}
