#include "MyParser.h"


int main(int argc, char* argv[]) {
	// Regular r("a?b|f...%|%");
	Regular r("a (<name>r) <name> a?z...");
	r.compile();
	return 0;
}