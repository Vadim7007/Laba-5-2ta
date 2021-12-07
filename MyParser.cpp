#include "MyParser.h"


int Node::get_char() {
	if (this->t.t == symbol) return this->t.attribute;
	throw std::exception("Error 1");
}


std::string Node::get_group_name() {
	return this->t.name;
}


int Regular::tokenization(std::string templ, std::vector<token>& tokens) {
	token t1(left_par);
	tokens.push_back(t1);

	for (int i = 0; i < templ.length(); i++)
	{
		int code = int(templ[i]);
		int code1 = 0;
		int	code2 = 0;
		if (templ.length() > 1 + i) {
			code1 = int(templ[i + 1]);
		};
		if (templ.length() > 2 + i) {
			code2 = int(templ[i + 2]);
		};

		if (code == 37) { // %
			if (code2 == 37) {
				token t(symbol, code1);
				tokens.push_back(t);
				i = i + 2;
				continue;
			}
			throw i;
		}
		if (code == 124) {
			token t(or_op);
			tokens.push_back(t);
			continue;
		}
		if (code == 63) {
			token t(ques_op);
			tokens.push_back(t);
			continue;
		}
		if (code == 46) {
			if (code1 == 46 && code2 == 46) {
				token t(kl_op);
				tokens.push_back(t);
				i = i + 2;
				continue;
			}
		}
		if (code == 123) {
			int k = templ.find('}', i);
			if (k != -1) {
				int num;
				try {
					num = std::stoi(templ.substr(i + 1, k - i));
				}
				catch (std::exception& e) {
					throw i;
				}
				token t(count_op, num);
				tokens.push_back(t);
				i = k;
				continue;
			}
			throw i;
		}
		if (code == 60) {
			int k = templ.find('>', i);
			if (k != -1) {
				token t(res_name, -1, templ.substr(i + 1, k - i));
				tokens.push_back(t);
				i = k;
				continue;
			}
			throw i;
		}
		if (code == 40 && code1 == 60) {
			int k = templ.find('>', i);
			if (k != -1) {
				std::string name = templ.substr(i + 2, k - i - 2);
				token t(lname, -1, name);
				tokens.push_back(t);
				i = k + 1;

				k = templ.find(')', i);
				if (k != -1) {
					std::string buffer = templ.substr(i, k - i);
					std::vector<token> group;

					try
					{
						tokenization(buffer, group);
					}
					catch (const std::exception& e)
					{
						throw (i + int(e.what()));
					}

					tokens.insert(tokens.end(), group.begin(), group.end());

					token t(rname, -1, name);
					tokens.push_back(t);
					i = k;
					continue;
				}
			}
			throw i;
		}
		if (code == 40) {
			token t(left_par);
			tokens.push_back(t);
			continue;
		}
		if (code == 41) {
			token t(right_par, -1);
			tokens.push_back(t);
			continue;
		}
		token t(symbol, code);
		tokens.push_back(t);
	}

	token t2(right_par);
	tokens.push_back(t2);
}


void Regular::run_tree(Node* root) {

	if (root->child.size() == 2) {
		run_tree(root->child[0]);
		run_tree(root->child[1]);
		if (root->type == or_opn) {
			// создание общего старта
			State_NFA* temp_start = new State_NFA();
			temp_start->transitions.insert({ 0, root->child[0]->st });
			temp_start->transitions.insert({ 0, root->child[1]->st });

			// —оздание общего конца
			State_NFA* temp_end = new State_NFA();
			root->child[0]->st->Acceptable_state->isAcceptable = false;
			root->child[1]->st->Acceptable_state->isAcceptable = false;
			temp_end->isAcceptable = true;
			root->child[0]->st->Acceptable_state->transitions.insert({ 0, temp_end });
			root->child[1]->st->Acceptable_state->transitions.insert({ 0, temp_end });
			root->child[0]->st->Acceptable_state = temp_end;
			root->child[1]->st->Acceptable_state = temp_end;

			// прив€зывание итогового автомата с узлом or
			root->st = temp_start;
			temp_start->Acceptable_state = temp_end;
			temp_end->Acceptable_state = temp_end;
			root->st->Acceptable_state = temp_end;

		}
		else if (root->type == and_opn) {
			root->child[0]->st->Acceptable_state->isAcceptable = false;
			root->child[0]->st->Acceptable_state->transitions.insert({ 0, root->child[1]->st });
			root->child[0]->st->Acceptable_state = root->child[1]->st->Acceptable_state;

			// прив€зывание итогового автомата с узлом and 
			root->st = root->child[0]->st;
		}
		else throw std::exception("Unexpected operation for 2 operands");
	}
	else if (root->child.size() == 1) {
		if (root->type == kl_opn) {
			run_tree(root->child[0]);

			root->st = root->child[0]->st;

			// —оздание узлов
			State_NFA* temp_start = new State_NFA();
			State_NFA* temp_end = new State_NFA();
			temp_end->isAcceptable = true;
			temp_start->Acceptable_state = temp_end;
			temp_end->Acceptable_state = temp_end;
			root->st->Acceptable_state->isAcceptable = false;

			// —оздание св€зей
			temp_start->transitions.insert({ 0, temp_end });
			temp_start->transitions.insert({ 0, root->st });
			root->st->Acceptable_state->transitions.insert({ 0, root->st });
			root->st->Acceptable_state->transitions.insert({ 0, temp_end });

			root->st->Acceptable_state = temp_end;
			root->st = temp_start;
		}
		else if (root->type == name_n) {
			root->st = new State_NFA();
			auto end = root->st->create_transition(0);

			run_tree(root->child[0]);
			end->g = new group(root->get_group_name());		
			end->g->root_nfa = root->st;
			this->add_group(end->g);

			end->isAcceptable = true;
			end->Acceptable_state = end;
			root->st->Acceptable_state = end;
		}
		else if (root->type == res_name_n) {
			root->st = new State_NFA();
			auto end = root->st->create_transition(0);
			end->g = this->get_group(root->get_group_name());
			end->isAcceptable = true;
			end->Acceptable_state = end;
			root->st->Acceptable_state = end;
		}
		else throw std::exception("Unexpected operation for one operand");
	}
	else if (root->child.size() == 0) {
		if (root->type == e_str) {
			root->st = new State_NFA();
			auto end = root->st->create_transition(0);
			end->isAcceptable = true;
			end->Acceptable_state = end;
			root->st->Acceptable_state = end;
		}
		if (root->type == str) {
			root->st = new State_NFA();
			auto end = root->st->create_transition(root->get_char());
			end->isAcceptable = true;
			end->Acceptable_state = end;
			root->st->Acceptable_state = end;
		}
	}
	else {
		std::cerr << "Unexpected count of child in node: " << root->type << '\n';
	}
}


Node* subtree(std::vector<token>::iterator first, std::vector<token>::iterator end)
{
	auto par1 = first;
	auto par2 = end;

	std::vector<Node*> buffer;
	for (auto it = par1 + 1; it != par2; it++)
	{
		Node* n;
		if ((*it).t == lname)
		{
			if ((it + 1)->t != node || (it + 2)->t != rname)
			{
				throw std::exception("node");
			}
			n = new Node(*it);
			n->type = name_n;
			n->child.push_back((it + 1)->n);
			n->isintree = true;
			it += 2;
		}
		else if ((*it).t == rname) {
			throw "Unexpected token";
		}
		else if ((*it).t == node) {
			n = (*it).n;
		}
		else if ((*it).t == ques_op) {
			Node* n_ = new Node();
			n_->h_prority = true;
			n_->type = or_opn;
			buffer.push_back(n_);
			n = new Node();
			n->isintree = true;
			n->type = e_str;
		}
		else if ((*it).t == count_op) {
			for (int i = 0; i < (*it).attribute; i++)
			{
				Node* n_ = new Node(*buffer[buffer.size() - 1]);
				buffer.push_back(n_);
			}
			continue;
		}
		else if ((*it).t == symbol) {
			n = new Node(*it);
			n->isintree = true;
		}
		else {
			n = new Node(*it);
		}
		buffer.push_back(n);
	}

	Node* temp_root;

	// ques
	if (buffer.size() > 2)
	{
		for (auto j = buffer.begin() + 1; (j != buffer.end() && (j + 1) != buffer.end()); j++) {
			if ((*(j - 1))->isintree && (*j)->type == or_opn && (*j)->h_prority && (*(j + 1))->isintree) {
				(*j)->isintree = true;
				(*j)->child.push_back(*(j - 1));
				(*j)->child.push_back(*(j + 1));
				j = buffer.erase(j - 1);
				j = buffer.erase(j + 1) - 1;
			}
		}
	}

	// res_name or empty
	for (auto j = buffer.begin(); j != buffer.end(); j++) {
		if ((*j)->type == res_name_n || (*j)->type == e_str) {
			(*j)->isintree = true;
		}
	}

	// klini
	for (auto j = buffer.begin(); j != buffer.end(); j++) {
		if ((*j)->type == kl_opn) {
			(*(j - 1))->isintree = true;
			(*j)->isintree = true;
			(*j)->child.push_back(*(j - 1));
			j = buffer.erase((j - 1));
		}
	}

	// and
	for (auto j = buffer.begin() + 1; j != buffer.end(); j++) {
		if ((*j)->isintree && (*(j - 1))->isintree) {
			Node* n = new Node();
			n->isintree = true;
			n->type = and_opn;
			n->child.push_back(*(j - 1));
			n->child.push_back(*j);
			auto res = buffer.insert(j - 1, n);
			buffer.erase(res + 1, res + 3);
			j = buffer.begin();
		}
	}

	// or
	if (buffer.size() > 2)
	{
		for (auto j = buffer.begin() + 1; (j != buffer.end() && (j + 1) != buffer.end()); j++) {
			if ((*(j - 1))->isintree && (*j)->type == or_opn && (*(j + 1))->isintree) {
				(*j)->isintree = true;
				(*j)->child.push_back(*(j - 1));
				(*j)->child.push_back(*(j + 1));
				j = buffer.erase(j - 1);
				j = buffer.erase(j + 1) - 1;
			}
		}
	}

	if (buffer.size() == 1) {
		temp_root = buffer[0];
	}
	else {
		throw buffer.size();
	}
	return temp_root;
};


bool Regular::syntax_tree() {

	while (tokens.size() != 1)
	{
		// Ќаходим ближайшую пару
		int min = tokens.size();
		int begin = 0;
		int end = tokens.size();
		int begin_c = begin;
		int end_c = end;

		std::vector<token>::iterator par1;
		std::vector<token>::iterator par2;

		for (int i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].t == 9) {
				begin = i;
			}
			if (tokens[i].t == 10) {
				end = i;
				if (end - begin < min) {
					min = end - begin;
					par1 = tokens.begin() + begin;
					par2 = tokens.begin() + end;
					begin_c = begin;
					end_c = end;
				}
			}
		}

		Node* temp_root = subtree(par1, par2);
		token t;
		t.t = node;
		t.n = temp_root;
		tokens.insert(par1, t);
		par1 = tokens.begin() + begin_c + 1;
		par2 = tokens.begin() + end_c + 2;

		tokens.erase(par1, par2);
	}

	this->root = tokens[0].n;

	return true;
}


std::set<State_NFA*> create_eclosure(std::set<State_NFA*> states) {
	std::set<State_NFA*> e_cl;
	std::set< State_NFA* > next_tr;

	for (auto i = states.begin(); i != states.end(); i++) {

		e_cl.insert(*i);
		auto st = *i;

		for (auto i = st->transitions.begin(); i != st->transitions.end(); i++) {
			if (i->first == 0) {
				next_tr.insert((State_NFA*)i->second);
			}
		}
	}
	if (next_tr.size() != 0) {
		e_cl = create_eclosure(next_tr);
	}
	return e_cl;
}


std::set<State_NFA*> create_eclosure(State_NFA* st) {
	std::set<State_NFA*> e_cl;
	std::set< State_NFA* > next_tr;

	e_cl.insert(st);

	for (auto i = st->transitions.begin(); i != st->transitions.end(); i++) {
		if (i->first == 0) {
			next_tr.insert((State_NFA*)i->second);
		}
	}

	e_cl = create_eclosure(next_tr);
	return e_cl;
}


std::set<State_NFA*> get_transitions_st(State_NFA* st, int signal) {
	std::set<State_NFA*> result;
	for (auto i = st->transitions.begin(); i != st->transitions.end(); i++)
	{
		if (i->first == signal) {
			result.insert((State_NFA*)i->second);
		}
	}
	return result;
}


State_DFA* search_set(const std::set<State_NFA*>& key, const std::vector<State_DFA*>& arr) {
	for (auto i = arr.begin(); i != arr.end(); i++)
	{
		if ((*i)->get_NFAs() == key) return *i;
	}
	return nullptr;
}


State_DFA* run_NFA(State_NFA* start) {

	std::vector<State_DFA*> DFA_states;
	
	std::set<State_NFA*> e_closure = create_eclosure(start);
	State_DFA* dst = new State_DFA(e_closure);
	DFA_states.push_back(dst);
	int index = 0;
	auto dstate = DFA_states.begin();

	do
	{	
		dstate = DFA_states.begin()+index;
		std::set<State_NFA*>e_closure = (*dstate)->get_NFAs();
		for (auto i = e_closure.begin(); i != e_closure.end(); i++) {
			for (int c = MIN_ABC_ITEM; c < MAX_ABC_ITEM + 1; c++) {
				std::set<State_NFA*> next_states = get_transitions_st(*i, c);
				next_states = create_eclosure(next_states);
				auto next_state_DFA = search_set(next_states, DFA_states);
				if (!next_state_DFA) {
					State_DFA* new_st = new State_DFA(next_states);
					DFA_states.push_back(new_st);
					(*i)->transitions.insert({ c, new_st });
				}
				else {
					(*i)->transitions.insert({ c, next_state_DFA});
				}
			}
		}
		DFA_states[index]->is_processed = true;
		index++;
	} while (!((*(DFA_states.end() - 1))->is_processed));

	return *DFA_states.begin();
}


State_NFA::State_NFA(int state) {
	isAcceptable = false;
	current_state = state;
}


State_NFA* State_NFA::create_transition(int signal) {
	State_NFA* n = new State_NFA();
	n->current_state = signal;
	this->transitions.insert({ signal, n });
	return n;
}


void Regular::add_group(group* g) {
	groups.insert({g->name, g});
}


group* Regular::get_group(std::string name) {
	auto result = groups.find(name);
	if (result == std::end(groups)) return nullptr;
	return result->second;
}


bool Regular::create_NFA() {
	run_tree(this->root);
	NFA_start = this->root->st;
	return true;
}


bool Regular::create_DFA() {
	this->DFA_start = run_NFA(this->NFA_start);
	for (auto i = this->groups.begin(); i != this->groups.end(); i++) {
		(i->second)->root_dfa = run_NFA((i->second)->root_nfa);
	}
	return true;
}


void Regular::minimize_DFA() {

}


bool Regular::compile() {
	create_NFA();
	create_DFA();
	minimize_DFA();
	return true;
}


bool Regular::match(std::string str) {

	return true;
}
