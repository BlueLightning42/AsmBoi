#include "pch.h"
#include "asmBoi.h"

inline boi::Cmd Expression::get_cmd() {
    return (boi::Cmd)((op_code >> 9) & 0b0000000000001111);
}
inline boi::Reg Expression::get_src1() {
    return (boi::Reg)((op_code >> 3) & 0b0000000000000111);
}
inline boi::Reg Expression::get_src2() {
    return (boi::Reg)((op_code >> 6) & 0b0000000000000111);
}
inline boi::Reg Expression::get_dest() {
    return (boi::Reg)(op_code & 0b0000000000000111);
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
	auto cmd = line.get_cmd();
	int16_t temp;
	switch (cmd) {
	case boi::jmp:
		program_counter = line.val;
		break;
	case boi::jma:
		program_counter += line.val;
		if (program_counter < 0) program_counter = 0;
		if (program_counter > lines.size()) program_counter = lines.size();
		break;
	case boi::jmc:
		if (regT)
			program_counter = line.val;
		break;
	case boi::mov:
		temp = get_reg(line.get_src1(), line);
		set_reg(line.get_dest(), temp);
		break;
	case boi::add:
		temp = get_reg(line.get_src1(), line);
		temp += get_reg(line.get_src2(), line);
		set_reg(line.get_dest(), temp);
		break;
	case boi::sub:
		temp = get_reg(line.get_src1(), line);
		temp -= get_reg(line.get_src2(), line);
		set_reg(line.get_dest(), temp);
		break;
	case boi::put:
		temp = get_reg(line.get_src1(), line);
		fmt::print("> {}", temp);
	default:
		break;
	}
}

