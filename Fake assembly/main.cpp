#include "pch.h"
#include "asmBoi.h"
// enspired by zachatronics games
// simplified assembly styled scripting language
// made cause I was bored in lockdown
int main() {
	AsmBoi boi("../Examples/test1.boi");
	fmt::print("Loaded fib script:\n");
	boi.evaluate();
	boi("../Examples/test2.boi");
	boi.show_step = true;
	boi.evaluate();
	return 0;
}