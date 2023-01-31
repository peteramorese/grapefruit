#include<iostream>

#include "core/StateSpace.h"


int main() {
 
    std::string filepath = "test_state_space.yaml";

	DiscreteModel::StateSpace ss_camera(3);
	std::vector<std::string> pan_labels = {"left","center","right"};
	std::vector<std::string> tilt_labels = {"up","center","down"};
	std::vector<std::string> power_labels = {"on","off"};
	
	// Create state space:
	ss_camera.setDimension(0, "pan", pan_labels); //pan
	ss_camera.setDimension(1, "tilt", tilt_labels); // tilt
	ss_camera.setDimension(2, "power", power_labels); // power

	std::vector<std::string> point_group = {"pan", "tilt"};
	ss_camera.addGroup("pointing locations", point_group);
	   
	// Write to file
    ss_camera.serialize(filepath);

	// Read from file
	DiscreteModel::StateSpace ss_camera_deserialize(filepath);
	ss_camera_deserialize.print();

	return 0;
}
