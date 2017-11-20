#include <veamy/models/constraints/EssentialConstraints.h>
#include <delynoi/models/Mesh.h>
#include <veamy/models/constraints/values/Constant.h>
#include <veamy/models/constraints/NaturalConstraints.h>
#include <veamy/models/constraints/ConstraintsContainer.h>
#include <veamy/physics/materials/Material.h>
#include <veamy/Veamer.h>
#include <utilities/utilities.h>
#include <veamy/physics/materials/MaterialPlaneStress.h>
#include <veamy/config/VeamyConfig.h>
#include <veamy/postprocess/analytic/DisplacementValue.h>
#include <veamy/postprocess/L2NormCalculator.h>
#include <delynoi/models/Region.h>
#include <delynoi/voronoi/TriangleVoronoiGenerator.h>
#include <veamy/postprocess/analytic/StrainValue.h>
#include <veamy/postprocess/analytic/StressValue.h>
#include <veamy/postprocess/H1NormCalculator.h>

Pair<double> analytic(double x, double y){
    double E = 3e7, v = 0.3;

    return Pair<double> ((v/E)*(1-x), y/E);
}

Trio<double> strain(double x, double y){
    double E = 3e7, v = 0.3;

    return Trio<double>(-v/E,1/E,0);
}

Trio<double> stress(double x, double y){
    return Trio<double>(0, 1, 0);
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
    std::string meshFileName = "equi_patch_test_mesh.txt";
    std::string dispFileName = "equi_patch_test_displacements.txt";
    
    // File that contains an external mesh
    std::vector<Point> region_points = {Point(0, 0), Point(1, 0), Point(1, 1), Point(0, 1)};
    Region square(region_points);
    int n = 5;
    square.generateSeedPoints(PointGenerator(functions::constantAlternating(), functions::constant()), n , n);

    std::vector<Point> seeds = square.getSeedPoints();
    TriangleVoronoiGenerator generator (seeds, square);
    Mesh<Polygon> mesh = generator.getMesh();

    mesh.printInFile("mesh_vem.txt");

    EssentialConstraints essential;
    PointSegment downSide(Point(0,0), Point(1,0));
    SegmentConstraint down (downSide, mesh.getPoints(), Constraint::Direction::Vertical, new Constant(0));
    essential.addConstraint(down, mesh.getPoints());
    Point cornerPoint(Point(1,0));
    PointConstraint corner(cornerPoint, Constraint::Direction::Horizontal, new Constant(0));
    essential.addConstraint(corner);

    NaturalConstraints natural;
    PointSegment topSide(Point(0,1), Point(1,1));
    SegmentConstraint top (topSide, mesh.getPoints(), Constraint::Direction::Vertical, new Constant(1));
    natural.addConstraint(top, mesh.getPoints());

    ConstraintsContainer container;
    container.addConstraints(essential, mesh.getPoints());
    container.addConstraints(natural, mesh.getPoints());
    std::cout << "done" << std::endl;

    std::cout << "+ Defining linear elastic material ... ";
    Material* material = new MaterialPlaneStress(3e7, 0.3);
    Conditions conditions(container, material);
    std::cout << "done" << std::endl;

    std::cout << "+ Preparing the simulation ... ";
    Veamer v;
    v.initProblem(mesh, conditions);
    std::cout << "done" << std::endl;

    std::cout << "+ Simulating ... ";
    Eigen::VectorXd x = v.simulate(mesh);
    std::cout << "done" << std::endl;

    std::cout << "+ Printing nodal displacement solution to a file ... ";
    v.writeDisplacements(dispFileName, x);
    std::cout << "done" << std::endl;
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

    DisplacementValue* realSolution = new DisplacementValue(analytic);
    L2NormCalculator<Polygon>* l2 = new L2NormCalculator<Polygon>(realSolution, x, v.DOFs);
    double norm = v.computeErrorNorm(l2, mesh);
    std::cout << "L2 norm: " << norm << std::endl;

    StrainValue* strainValue = new StrainValue(strain);
    StressValue* stressValue = new StressValue(stress);
    H1NormCalculator<Polygon>* h1 = new H1NormCalculator<Polygon>(strainValue, stressValue, realSolution, x, v.DOFs, mesh.getPoints().getList());
    double h1norm = v.computeErrorNorm(h1, mesh);
    std::cout << "H1 norm: " << h1norm << std::endl;
}
