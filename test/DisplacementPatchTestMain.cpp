#include <delynoi/voronoi/TriangleVoronoiGenerator.h>
#include <veamy/Veamer.h>
#include <veamy/models/constraints/values/Function.h>
#include <delynoi/models/generator/functions/functions.h>
#include <utilities/utilities.h>
#include <veamy/config/VeamyConfig.h>
#include <veamy/physics/materials/MaterialPlaneStrain.h>
#include <veamy/physics/conditions/LinearElasticityConditions.h>
#include <veamy/problems/VeamyLinearElasticityDiscretization.h>
#include <veamy/postprocess/L2NormCalculator.h>
#include <veamy/postprocess/analytic/StrainValue.h>
#include <veamy/postprocess/ElasticityH1NormCalculator.h>

double uXPatch(double x, double y){
    return x;
}

double uYPatch(double x, double y){
    return x + y;
}

Pair<double> realDisplacement(double x, double y){
    return Pair<double>(x, x+y);
}

Trio<double> realStrain(double x, double y){
    return Trio<double>(1,1,0.5);
}

int main(){
    // Set precision for plotting to output files:       
    // OPTION 1: in "VeamyConfig::instance()->setPrecision(Precision::precision::mid)"       
    // use "small" for 6 digits; "mid" for 10 digits; "large" for 16 digits.       
    // OPTION 2: set the desired precision, for instance, as:       
    // VeamyConfig::instance()->setPrecision(12) for 12 digits. Change "12" by the desired precision.       
    // OPTION 3: Omit any instruction "VeamyConfig::instance()->setPrecision(.....)"       
    // from this file. In this case, the default precision, which is 6 digits, will be used.      
    VeamyConfig::instance()->setPrecision(Precision::precision::large);

    // DEFINING PATH FOR THE OUTPUT FILES:
    // If the path for the output files is not given, they are written to /home directory by default.
    // Otherwise, include the path. For instance, for /home/user/Documents/Veamy/output.txt , the path
    // must be "Documents/Veamy/output.txt"
    // CAUTION: the path must exists either because it is already in your system or becuase it is created
    // by Veamy's configuration files. For instance, Veamy creates the folder "/test" inside "/build", so
    // one can save the output files to "/build/test/" folder, but not to "/build/test/mycustom_folder",
    // since "/mycustom_folder" won't be created by Veamy's configuration files.
    std::string meshFileName = "disp_patch_test_mesh.txt";
    std::string dispFileName = "disp_patch_test_displacements.txt";

    std::cout << "*** Starting Veamy ***" << std::endl;
    std::cout << "--> Test: Displacement patch test <--" << std::endl;
    std::cout << "..." << std::endl;

    std::cout << "+ Defining the domain ... ";
    std::vector<Point> rectangle4x8_points = {Point(0, -2), Point(8, -2), Point(8, 2), Point(0, 2)};
    Region rectangle4x8(rectangle4x8_points);
    std::cout << "done" << std::endl;

    std::cout << "+ Generating polygonal mesh ... ";
    rectangle4x8.generateSeedPoints(PointGenerator(functions::constant(), functions::constant()), 12, 6);
    std::vector<Point> seeds = rectangle4x8.getSeedPoints();
    TriangleVoronoiGenerator meshGenerator (seeds, rectangle4x8);
    Mesh<Polygon> mesh = meshGenerator.getMesh();
    std::cout << "done" << std::endl;

    std::cout << "+ Printing mesh to a file ... ";
    mesh.printInFile(meshFileName);
    std::cout << "done" << std::endl;

    std::cout << "+ Defining linear elastic material ... ";
    Material* material = new MaterialPlaneStrain (1e7, 0.3);
    LinearElasticityConditions* conditions = new LinearElasticityConditions(material);
    std::cout << "done" << std::endl;

    std::cout << "+ Defining Dirichlet and Neumann boundary conditions ... ";
    Function* uXConstraint = new Function(uXPatch);
    Function* uYConstraint = new Function(uYPatch);

    PointSegment leftSide(Point(0,-2), Point(0,2));
    SegmentConstraint leftX (leftSide, mesh.getPoints(), uXConstraint);
    conditions->addEssentialConstraint(leftX, mesh.getPoints(), elasticity_constraints::Direction::Horizontal);
    SegmentConstraint  leftY (leftSide, mesh.getPoints(), uYConstraint);
    conditions->addEssentialConstraint(leftY, mesh.getPoints(), elasticity_constraints::Direction::Vertical);

    PointSegment downSide(Point(0,-2), Point(8,-2));
    SegmentConstraint downX (downSide, mesh.getPoints(), uXConstraint);
    conditions->addEssentialConstraint(downX, mesh.getPoints(), elasticity_constraints::Direction::Horizontal);
    SegmentConstraint  downY (downSide, mesh.getPoints(), uYConstraint);
    conditions->addEssentialConstraint(downY, mesh.getPoints(), elasticity_constraints::Direction::Vertical);

    PointSegment rightSide(Point(8,-2), Point(8, 2));
    SegmentConstraint rightX (rightSide, mesh.getPoints(), uXConstraint);
    conditions->addEssentialConstraint(rightX, mesh.getPoints(), elasticity_constraints::Direction::Horizontal);
    SegmentConstraint  rightY (rightSide, mesh.getPoints(), uYConstraint);
    conditions->addEssentialConstraint(rightY, mesh.getPoints(), elasticity_constraints::Direction::Vertical);

    PointSegment topSide(Point(0, 2), Point(8, 2));
    SegmentConstraint topX (topSide, mesh.getPoints(), uXConstraint);
    conditions->addEssentialConstraint(topX, mesh.getPoints(), elasticity_constraints::Direction::Horizontal);
    SegmentConstraint  topY (topSide, mesh.getPoints(), uYConstraint);
    conditions->addEssentialConstraint(topY, mesh.getPoints(), elasticity_constraints::Direction::Vertical);
    std::cout << "done" << std::endl;

    std::cout << "+ Preparing the simulation ... ";
    VeamyLinearElasticityDiscretization* problem = new VeamyLinearElasticityDiscretization(conditions);

    Veamer v(problem);
    v.initProblem(mesh);
    std::cout << "done" << std::endl;

    std::cout << "+ Simulating ... ";
    Eigen::VectorXd x = v.simulate(mesh);
    std::cout << "done" << std::endl;

    std::cout << "+ Printing nodal displacement solution to a file ... ";
    v.writeDisplacements(dispFileName, x);
    std::cout << "done" << std::endl;

    std::cout << "+ Computing error norms ...";
    DisplacementValue* value = new DisplacementValue(realDisplacement);
    L2NormCalculator<Polygon>* calculator = new L2NormCalculator<Polygon>(value, x, v.DOFs);
    NormResult L2norm = problem->computeErrorNorm(calculator, mesh);

    std::cout << " + L2 norm value: ";
    std::cout << utilities::toString(L2norm.NormValue) << std::endl;

    std::cout << " + Maximum edge: ";
    std::cout << utilities::toString(L2norm.MaxEdge) << std::endl;
    StrainValue* strainValue = new StrainValue(realStrain);
    ElasticityH1NormCalculator<Polygon>* h1NormCalculator = new ElasticityH1NormCalculator<Polygon>(strainValue, x, v.DOFs);
    NormResult H1Norm = problem->computeErrorNorm(h1NormCalculator, mesh);

    std::cout << " +  H1 norm value: ";
    std::cout << utilities::toString(H1Norm.NormValue) << std::endl;

    std::cout << "+ Problem finished successfully" << std::endl;
    std::cout << "..." << std::endl;
    std::cout << "Check output files:" << std::endl;
    std::string path1 = utilities::getPath();
    std::string path2 = utilities::getPath();
    path1 +=  meshFileName;
    path2 +=  dispFileName;
    std::cout << path1 << std::endl;
    std::cout << path2 << std::endl;
    std::cout << "*** Veamy has ended ***" << std::endl;
}
