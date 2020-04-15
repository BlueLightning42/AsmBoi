#include "pch.h"
#include "asmBoi.h"

Expression::Expression(byte cmd, byte scr1, byte scr2, byte dest, int16_t _val) : val(_val) {
    // 000cccc222111ddd
    op_code = dest & 0b0000000000000111;
    op_code |= (scr1 << 3) & 0b0000000000111000;
    op_code |= (scr2 << 6) & 0b0000000111000000;
    op_code |= (cmd << 9) & 0b0001111000000000;
}
Expression::Expression(byte cmd, byte scr1, byte dest, int16_t _val) : val(_val) {
    // 000cccc222111ddd
    op_code = dest & 0b0000000000000111;
    op_code |= (scr1 << 3) & 0b0000000000111000;
    op_code |= (cmd << 9) & 0b0001111000000000;
}
Expression::Expression(byte cmd, int16_t _val) : val(_val) {
	op_code = (cmd << 9) & 0b0001111000000000;
}

using byte = unsigned char;
inline Cmd Expression::get_cmd() {
    return (Cmd)((op_code >> 9) & 0b0000000000001111);
}
inline Reg Expression::get_src1() {
    return (Reg)((op_code >> 3) & 0b0000000000000111);
}
inline Reg Expression::get_src2() {
    return (Reg)((op_code >> 6) & 0b0000000000000111);
}
inline Reg Expression::get_dest() {
	return (Reg)(op_code & 0b0000000000000111);
}

namespace {
	// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
            }));
    }

    // trim from end (in place)
    inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }
}
//parse
AsmBoi::AsmBoi(const char* filename) {
	std::ifstream in(filename);
	size_t lino = 0;
	for (std::string line; std::getline(in, line); lino++) {
		trim(line);
		auto coln = line.find(":");
		//extract labels
		if (coln != std::string::npos) {
			std::string label(line, 0, coln); // line[:coln:]
			labels.emplace_back(label, lino);
			line = std::string(line, coln, std::string::npos); // line[coln::]
			fmt::print("lable: {} on line: {} rest: {}", label, lino, line);
		}
		if (line[3] != ' ') {
			fmt::print("Error on line: {}", lino);
			break;
		}
		auto start = line.substr(0, 3);
		if (start == "mov") {
			int16_t val;
			Reg src;
			Reg dest;
			lines.emplace_back(Cmd::mov, src, dest, val);
		}
		if (start == "jmp") {
			//parse label 
			int val = 0;
			lines.emplace_back(Cmd::jmp, val);
		}
	}
	program_counter = 0;
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
	 case jmp:
		program_counter = line.val;
		break;
	 case jma:
		program_counter += line.val;
		if (program_counter < 0) program_counter = 0;
		if (program_counter > lines.size()) program_counter = lines.size();
		break;
	 case jmc:
		if (regT)
			program_counter = line.val;
		break;
	 case mov:
		 temp =  get_reg(line.get_src1(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case add:
		 temp =  get_reg(line.get_src1(), line);
		 temp += get_reg(line.get_src2(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case sub:
		 temp =  get_reg(line.get_src1(), line);
		 temp -= get_reg(line.get_src2(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case put:
		 temp = get_reg(line.get_src1(), line);
		 fmt::print("> {}", temp);
	 default:
		break;
	}
}
int16_t AsmBoi::get_reg(Reg source, Expression expression) {
	switch (source) {
	case A: return regA;
	case B: return regB;
	case T: return regT;
	case X: return expression.val;
	default: break;
	}
	return 0;
}
void AsmBoi::set_reg(Reg destination, int16_t value) {
	switch (destination) {
	case A: regA = value; break;
	case B: regB = value; break;
	case T: regT = value; break;
	default: break; // equivalent to NOP
	}
}
