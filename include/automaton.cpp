#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include "automaton.h"
#include "edge.h"

void Automaton::addWord(const std::string& word) {
	if (!word.empty()){
		alphabet.push_back(word);
	}
	
}

void Automaton::addAcceptingState(unsigned int accepting_state) {
	accepting_states.push_back(accepting_state);
	if (accepting_state > max_accepting_state_index) {
		max_accepting_state_index = accepting_state;	
	}

}

Automaton::Automaton() : Edge(true), max_accepting_state_index(0), max_init_state_index(0)
{
	
}

bool Automaton::isAutomatonValid() {
	bool valid = true;
	int graph_size = size();
	if (max_accepting_state_index > graph_size || max_init_state_index > graph_size) {
		valid = false;	
	}
	return valid;
}

bool Automaton::inAlphabet(std::string s) {
	for (int i=0; i<alphabet.size(); ++i) {
		if (s == alphabet[i]) {
			return true;
		}
	}	
	return false;
}

void Automaton::setAcceptingStates(const std::vector<unsigned int>& accepting_states_){
	// Determine the max accepting state index to easily verify that all
	// accepting states exist in the graph
	accepting_states = accepting_states_;
	for (int i=0; i<accepting_states.size(); ++i) {
		if (accepting_states[i] > max_accepting_state_index) {
			max_accepting_state_index = accepting_states[i];	
		}
	}	
}

void Automaton::setInitStates(const std::vector<unsigned int>& init_states_){
	// Determine the max init state index to easily verify that all
	// init states exist in the graph
	init_states = init_states_;
	for (int i=0; i<init_states.size(); ++i) {
		if (init_states[i] > max_init_state_index) {
			max_init_state_index = init_states[i];	
		}
	}	
}

void Automaton::setAlphabet(const std::vector<std::string>& alphabet_) {
	alphabet = alphabet_;
}

void Automaton::print() {
	std::cout<<"Alphabet: ";
	for (int i=0; i<alphabet.size(); ++i) {
		std::cout<<alphabet[i]<<" ";
	}
	std::cout<<"\nInitial States: ";
	for (int i=0; i<init_states.size(); ++i) {
		std::cout<<init_states[i]<<" ";
	}
	std::cout<<"\nAccepting States: ";
	for (int i=0; i<accepting_states.size(); ++i) {
		std::cout<<accepting_states[i]<<" ";
	}
	std::cout<<"\n";
	Edge::print();
}



DFA::DFA() : check_det(true) {}

void DFA::toggleCheckDeterminism(bool check_det_) {
	check_det = check_det_;
}

bool DFA::connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_) {
	//if (inAlphabet(label_)) {
		if (check_det){
			bool new_state_from, new_state_to;
			new_state_from = (ind_from > size()) ? true : false;
			//if (state_ind_from > g.size()) {
			//	new_state_from = true;
			//} else {
			//	new_state_from = false;
			//}
			if (new_state_from) {
				std::vector<std::string> label_list;
				returnListLabels(ind_from, label_list);
				for (int i=0; i<label_list.size(); ++i) {
					if (label_ == label_list[i]) {
						std::cout<<"Error: Determinism check failed when connecting state "<<ind_from<<" to "<<ind_to<<" with letter: "<<label_<<"\n";
						return false;
					}
				}
			}
		}
		// If the letter is not seen among all other outgoing edges
		// from the 'from state', the connection is deterministic and
		// the states can be connected. DFA is unweighted by default.
		Edge::connect(ind_from, ind_to, 0, label_);
		return true;
	//} else {
	//	std::cout<<"Warning: Connecting edge with label that is not in alphabet: "<<label_<<"\n";
	//	return false;
	//}
}

bool DFA::readFromFile(std::string filename) {
	std::string line;
	std::ifstream dfa_file(filename);
	std::vector<std::string> modes = {"Alphabet", "InitialStates", "Graph", "AcceptingStates"};
	if (dfa_file.is_open()) {
		int curr_mode = -1;
		unsigned int source_state, to_state;
		source_state = -1;
		while (std::getline(dfa_file, line)) {
			std::cout<<line<<"\n";	
			// Get the current data type:
			std::string temp_word;
			std::string line_dec; // Line deconstructed into words
			bool is_element = false; // Quickly determine if the line is an element
			for (int i=0; i<line.size(); ++i) {
				switch (line.at(i)) {
					case '-':
						is_element = true;
						break;
					case ':':
					       break;
					case ' ':
					//       if (line_dec.size()==0) {
					//	       line_dec = temp_word;
					//	       temp_word.clear();
					       line_dec = line_dec + temp_word;
					       temp_word.clear();
					       break;
					default:
					       temp_word.push_back(line.at(i));
				}
				if (is_element) {
					break;
				}
				
			}
			if (temp_word.size() != 0) {
				line_dec += temp_word;
			}
			// Cycle through different modes depending on the current data type:
			bool found = false;
			if (!is_element) {
				for (int i=0; i<modes.size(); ++i) {
					if (modes[i] == line_dec) {
						curr_mode = i;
						found = true;
						break;
					}	
				}
				if (!found) {
					curr_mode = -1;
					if (line_dec.size() > 0) {
						std::cout<<"Warning: Unrecognized Automaton field: "<<line_dec<<"\n";
					}
					continue;
				}
			}
			if (!is_element) {
				continue;
			}
			std::string temp_word_2;
			switch (curr_mode) {
				case -1: 
					std::cout<<"Error: No field specified\n";
					break;
				case 0:
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}
					}
					addWord(temp_word);
					break;
				case 1:
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int init_state = std::stoi(temp_word_2,&sz);
						// Only include functionality for 1 initial state
						std::vector<unsigned int> init_states(1);
						init_states[0] = init_state;
						setInitStates(init_states);
					}
					break;
				case 2:
					{
						bool is_label = false;
						bool is_source_state = true;
						std::string temp_label;
						for (int i=0; i<line.size(); ++i) {
							switch (line[i]) {
								case '-':
									break;
								case ' ':
									break;
								case '>':
									temp_word_2.clear(); //disregard anything before the arrow
									is_source_state = false;
									break;
								case ':':
									is_label = true;
									break;
								default:
									if (is_label) {
										temp_label.push_back(line[i]);
									} else {
										temp_word_2.push_back(line[i]);
									}
							}	
						}
						if (is_source_state) {
							{
								std::string::size_type sz;
								source_state = std::stoi(temp_word_2,&sz);
							}
						} else {
							{
								std::string::size_type sz;
								to_state = std::stoi(temp_word_2,&sz);
								connect(source_state, to_state, 0, temp_label);
							}
						}
					}
					break;
				case 3:
					for (int i=0; i<line.size(); ++i) {
						switch (line[i]) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int accepting_state = std::stoi(temp_word_2,&sz);
						addAcceptingState(accepting_state);
					}
			}

		}	
		dfa_file.close();
	} else {
		std::cout<<"Cannot open file: "<<filename<<"\n";
		return false;
	}
}

