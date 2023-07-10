#include<iostream>

#include "core/StateSpace.h"

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
    ss_camera.serialize(filepath);

	// Read from file
	DiscreteModel::StateSpace ss_camera_deserialize(filepath);
	ss_camera_deserialize.print();

	return 0;
}
