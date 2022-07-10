#include<array>
#include "graph.h"
#include "condition.h"
#include "stateSpace.h"
#include "state.h"
#include "game.h"
#include "stratSynthesis.h"
#include "mockGamePlay.h"

int main() {



	/* CREATE ENVIRONMENT FOR GRID ROBOT */
	StateSpace SS;

	std::vector<std::string> states = {"a", "b", "c", "d", "e", "f", "g", "h"};

	// Create state space:
	SS.setStateDimension(states, 0); // x

	// Label state space:
	SS.setStateDimensionLabel(0, "letter");

	// Create object location group:


	Game<State> game(2, true, true); // by default, the init node for the ts is 0

    // Make the graph here: ///////////
    State a(&SS);
    a.setState({"a"});
    State b(&SS);
    b.setState({"b"});
    State c(&SS);
    c.setState({"c"});
    State d(&SS);
    d.setState({"d"});
    State e(&SS);
    e.setState({"e"});
    State f(&SS);
    f.setState({"f"});
    State g(&SS);
    g.setState({"g"});
    State h(&SS);
    h.setState({"h"});
    State i(&SS);
    i.setState({"h"});
    State j(&SS);
    j.setState({"h"});
    State k(&SS);
    k.setState({"h"});
    State l(&SS);
    l.setState({"h"});

	game.setInitState(&j, 1);

    game.connect(&j, 1, &g, 0, 1.0f, "to_g");
    game.connect(&j, 1, &h, 0, 1.0f, "to_h");
    game.connect(&g, 0, &b, 1, 1.0f, "to_b");
    game.connect(&g, 0, &c, 1, 1.0f, "to_c");
    game.connect(&h, 0, &d, 1, 1.0f, "to_d");
    game.connect(&h, 0, &e, 1, 1.0f, "to_e");
    game.connect(&e, 1, &i, 0, 1.0f, "to_i");
    game.connect(&e, 1, &a, 0, 1.0f, "to_a");
    game.connect(&i, 0, &j, 1, 1.0f, "to_j");
    game.connect(&d, 1, &a, 0, 1.0f, "to_a");
    game.connect(&b, 1, &b, 1, 1.0f, "to_b");
    game.connect(&b, 1, &a, 0, 1.0f, "to_a");
    game.connect(&c, 1, &a, 0, 1.0f, "to_a");
    game.connect(&c, 1, &f, 0, 1.0f, "to_f");
    //game.connect(&c, 1, &e, 0, 1.0f, "to_e");


    ///////////////////////////////////
	game.finishConnecting();


	/* Propositions */
	std::cout<<"Setting Atomic Propositions... "<<std::endl;
	std::vector<SimpleCondition> APs;
	std::vector<SimpleCondition*> AP_ptrs;
	for (auto& s : states) {
        SimpleCondition temp_AP;
        temp_AP.addCondition(Condition::SIMPLE, Condition::LABEL, "letter", Condition::EQUALS, Condition::VAR, s);
        temp_AP.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
        temp_AP.setLabel(s);
        std::cout<<"> Made ap: "<<s<<std::endl;
        APs.push_back(temp_AP);
	}
	AP_ptrs.resize(APs.size());
	for (int i=0; i<APs.size(); ++i) {
		AP_ptrs[i] = &APs[i];
	}

	game.setPropositions(AP_ptrs);
	std::cout<<"\n\n Printing the Game: \n\n"<<std::endl;
	game.print();


	// Get the liveness specification:

	DFA A;
	A.readFileSingle("../../spot_automaton_file_dump/dfas/dfa_complete_liveness.txt");
	A.print();
	DFA_EVAL A_eval(&A);


	RiskAvoidStrategy<State> RAS;
	Game<State>::Strategy strat = RAS.synthesize(game, &A_eval);

	std::vector<int> graph_sizes(2);
	graph_sizes[0] = game.size();
	graph_sizes[1] = A.size();
	for (int i=0; i<strat.policy.size(); ++i) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, i, ret_inds);
        int s = ret_inds[0]; // game state
		std::cout<<"action (s: "<<s<<", q: "<<ret_inds[1]<<", p: "<<i<<"): "<<strat.policy[i]<<std::endl;
	}


	return 0;
}
