#include <veamy/models/constraints/Constraints.h>
#include <delynoi/models/basic/Angle.h>

void Constraints::addConstraint(SegmentConstraint c, std::vector<Point> p) {
    UniqueList<IndexSegment> segments = c.getSegments();

    for (int i = 0; i < segments.size(); ++i) {
        IndexSegment s = segments[i];
        Angle angle(s.cartesianAngle(p));

        std::vector<IndexSegment>& v = constrained_segments[angle];
        v.push_back(s);

        std::vector<SegmentConstraint>& relatedConstraints = segment_map[segments[i]];
        relatedConstraints.push_back(c);
    }
}

void Constraints::addConstraint(SegmentConstraint c, UniqueList<Point> p) {
    addConstraint(c, p.getList());
}

void Constraints::addConstraint(PointConstraint c) {
    UniqueList<Point> points = c.getPoints();

    for (int i = 0; i < points.size(); ++i) {
        std::vector<PointConstraint>& relatedConstraints = point_map[points[i]];
        relatedConstraints.push_back(c);
    }
}

isConstrainedInfo Constraints::isConstrainedBySegment(std::vector <Point> points, IndexSegment s) {
    Angle angle(s.cartesianAngle(points));
    auto iter = constrained_segments.find(angle);

    if(iter == constrained_segments.end())
        return isConstrainedInfo();

    std::vector<IndexSegment> segments = iter->second;

    for (int i = 0; i < segments.size(); ++i) {
        if(segments[i].contains(points,s)){
            return isConstrainedInfo(segments[i]);
        }
    }

    return isConstrainedInfo();
}

isConstrainedInfo Constraints::isConstrainedByPoint(Point p) {
    auto iter = point_map.find(p);

    if(iter == point_map.end()){
        return isConstrainedInfo();
    }

    return isConstrainedInfo(true);
}

bool Constraints::isConstrained(int dof) {
    return constrained_dofs.contains(dof);
}

std::vector<int> Constraints::getConstrainedDOF() {
    return constrained_dofs.getList();
}

void Constraints::addConstrainedDOF(std::vector <Point> points, int DOF_index, int axis, SegmentPair pair,
                                    int dof_point) {
    bool added1 = addConstrainedDOFBySegment(points, DOF_index, axis, pair.s1);
    bool added2 = addConstrainedDOFBySegment(points, DOF_index, axis, pair.s2);

    if(!added1 && !added2){
        checkIfContainedInConstraint(points[dof_point], points, DOF_index, axis);
    }

    addConstrainedDOFByPoint(DOF_index, axis, points[dof_point]);
}

bool Constraints::addConstrainedDOFBySegment(std::vector<Point> points, int DOF_index, int axis, IndexSegment s) {
    isConstrainedInfo info = isConstrainedBySegment(points, s);

    if(info.isConstrained){
        constrainDOFBySegment(info.container, DOF_index, axis);
    }
    return false;
}

bool Constraints::addConstrainedDOFByPoint(int DOF_index, int axis, Point p) {
    auto iter = point_map.find(p);

    if(iter != point_map.end()){
        std::vector<PointConstraint> constraints = iter->second;

        for(PointConstraint constraint: constraints){
            std::vector<int> direction = constraint.getDirection();

            if(std::find(direction.begin(), direction.end(), axis) != direction.end()){
                constrained_dofs.push_back(DOF_index);
                point_constraints_map[DOF_index] = constraint;
            }
        }

        return false;
    }

    return false;
}

bool Constraints::checkIfContainedInConstraint(Point p, std::vector<Point> points, int DOF_index, int axis) {
    for(auto seg: constrained_segments){
        std::vector<IndexSegment> segs = seg.second;

        for (IndexSegment s: segs){
            if(s.contains(points, p)){
                constrainDOFBySegment(s, DOF_index, axis);
            }
        }
    }
}

std::unordered_map<IndexSegment, std::vector<SegmentConstraint>, SegmentHasher> Constraints::getConstrainedSegments() {
    return this->segment_map;
}

Constraint Constraints::getAssociatedConstraint(int dof_index) {
    auto iter = segment_constraints_map.find(dof_index);

    if(iter != segment_constraints_map.end()){
        return iter->second;
    }

    auto iter2 = point_constraints_map.find(dof_index);
    if(iter2 != point_constraints_map.end()){
        return iter2->second;
    }

    return Constraint();
}

bool Constraints::constrainDOFBySegment(IndexSegment segment, int DOF_index, int axis) {
    std::vector<SegmentConstraint> constraints = segment_map[segment];

    for (SegmentConstraint constraint: constraints) {
        std::vector<int> direction = constraint.getDirection();

        if(std::find(direction.begin(), direction.end(), axis) != direction.end()){
            constrained_dofs.push_back(DOF_index);
            segment_constraints_map[DOF_index] = constraint;
            return true;
        }
    }
    return false;
}


