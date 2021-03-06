#ifndef VEAMY_ELEMENT_H
#define VEAMY_ELEMENT_H

#include <veamy/lib/Eigen/Dense>
#include <veamy/models/dof/DOFS.h>
#include <veamy/physics/conditions/Conditions.h>
#include <veamy/physics/traction/TractionVector.h>
#include <veamy/physics/bodyforces/BodyForceVector.h>

/*
 * Class that models a generic element of a problem
 */
template <typename T>
class Element {
protected:
    /*
     * Number of degrees of freedom per point
     */
    int n_dofs;

    /*
     * List of the indexes of the degrees of freedom of the element
     */
    std::vector<int> dofs;

    /*
     * Geometric equivalent of the element (Polygon, Triangle, etc)
     */
    T p;

    /*
     * Elemental load vector and stiffness matrix
     */
    Eigen::VectorXd f;
    Eigen::MatrixXd K;
public:
    /*
     * Creates the element from the problem conditions and the geometry
     * @param conditions of the problem
     * @param p geometric representation of the element
     * @param point mesh points
     * @param out set of degrees of freedom of the system
     */
    void initializeElement(Conditions *conditions, T &p, UniqueList<Point> &points, DOFS &out, int n_dofs);

    /*
     * @return the geometric equivalent of the element
     */
    T getAssociatedPolygon();

    /* Assembles the global stiffness matrix and load vector
     * @param out degrees of freedom of the system
     * @param Kglobal global stiffness matrix
     * @param Fglobal global load vector
     */
    void assemble(DOFS out, Eigen::MatrixXd &Kglobal, Eigen::VectorXd &Fglobal);

    /* Computes the global load vector considering both body forces and neumann boundary conditions
     * @param d degrees of freedom of the system
     * @param points mesh points
     * @param conditions conditions of the problem
     * @param bodyForceVector implementation of a body force vector calculator
     * @param tractionVector implementation of a traction vector calculator
     */
    void computeF(DOFS d, UniqueList<Point> points, Conditions *conditions, BodyForceVector *bodyForceVector,
                  TractionVector *tractionVector);

    /* Computes the elemental load vector
     * @param d degrees of freedom of the system
     * @param points mesh points
     * @param conditions conditions of the problem
     */
    virtual void computeF(DOFS &d, UniqueList<Point> &points, Conditions *conditions) = 0;

    /* Computes the elemental stiffness matrix
     * @param d degrees of freedom of the system
     * @param points mesh points
     */
    virtual void computeK(DOFS &d, UniqueList<Point> &points) = 0;
};

#endif