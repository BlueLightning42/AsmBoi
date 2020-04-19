#include "pch.h"
#include "asmBoi.h"

using byte = unsigned char;

Expression::Expression(byte _cmd, byte _src1, byte _src2, byte _dest, int16_t _val) : val(_val) {
    this->dest = (boi::Reg)(_dest & 0b111);
	this->src1 = (boi::Reg)(_src1 & 0b111);
	this->src2 = (boi::Reg)(_src2 & 0b111);
    this->cmd =  (boi::Cmd)(_cmd & 0b1111);
}
Expression::Expression(byte _cmd, byte _src1, byte _dest, int16_t _val) : val(_val), src2(boi::NUL) {
	this->dest = (boi::Reg)(_dest & 0b111);
	this->src1 = (boi::Reg)(_src1 & 0b111);
	this->cmd = (boi::Cmd)(_cmd & 0b1111);
}
Expression::Expression(byte _cmd, byte _src1, int16_t _val) : val(_val), src2(boi::NUL), dest(boi::NUL) {
	this->src1 = (boi::Reg)(_src1 & 0b111);
	this->cmd = (boi::Cmd)(_cmd & 0b1111);
}
Expression::Expression(byte _cmd, int16_t _val) : val(_val), src1(boi::NUL), src2(boi::NUL), dest(boi::NUL) {
	this->cmd = (boi::Cmd)(_cmd & 0b1111);
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

	std::string extractWord(std::string &line, size_t &found) {
		line.erase(0, found); //erase previous word
		ltrim(line); //erase starting whitespace + comma
		const auto search_str = " ,\t\n\r\f\v";
		found = line.find_first_of(search_str);
		return line.substr(0, found);
	}
	boi::Reg parseReg(std::string r) {
		if (r.size() > 1) return boi::X;
		int c = std::tolower(r.front());
		if (c == (int)('a')) return boi::A;
		if (c == (int)('b')) return boi::B;
		if (c == (int)('t')) return boi::T;
		return boi::X;
	}
	inline bool isACmd3big(const std::string &command) {
		const std::vector<const char*> cmds_3_long = { "add","sub","mod","div","mul" };
		return std::any_of(cmds_3_long.begin(), cmds_3_long.end(),
			[&](const char* cmd) {return command == cmd; });
	}
	inline bool isACmd1big(const std::string& command) {
		const std::vector<const char*> cmds_1_long = { "jmp", "jmc", "put", "jma" };
		return std::any_of(cmds_1_long.begin(), cmds_1_long.end(),
			[&](const char* cmd) {return command == cmd; });
	}
}

//parse as setup
AsmBoi::AsmBoi(const char* filename): regA(0), regB(0), regT(0), program_counter(0), show_step(false) {
	std::ifstream in(filename);
	parse(in);
}
AsmBoi::AsmBoi(std::ifstream& in) : regA(0), regB(0), regT(0), program_counter(0), show_step(false) {
	parse(in);
}
//remake the whole object...
void AsmBoi::operator()(const char* filename) {
	std::ifstream in(filename);
	*this = AsmBoi(in);
}
void AsmBoi::operator()(std::ifstream& in) {
	*this = AsmBoi(in);
}
//TODO: split into several inline functions to increase readability.
void AsmBoi::parse(std::ifstream &in) {
	int16_t lino = 0;
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
		}
		if (line.size() < 1) continue; //no error empty lines are allowed
		if (line.size() < 4 || !std::isspace(line[3])) { //error
			fmt::print(stderr, "\nError on line: {}\nbad command must be exactly 3 characters", lino);
			break;
		}
		lino++;
		std::transform(line.begin(), line.begin() + 3, command.begin(),
			[](unsigned char c) { return std::tolower(c); }); // cmd = line[:3:].lower()
		// only command operations
		if (command == "nop") {
			lines.emplace_back(boi::Cmd::NOP, -1);
			continue;
		}
		// 1 operand commands
		found = 3; //command is 3
		auto opr1 = extractWord(line, found);
		if (isACmd1big(command)) {
			if (command == "put" || command == "jma") {
				if (command == "put") cmd = boi::put;
				if (command == "jma") cmd = boi::jma;
				boi::Reg src = parseReg(opr1);
				int16_t val;
				if (src == boi::X) {
					val = std::stoi(opr1);
				}
				else {
					val = 0;
				}
				lines.emplace_back(cmd, src, val);
			}
			else {
				if (command == "jmp") cmd = boi::jmp;
				if (command == "jmc") cmd = boi::jmc;
				auto val = find_if(labels.begin(), labels.end(),
					[&opr1](std::pair<std::string, int16_t> l) { return l.first == opr1; });
				if (val == labels.end()) {
					fmt::print(stderr, "\nTried to jump to a label that doesnt exist\n");
					break;
				}
				lines.emplace_back(cmd, val->second);
			}
			continue;
		}
		// 2 operand commands
		auto opr2 = extractWord(line, found);
		if (command == "mov") {
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
			continue;
		}
		// 3 operand commands
		auto opr3 = extractWord(line, found);
		if (isACmd3big(command)) {
			if (command == "add") cmd = boi::add;
			if (command == "sub") cmd = boi::sub;
			if (command == "div") cmd = boi::div;
			if (command == "mul") cmd = boi::mul;
			if (command == "mod") cmd = boi::mod;
			int16_t val;
			boi::Reg src = parseReg(opr1);
			boi::Reg src2 = parseReg(opr2); //if x than noop
			boi::Reg dest = parseReg(opr3); //if x than noop
			if (dest == boi::X) {
				lines.emplace_back(boi::NOP, -1 * cmd);
				break;
			}
			if (src == boi::X) {
				if (src2 == boi::X) {
					fmt::print(stderr, "both operands are integer values- consider using a mov?");
					break;
				}
				val = std::stoi(opr1);
			}
			else if (src2 == boi::X) {
				val = std::stoi(opr2);
			}
			else {
				val = 0;
			}
			lines.emplace_back(cmd, src, src2, dest, val);
			continue;
		}
	}
}