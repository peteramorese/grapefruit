#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include "automaton.h"
#include "graph.h"

template<class T>
void Automaton<T>::addWord(const std::string& word) {
	if (!word.empty()){
		alphabet.push_back(word);
	}
	
}

template<class T>
void Automaton<T>::addAcceptingState(unsigned int accepting_state) {
	if (accepting_state > max_accepting_state_index) {
		max_accepting_state_index = accepting_state;	
	}

}

template<class T>
Automaton<T>::Automaton() : Graph<T>(true), max_accepting_state_index(0), max_init_state_index(0)
{
	
}

template<class T>
bool Automaton<T>::isAutomatonValid() {
	bool valid = true;
	int graph_size = Graph<T>::size();
	if (max_accepting_state_index > graph_size || max_init_state_index > graph_size) {
		valid = false;	
	}
	return valid;
}

template<class T>
bool Automaton<T>::inAlphabet(std::string s) {
	for (int i=0; i<alphabet.size(); ++i) {
		if (s == alphabet[i]) {
			return true;
			break;
		}
	}	
	return false;
}

template<class T>
bool Automaton<T>::checkAlphabet(const Automaton* arg_dfa) {
	for (int i=0; i<arg_dfa->alphabet.size(); ++i) {
		if (!inAlphabet(arg_dfa->alphabet[i])) {
			return false;
		}
	}
	return true;
}



template<class T>
void Automaton<T>::setAcceptingStates(const std::vector<unsigned int>& accepting_states_){
	// Determine the max accepting state index to easily verify that all
	// accepting states exist in the graph
	accepting_states = accepting_states_;
	for (int i=0; i<accepting_states.size(); ++i) {
		if (accepting_states[i] > max_accepting_state_index) {
			max_accepting_state_index = accepting_states[i];	
		}
	}	
}

template<class T>
void Automaton<T>::setInitStates(const std::vector<unsigned int>& init_states_){
	// Determine the max init state index to easily verify that all
	// init states exist in the graph
	init_states = init_states_;
	for (int i=0; i<init_states.size(); ++i) {
		if (init_states[i] > max_init_state_index) {
			max_init_state_index = init_states[i];	
		}
	}	
}

template<class T>
void Automaton<T>::setAlphabet(const std::vector<std::string>& alphabet_) {
	alphabet = alphabet_;
}





DFA::DFA() : check_det(true) {}

void DFA::toggleCheckDeterminism(bool check_det_) {
	check_det = check_det_;
}

int DFA::getInitState() {
	//Assume only a single init state:
	return init_states[0];
}

bool DFA::connectDFA(unsigned int ind_from, unsigned int ind_to, const std::string& label_) {
	node_data_list.push_back(label_);
	if (check_det){
		bool new_state_from, new_state_to;
		new_state_from = (ind_from > size()) ? true : false;
		if (new_state_from) {
			std::vector<std::string*> label_list;
			getConnectedData(ind_from, label_list);
			for (int i=0; i<label_list.size(); ++i) {
				if (label_ == *label_list[i]) {
					std::cout<<"Error: Determinism check failed when connecting state "<<ind_from<<" to "<<ind_to<<" with letter: "<<label_<<"\n";
					return false;
				}
			}
		}
	}
	// If the letter is not seen among all other outgoing edges
	// from the 'from state', the connection is deterministic and
	// the states can be connected. DFA is unweighted by default.
	Graph::connect({ind_from, nullptr}, {ind_to, &node_data_list.back()});
	return true;
}

bool DFA::readFileSingle(const std::string& filename) {
	std::string line;
	std::ifstream dfa_file(filename);
	std::vector<std::string> modes = {"Alphabet", "InitialStates", "Graph", "AcceptingStates"};
	int entry_mode = 0; // First mode (0) dictates start of a new automaton
	if (dfa_file.is_open()) {
		int curr_mode = -1;
		unsigned int source_state, to_state;
		source_state = -1;
		bool seen_entry_mode = false;
		int entry_num = 0;
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
			if (curr_mode == entry_mode) {
				if (!seen_entry_mode) {
					seen_entry_mode = true;
					entry_num++;
				} 
				if (entry_num > 1) {
					std::cout<<"Warning: Can only read a single DFA into data structure. Ignoring extra DFA's in file...\n";
					break;
				}
			} else {
				if (seen_entry_mode) {
					seen_entry_mode = false;	
				}
			}
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
								connectDFA(source_state, to_state, temp_label);
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
		return true;
	} else {
		std::cout<<"Cannot open file: "<<filename<<"\n";
		return false;
	}
}

void DFA::print() {
	auto printLAM = [](Graph<std::string>::node* dst, Graph<std::string>::node* prv){std::cout<<"   ~> "<<dst->nodeind<<" : "<<*(dst->dataptr)<<"\n";};
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
	//Graph<std::string>::print(printLAM);

	for (int i=0; i<heads.size(); i++) {
		if (!isEmpty(heads[i])) {
			if (!isEmpty(heads[i]->adjptr)) {
				std::cout<<"Node: "<<i<<" is connected to:\n";
				Graph<std::string>::hop(i, printLAM);
			} else {
				std::cout<<"Node: "<<i<<" has no outgoing transitions.\n";
			}
		}
	}
}

//bool DFA::syncProduct(const DFA* dfa1, const DFA* dfa2, DFA* product) {
//	if (!dfa1->checkAlphabet(dfa2)) {
//		std::cout<<"Warning: Alphabet of dfa1 does not match that of dfa2\n";
//		return false;
//	}
//	int n = dfa1->size();
//	int m = dfa2->size();
//	auto heads_dfa1 = dfa1->getHeads();
//	auto heads_dfa2 = dfa2->getHeads();
//	int init_state_dfa1 = dfa1->getInitState();
//	int init_state_dfa2 = dfa2->getInitState();
//	int p_i = 0;
//	// THIS IS UNFINISHED
//}

/*
bool DFA::readFileMultiple(const std::string& filename, std::array<DFA, int>& dfa_arr) {
	std::string line;
	std::ifstream dfa_file(filename);
	std::vector<std::string> modes = {"Alphabet", "InitialStates", "Graph", "AcceptingStates"};
	int entry_mode = 0; // First mode (0) dictates start of a new automaton
	if (dfa_file.is_open()) {
		int curr_mode = -1;
		unsigned int source_state, to_state;
		source_state = -1;
		int dfa_counter = -1;
		bool seen_entry_mode = false;
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
			if (curr_mode == entry_mode) {
				if (!seen_entry_mode) {
					dfa_counter++;
					if (dfa_counter > dfa_arr.size()) {
						std::cout<<"Error: More automata in file than input array size.\n";
					}
					seen_entry_mode = true;
				}
			} else {
				if (seen_entry_mode) {
					seen_entry_mode = false;	
				}
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
					//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
					dfa_arr[dfa_counter].addWord(temp_word);
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
						//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
						dfa_arr[dfa_counter].setInitStates(init_states);
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
								//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
								dfa_arr[dfa_counter].connect(source_state, to_state, 0, temp_label);
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
						dfa_arr[dfa_counter].addAcceptingState(accepting_state);
					}
			}

		}	
		dfa_file.close();
		return true;
	} else {
		std::cout<<"Cannot open file: "<<filename<<"\n";
		return false;
	}
}
*/
