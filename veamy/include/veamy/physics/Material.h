#ifndef THESIS_MATERIAL_H
#define THESIS_MATERIAL_H

#include <veamy/lib/Eigen/Dense>
#include <veamy/physics/materials/Materials.h>

class Material{
protected:
    double material_E;
    double material_v;
public:
    Material(Materials::material m);
    Material(double young, double poisson);
    Material();

    virtual Eigen::MatrixXd getMaterialMatrix()=0;
    virtual double trace()=0;

    double k();
    double v();
    double E();
    bool operator==(const Material& other) const;
};

#endif
