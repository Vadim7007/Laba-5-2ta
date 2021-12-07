#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>

#define MIN_ABC_ITEM 32
#define MAX_ABC_ITEM 126

enum node_type
{
	kl_opn,
	and_opn,
	name_n,
	res_name_n,
	or_opn,
	str,
	e_str
};


enum token_type
{
	symbol,
	or_op,
	and_op,
	kl_op, // замыкание Клини - *
	ques_op,
	count_op,
	lname,
	rname,
	res_name,
	left_par,
	right_par,
	node
};


class Node;
class group;

struct token
{
	token() {};
	token(token_type t_, int a_ = -1, std::string str = "") {
		t = t_;
		attribute = a_;
		name = str;
	}
	token_type t;
	int attribute;
	std::string name;
	Node* n;
};


class State {
public:
	int current_state;
	bool isAcceptable;
	group* g;
	State() {
		isAcceptable = false;
		g = nullptr;
		current_state = 0;
	};
	~State() {};
	std::set<std::pair<int, State*>> transitions;
};


class State_NFA: public State
{
public:
	State_NFA* Acceptable_state;

	State_NFA* create_transition(int);
	State_NFA(int state = -1);
	~State_NFA() {};
};


class State_DFA: public State
{
public:
	State_DFA():
	is_processed(false)
	{};
	State_DFA(std::set<State_NFA*> s):
	NFAs(s),
	is_processed(false)
	{};
	~State_DFA() {};
	std::set<State_NFA*> get_NFAs(){
		return NFAs;
	}

	bool is_processed;
private:
	std::set<State_NFA*> NFAs;
};


class Node
{
public:
	Node() {
		type = str;
		isintree = false;
		h_prority = false;
	};
	Node(token& t):
	t(t)
	{
		isintree = false;
		h_prority = false;
		if (t.t == symbol) {
			type = str;
		}
		if (t.t == or_op) {
			type = or_opn;
		}
		if (t.t == and_op) {
			type = and_opn;
		}
		if (t.t == kl_op) {
			type = kl_opn;
		}
		if (t.t == res_name) {
			type = res_name_n;
		}
	};
	~Node() {};
	int get_char();
	std::string get_group_name();
	std::vector<Node*> child;
	node_type type;
	bool isintree;
	State_NFA* st;
	bool h_prority;
private:
	token t;
};


class group {
public:
	group(std::string name):
	name(name)
	{};
	~group() {};

	const std::string name;
	State_NFA* root_nfa;
	State_DFA* root_dfa;
	std::string result;
};


class Regular {
public:
	Regular(std::string templ):
	templ(templ),
	root(nullptr)
	{
		try
		{
			tokenization(templ, tokens);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error in syntax, position: " << e.what();
		}

		syntax_tree();

	}
	~Regular() {};

	bool compile();
	bool match(std::string str);
	void add_group(group*);
	group* get_group(std::string name);
private:
	int tokenization(std::string, std::vector<token>&);
	bool syntax_tree();
	bool create_NFA();
	bool create_DFA();
	void minimize_DFA();

	void run_tree(Node* root);

	const std::string templ;
	std::vector<token> tokens;
	Node* root;
	State_NFA* NFA_start;
	State_DFA* DFA_start;
	std::map<std::string, group*> groups;
};