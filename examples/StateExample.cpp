#include<iostream>

#include "core/State.h"
#include "core/StateSpace.h"

int main() {
 
    // Set up state space:
	DiscreteModel::StateSpace ss_camera(3);
	std::vector<std::string> pan_labels = {"left","center","right"};
	std::vector<std::string> tilt_labels = {"up","center","down"};
	std::vector<std::string> power_labels = {"on","off"};
	
	ss_camera.setDimension(0, "pan", pan_labels); //pan
	ss_camera.setDimension(1, "tilt", tilt_labels); // tilt
	ss_camera.setDimension(2, "power", power_labels); // power

	std::vector<std::string> point_group = {"pan", "tilt"};
	ss_camera.addGroup("pointing locations", point_group);
	   
    DiscreteModel::State state_1(&ss_camera, {"center", "up", "off"});
    LOG("state_1:");
    state_1.print();

    DiscreteModel::State state_2 = state_1;
    LOG("state_2 (copied):");
    state_2.print();

    LOG("state_2['pan']: " << (const std::string&)state_2["pan"]);
    LOG("state_2['tilt']: " << (const std::string&)state_2["tilt"]);
    LOG("state_2['power']: " << (const std::string&)state_2["power"]);

    state_2["tilt"] = "down";
    LOG("Edited tilt to be down:");
    state_2.print();

    LOG("Trying to edit power to be 'out' (should fail)");
    state_2["power"] = "out";


	return 0;
}
