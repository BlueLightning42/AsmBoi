#include "pch.h"
#include "asmBoi.h"


const char* to_string(boi::Cmd c) {
	switch (c) {
	 case boi::mov: return "mov";
	 case boi::add: return "add";
	 case boi::sub: return "sub";
	 case boi::mod: return "mod";
	 case boi::div: return "div";
	 case boi::mul: return "mul";
	 case boi::put: return "put";
	 case boi::jmp: return "jmp";
	 case boi::jma: return "jma";
	 case boi::jmc: return "jmc";
	 case boi::NOP: return "nop";
	 default: break;
	}
	return "XXX";
}
const char* to_string(boi::Reg c) {
	switch (c) {
	 case boi::A: return "A";
	 case boi::B: return "B";
	 case boi::T: return "T";
	 case boi::NUL: return " ";
	 default: break;
	}
	return "X";
}

// just helper (member) functions
int16_t AsmBoi::get_reg(boi::Reg source, Expression expression) {
	switch (source) {
	 case boi::A: return regA;
	 case boi::B: return regB;
	 case boi::T: return regT;
	 case boi::X: return expression.val;
	 default: break;
	}
	return 0;
}
void AsmBoi::set_reg(boi::Reg destination, int16_t value) {
	switch (destination) {
	 case boi::A: regA = value; break;
	 case boi::B: regB = value; break;
	 case boi::T: regT = value; break;
	 default: break; // equivalent to NOP
	}
}

void AsmBoi::evaluate() {
	program_counter = 0;
	while (program_counter < lines.size())
		step();
}
void AsmBoi::step() {
	auto line = lines[program_counter];
	int16_t temp;
	if (this->show_step) fmt::print("\nLine:{} |{} {} {} {} #{}|", program_counter,
		to_string(line.cmd), to_string(line.src1), to_string(line.src2), to_string(line.dest), line.val);
	switch (line.cmd) {
	 case boi::jmp:
		program_counter = line.val-1;
		break;
	 case boi::jma:
		program_counter += get_reg(line.src1, line);
		if (program_counter < 0) program_counter = 0;
		if (program_counter > lines.size()) program_counter = lines.size();
		break;
	 case boi::jmc:
		if (regT > 0) program_counter = line.val-1;
		break;
	 case boi::mov:
		temp = get_reg(line.src1, line);
		set_reg(line.dest, temp);
		break;
	 case boi::add:
		temp = get_reg(line.src1, line);
		temp += get_reg(line.src2, line);
		set_reg(line.dest, temp);
		break;
	 case boi::sub:
		temp = get_reg(line.src1, line);
		temp -= get_reg(line.src2, line);
		set_reg(line.dest, temp);

		break;
	 case boi::put:
		temp = get_reg(line.src1, line);
		if (show_step) {
			fmt::print(" > {}", temp);
		}else {
			fmt::print("> {}\n", temp);
		}
	default:
		break;
	}
	program_counter++;
}

