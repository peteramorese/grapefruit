#include "lexSet.h"

LexSet::LexSet(unsigned int S_) : S(S_), lex_set(S_,0), inf_set(false) {}

LexSet::LexSet(float fill_val, unsigned int S_) : S(S_), lex_set(S_,fill_val), inf_set(false) {}

LexSet::LexSet(const std::vector<float>* fill_set, unsigned int S_) : S(S_), inf_set(false) {
	lex_set = *fill_set;
}

unsigned int LexSet::size() const {
	return S;
}

void LexSet::setInf() {
	inf_set = true;
}

void LexSet::fill(float value) {
	for (int i=0; i<S; ++i) {
		lex_set[i] = value;
	}
}

bool LexSet::isInf() const {
	return inf_set;
}

float LexSet::getMaxVal() const {
	float ret_val = -1.0;
	for (int i=0; i<S; ++i) {
		if (i == 0 || lex_set[i] > ret_val) {
			ret_val = lex_set[i];
		}
	}
	return ret_val;
}

void LexSet::addToMax(float v) {
	float max_val;
	int max_ind;
	for (int i=0; i<S; ++i) {
		if (i == 0 || lex_set[i] > max_val) {
			max_val = lex_set[i];
			max_ind = i;
		}
	}
	lex_set[max_ind] += v;
}

void LexSet::operator+=(const LexSet& arg_set) {
	if (arg_set.size() == S) {
		for (int i=0; i<S; ++i) {
			lex_set[i] += arg_set.lex_set[i];
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void LexSet::operator+=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		for (int i=0; i<S; ++i) {
			lex_set[i] += arg_vec[i];
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void LexSet::operator=(const LexSet& arg_set) {
	if (arg_set.size() == S) {
		if (arg_set.inf_set) {
			inf_set = true;	
		} else {
			inf_set = false;
			lex_set = arg_set.lex_set;
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void LexSet::operator=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		inf_set = false;
		lex_set = arg_vec;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

bool LexSet::operator==(const LexSet& arg_set) const {
	if (arg_set.size() == lex_set.size()) {
		if (inf_set || arg_set.inf_set) {
			return (inf_set && arg_set.inf_set);
		}
		return lex_set == arg_set.lex_set;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool LexSet::operator<(const LexSet& arg_set) const {
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Infinite set is not less than infinite set
			return (inf_set) ? false : true;
		} else if (inf_set) {
			// Nothing is greater than infinite set
			return false;
		}
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] < arg_set.lex_set[i]) {
				return true;
			} else if (lex_set[i] > arg_set.lex_set[i]) {
				return false;
			}
		}
		return false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool LexSet::operator<=(const LexSet& arg_set) const {
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Any set is less than or equal to an infinite set
			return true;
		} else if (inf_set) {
			// If set is inf, only thing geq is another infinite set
			return (arg_set.inf_set) ? true : false;
		}
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] < arg_set.lex_set[i]) {
				return true;
			} else if (lex_set[i] > arg_set.lex_set[i]) {
				return false;
			}
		}
		return true;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool LexSet::operator>(const LexSet& arg_set) const {
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Any set is less than or equal to an infinite set
			return false;
		} else if (inf_set) {
			// Infinite set is not greater
			return (arg_set.inf_set) ? false : true;
		}
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] > arg_set.lex_set[i]) {
				return true;
			} else if (lex_set[i] < arg_set.lex_set[i]) {
				return false;
			}
		}
		return false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool LexSet::operator>=(const LexSet& arg_set) const {
	if (arg_set.size() == lex_set.size()) {
		if (inf_set) {
			// Any set is less than or equal to an infinite set
			return true;
		} else if (arg_set.inf_set) {
			// If set is inf, only thing geq is another infinite set
			return (inf_set) ? true : false;
		}
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] > arg_set.lex_set[i]) {
				return true;
			} else if (lex_set[i] < arg_set.lex_set[i]) {
				return false;
			}
		}
		return true;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

void LexSet::print() const {
	std::cout<<"{";
	if (inf_set) {
		std::cout<<"inf}\n";
	} else {
		for (int i=0; i<S; ++i) {
			std::cout<<lex_set[i];
			if (i != S-1) {
				std::cout<<", ";
			} else {
				std::cout<<"}\n";
			}
		}
	}
}


		/////////////////////////
		/* FlexLexSetS CLASSES */
		/////////////////////////



FlexLexSetS::FlexLexSetS(float mu_, unsigned int S_) : mu(mu_), LexSet(S_) {}

FlexLexSetS::FlexLexSetS(float mu_, float fill_val, unsigned int S_) : mu(mu_), LexSet(fill_val, S_)  {}

FlexLexSetS::FlexLexSetS(float mu_, const std::vector<float>* fill_set, unsigned int S_) : mu(mu_), LexSet(fill_set, S_) {}

void FlexLexSetS::overflow() {
	float buffer = 0;
	for (int i=S-1; i>=0; --i) {
		lex_set[i] = lex_set[i] + buffer;
		if (lex_set[i] > mu && i != 0) {
			buffer = lex_set[i] - mu;
			lex_set[i] = mu;
		}
	}
}

void FlexLexSetS::operator+=(const FlexLexSetS& arg_set) {
	if (arg_set.size() == S) {
		float buffer;
		for (int i=S-1; i>=0; --i) {
			lex_set[i] += arg_set.lex_set[i];
		}
		overflow();
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void FlexLexSetS::operator+=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		for (int i=S-1; i>=0; --i) {
			lex_set[i] += arg_vec[i];
		}
		overflow();
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void FlexLexSetS::operator=(const FlexLexSetS& arg_set) {
	if (arg_set.size() == S) {
		if (arg_set.inf_set) {
			inf_set = true;	
		} else {
			inf_set = false;
			lex_set = arg_set.lex_set;
		}
		overflow();
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void FlexLexSetS::operator=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		inf_set = false;
		lex_set = arg_vec;
		overflow();
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}



		////////////////////////
		/*   REQLex CLASSES   */
		////////////////////////


REQLex::REQLex(float mu_, unsigned int S_) : mu(mu_), LexSet(S_) {}

REQLex::REQLex(float mu_, float fill_val, unsigned int S_) : mu(mu_), LexSet(fill_val, S_)  {}

REQLex::REQLex(float mu_, const std::vector<float>* fill_set, unsigned int S_) : mu(mu_), LexSet(fill_set, S_) {}

void REQLex::operator=(const REQLex& arg_set) {
	if (arg_set.size() == S) {
		if (arg_set.inf_set) {
			inf_set = true;	
		} else {
			inf_set = false;
			lex_set = arg_set.lex_set;
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

bool REQLex::operator==(const REQLex& arg_set) {
	if (arg_set.size() == lex_set.size()) {
		if (inf_set || arg_set.inf_set) {
			return (inf_set && arg_set.inf_set);
		}
		return lex_set == arg_set.lex_set;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

void REQLex::operator=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		inf_set = false;
		lex_set = arg_vec;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

bool REQLex::operator<(const REQLex& arg_set) {
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Infinite set is not less than infinite set
			return (inf_set) ? false : true;
		} else if (inf_set) {
			// Nothing is greater than infinite set
			return false;
		}
		float total_val = 0;
		float arg_total_val = 0;
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] < (arg_set.lex_set[i] - mu)) {
				return true;
			} else if (lex_set[i] > arg_set.lex_set[i] + mu) {
				return false;
			}
			total_val += lex_set[i];
			arg_total_val += arg_set.lex_set[i];
			//if (lex_set[i] > total_val) {
			//	total_val = lex_set[i];
			//}
			//if (arg_set.lex_set[i] > arg_total_val) {
			//	arg_total_val = arg_set.lex_set[i];
			//}
		}
		return (total_val < arg_total_val) ? true : false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool REQLex::operator<=(const REQLex& arg_set) {
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Any set is less than or equal to an infinite set
			return true;
		} else if (inf_set) {
			// If set is inf, only thing geq is another infinite set
			return (arg_set.inf_set) ? true : false;
		}
		if (this->operator==(arg_set)) {
			return true;
		}
		float total_val = 0;
		float arg_total_val = 0;
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] <= (arg_set.lex_set[i] - mu)) {
				return true;
			} else if (lex_set[i] >= (arg_set.lex_set[i] + mu)) {
				return false;
			}
			if (lex_set[i] > total_val) {
				total_val = lex_set[i];
			}
			if (arg_set.lex_set[i] > arg_total_val) {
				arg_total_val = arg_set.lex_set[i];
			}
		}
		return (total_val <= arg_total_val) ? true : false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool REQLex::operator>(const REQLex& arg_set) {
	//std::cout<<"COMPARING this: ";
	//LexSet::print();
	//std::cout<<"TO ARG: ";
	//arg_set.print();
	if (arg_set.size() == lex_set.size()) {
		if (arg_set.inf_set) {
			// Any set is less than or equal to an infinite set
			return false;
		} else if (inf_set) {
			// Infinite set is not greater
			return (arg_set.inf_set) ? false : true;
		}
		float total_val = 0;
		float arg_total_val = 0;
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] >= arg_set.lex_set[i] + mu) {
				//std::cout<<"we in bigger than"<<std::endl;
				return true;
			} else if (lex_set[i] <= arg_set.lex_set[i] - mu) {
				//std::cout<<"we in smaller than"<<std::endl;
				return false;
			}
			total_val = total_val + lex_set[i];
			arg_total_val = arg_total_val + arg_set.lex_set[i];
			//if (lex_set[i] > total_val) {
			//	total_val = lex_set[i];
			//}
			//if (arg_set.lex_set[i] > arg_total_val) {
			//	arg_total_val = arg_set.lex_set[i];
			//}
		}
		//std::cout<<"total_val: "<<total_val<<" arg_total_val: "<<arg_total_val<<" returning: "<<(total_val > arg_total_val)<<std::endl;
		return (total_val > arg_total_val) ? true : false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}

bool REQLex::operator>=(const REQLex& arg_set) {
	if (arg_set.size() == lex_set.size()) {
		if (inf_set) {
			// Any set is less than or equal to an infinite set
			return true;
		} else if (arg_set.inf_set) {
			// If set is inf, only thing geq is another infinite set
			return (inf_set) ? true : false;
		}
		if (this->operator==(arg_set)) {
			return true;
		}
		float total_val = 0;
		float arg_total_val = 0;
		for (int i=0; i<arg_set.size(); ++i) {
			if (lex_set[i] > arg_set.lex_set[i] + mu) {
				return true;
			} else if (lex_set[i] < arg_set.lex_set[i] - mu) {
				return false;
			}
			total_val = total_val + lex_set[i];
			arg_total_val = arg_total_val + arg_set.lex_set[i];
			//if (lex_set[i] > total_val) {
			//	total_val = lex_set[i];
			//}
			//if (arg_set.lex_set[i] > arg_total_val) {
			//	arg_total_val = arg_set.lex_set[i];
			//}
		}
		return (total_val >= arg_total_val) ? true : false;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
		return false;
	}
}


		////////////////////////
		/*  DetourLex CLASS   */
		////////////////////////


DetourLex::DetourLex(float mu_, unsigned int S_) : mu(mu_), LexSet(S_) {}

DetourLex::DetourLex(float mu_, float fill_val, unsigned int S_) : mu(mu_), LexSet(fill_val, S_)  {}

DetourLex::DetourLex(float mu_, const std::vector<float>* fill_set, unsigned int S_) : mu(mu_), LexSet(fill_set, S_) {}

bool DetourLex::withinBounds(const DetourLex& arg_set) const {
	if (arg_set.size() == S) {
		for (int i=0; i<S; ++i) {
			if (arg_set.lex_set[i] > lex_set[i] + mu) {
				return false;
			}
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
	return true;
}

void DetourLex::operator+=(const DetourLex& arg_set) {
	if (arg_set.size() == S) {
		for (int i=0; i<S; ++i) {
			lex_set[i] += arg_set.lex_set[i];
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void DetourLex::operator+=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		for (int i=0; i<S; ++i) {
			lex_set[i] += arg_vec[i];
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}
void DetourLex::operator=(const DetourLex& arg_set) {
	if (arg_set.size() == S) {
		if (arg_set.inf_set) {
			inf_set = true;	
		} else {
			inf_set = false;
			lex_set = arg_set.lex_set;
		}
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}


void DetourLex::operator=(const std::vector<float>& arg_vec) {
	if (arg_vec.size() == S) {
		inf_set = false;
		lex_set = arg_vec;
	} else {
		std::cout<<"Error: Cannot operate on sets of different size.\n";
	}
}

void DetourLex::addHeuristic(const std::vector<float>& h_vals) {
	int max_val = 0;
	int max_ind = -1;
	for (int i=0; i<S; ++i) {
		if ((lex_set[i] + h_vals[i]) > max_val) {
			max_val = lex_set[i] + h_vals[i];
			max_ind = i;
		}
	}
	if (max_ind >= 0 && max_val >= 0) {
		lex_set[max_ind] = max_val;
	}
}






