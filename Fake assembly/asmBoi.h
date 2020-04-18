#pragma once
#include "pch.h"


// there are defenitly better ways of doing this but having the command encoded in the expression instead of deduced from the stuff around it feels better
namespace boi{
	using byte = unsigned char;
	enum Cmd : byte {
		mov,
		add,
		sub,
		mod,
		div,
		mul,
		put,
		jmp,
		jma, // jump absolute
		jmc  // jump conditional
	};
	enum Reg : byte { A, B, X, T, OUT1, OUT2, OUT3, OUT4, IN1, IN2, IN3, IN4 };
}
struct Expression {
	using byte = unsigned char;
	uint16_t op_code;
	int16_t val;
	Expression(byte cmd, byte scr1, byte scr2, byte dest, int16_t _val);
	Expression(byte cmd, byte scr1, byte dest, int16_t _val);
	Expression(byte cmd, byte scr1, int16_t _val);
	Expression(byte cmd, int16_t _val);
	boi::Cmd get_cmd();
	boi::Reg get_src1();
	boi::Reg get_src2();
	boi::Reg get_dest();
};
class AsmBoi{
public:
	AsmBoi(const char* filename);
	void evaluate();
	void step();
private:
	int16_t get_reg(boi::Reg, Expression expression);
	void set_reg(boi::Reg, int16_t);
	int16_t regA;
	int16_t regB;
	int16_t regT;
	size_t program_counter;
	std::vector<Expression> lines;
	std::vector<std::pair<std::string, int16_t>> labels;
};

