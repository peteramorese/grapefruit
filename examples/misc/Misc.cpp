#include "Grapefruit.h"

#include <Eigen/Dense>

#include <iostream>
#include <spot/misc/version.hh>

using namespace GF;

int main() {

    std::string filepath = "test_state_space.yaml";

	DiscreteModel::StateSpace ss_camera(3);

	// Create state space:
	ss_camera.setDimension(0, "pan", {"left","center","right"}); //pan
	ss_camera.setDimension(1, "tilt", {"up","center","down"}); // tilt
	ss_camera.setDimension(2, "power", {"on","off"}); // power

	ss_camera.addGroup("pointing locations", {"pan", "tilt"});
	   
	// Write to file
    //ss_camera.serialize(filepath);

	// Read from file
	//DiscreteModel::StateSpace ss_camera_deserialize(filepath);
	//ss_camera_deserialize.print();

    DiscreteModel::State state_1(&ss_camera);
    state_1 = {"left", "down", "on"};
    Serializer szr("test.yaml");
    YAML::Emitter& out = szr.get();
    out << YAML::Key << "test_state" << YAML::Value;
    state_1.serialize(szr);

    DiscreteModel::State state_2(&ss_camera);
    state_2 = {"right", "down", "on"};
    out << YAML::Key << "test_state_2" << YAML::Value;
    state_2.serialize(szr);

    szr.done();
    
    return 0;
}
