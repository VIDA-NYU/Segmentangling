#pragma once

#include <igl/edges.h>

#include <Eigen/Core>

#include <numeric>


namespace fresh_prince_of_utils {

    //#include "datfile.h"


    void edge_endpoints(const Eigen::MatrixXd& V,
        const Eigen::MatrixXi& F,
        Eigen::MatrixXd& V1,
        Eigen::MatrixXd& V2);


    // Scale a vector so its values lie between zero and one
    void scale_zero_one(const Eigen::VectorXd& V, Eigen::VectorXd& V_scaled);

    void scale_zero_one(Eigen::VectorXd& V);


    // Check if the point pt is in the tet at ID tet
    bool point_in_tet(const Eigen::MatrixXd& TV,
        const Eigen::MatrixXi& TT,
        const Eigen::RowVector3d& pt,
        int tet);


    // Return the index of the tet containing the point p or -1 if the vertex is in no tets
    int containing_tet(const Eigen::MatrixXd& TV,
        const Eigen::MatrixXi& TT,
        const Eigen::RowVector3d& p);

    // Return the index of the closest vertex to p
    int nearest_vertex(const Eigen::MatrixXd& TV,
        const Eigen::RowVector3d& p);



}