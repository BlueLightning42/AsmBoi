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

namespace {
	// stolen from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch) && !(ch == (int)(','));
			}));
	}

	// trim from end (in place)
	inline void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch) && !(ch == (int)(','));
            }).base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }
}

namespace {
	std::string extractWord(std::string &line, size_t &found) {
		line.erase(0, found); //erase previous word
		ltrim(line); //erase starting whitespace + comma
		const auto search_str = " ,\t\n\r\f\v";
		found = line.find_first_of(search_str);
		auto opr1 = line.substr(0, found);
	}
	boi::Reg parseReg(std::string r) {
		if (r.size() > 1) return boi::X;
		int c = std::tolower(r.front());
		if (c = (int)('a')) return boi::A;
		if (c = (int)('b')) return boi::B;
		if (c = (int)('t')) return boi::T;
		return boi::X;
	}
	inline bool isACmd3big(const std::string &command) {
		const std::vector<const char*> cmds_3_long = { "add","sub","mod","div","mul" };
		return std::any_of(cmds_3_long.begin(), cmds_3_long.end(),
			[&](const char* cmd) {return command == cmd; });
	}
	inline bool isACmd1big(const std::string& command) {
		const std::vector<const char*> cmds_1_long = { "jmp", "jnc", "put" };
		return std::any_of(cmds_1_long.begin(), cmds_1_long.end(),
			[&](const char* cmd) {return command == cmd; });
	}
}

//parse
AsmBoi::AsmBoi(const char* filename): regA(0), regB(0), regT(0) {
	std::ifstream in(filename);
	size_t lino = 0;
	std::string command(3, 'x');
	boi::Cmd cmd;

	for (std::string line; std::getline(in, line); ) {

		// trash comments
		size_t found = line.find(";");
		if (found != std::string::npos) {
			line = std::string(line, 0, found);
		}
		trim(line);
		found = line.find(":");
		//extract labels
		if (found != std::string::npos) {
			std::string label(line, 0, found); // line[:found:]
			labels.emplace_back(label, lino);
			line.erase(0, found + 1);
			fmt::print("\nlabel: '{}' on line: {} rest: '{}'", label, lino, line);
		}
		if (line.size() < 1) continue; //no error empty lines are allowed
		if (line.size() < 4 || !std::isspace(line[3])) { //error
			fmt::print("\nError on line: {}\nbad command must be exactly 3 characters", lino);
			break;
		}
		lino++;
		std::transform(line.begin(), line.begin()+3, command.begin(),
			[](unsigned char c) { return std::tolower(c); }); // cmd = line[:3:].lower()
		// only command operations
		if (command == "nop") {
			lines.emplace_back(boi::Cmd::add, 0, 0, boi::Reg::T);
			continue;
		}
		// 1 operand commands
		found = 3; //command is 3
		auto opr1 = extractWord(line, found);
		fmt::print("\ncommand: '{}' opr1: '{}'", command, opr1);
		if (isACmd1big(command)){
			if (command == "jmp") cmd = boi::jmp;
			if (command == "jmc") cmd = boi::jmc;
			if (command == "put") cmd = boi::put;
			auto val = find_if(labels.begin(), labels.end(), 
				[&opr1](std::pair<std::string, size_t> l) { return l.first == opr1; });

			if (val == labels.end()) {
				fmt::print("Tried to jump to a label that doesnt exist");
				break;
			}
			lines.emplace_back(cmd, val);
		}
		// 2 operand commands
		auto opr2 = extractWord(line, found); 
		fmt::print("opr2: '{}'", opr2);
		if (command == "mov") {
			int16_t val;
			boi::Reg src = parseReg(opr1);
			if (src == boi::X) {
				val = std::stoi(opr1);
			}else {
				val = 0;
			}
			boi::Reg dest = parseReg(opr2); //if x than noop
			lines.emplace_back(boi::Cmd::mov, src, dest, val);
		}
		// 3 operand commands
		auto opr3 = extractWord(line, found);
		fmt::print("opr3: '{}'", opr3);
		if (isACmd3big(command)) {
			int16_t val;
			boi::Reg src = parseReg(opr1);
			if (src == boi::X) {
				val = std::stoi(opr1);
			}
			else {
				val = 0;
			}
			boi::Reg dest = parseReg(opr2); //if x than noop
			lines.emplace_back(boi::Cmd::mov, src, dest, val);
		}
	}
	fmt::print("Loaded\n");
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
	case boi:: jmp:
		program_counter = line.val;
		break;
	 case boi:: jma:
		program_counter += line.val;
		if (program_counter < 0) program_counter = 0;
		if (program_counter > lines.size()) program_counter = lines.size();
		break;
	 case boi:: jmc:
		if (regT)
			program_counter = line.val;
		break;
	 case boi:: mov:
		 temp =  get_reg(line.get_src1(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case boi:: add:
		 temp =  get_reg(line.get_src1(), line);
		 temp += get_reg(line.get_src2(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case boi:: sub:
		 temp =  get_reg(line.get_src1(), line);
		 temp -= get_reg(line.get_src2(), line);
		 set_reg(line.get_dest(), temp);
		 break;
	 case boi:: put:
		 temp = get_reg(line.get_src1(), line);
		 fmt::print("> {}", temp);
	 default:
		break;
	}
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
