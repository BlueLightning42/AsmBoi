#pragma once
#include "pch.h"


// there are defenitly better ways of doing this but having the command encoded in the expression instead of deduced from the stuff around it feels better
namespace {
	enum Cmd {
		mov,
		add,
		sub,
		put,
		jmp,
		jma, //jump absolute
		jmc //jump conditional
	};
	enum Reg { A, B, X, T, OUT1, OUT2, OUT3, OUT4, IN1, IN2, IN3, IN4 };
}
struct Expression {
	uint16_t op_code;
	int16_t val;
	using byte = unsigned char;
	Expression(byte cmd, byte scr1, byte scr2, byte dest, int16_t _val);
	Expression(byte cmd, byte scr1, byte dest, int16_t _val);
	Expression(byte cmd, int16_t _val);
	Cmd get_cmd();
	Reg get_src1();
	Reg get_src2();
	Reg get_dest();
};
class AsmBoi{
public:
	AsmBoi(const char* filename);
	void evaluate();
	void step();
private:
	int16_t get_reg(Reg, Expression expression);
	void set_reg(Reg, int16_t);
	int16_t regA;
	int16_t regB;
	int16_t regT;
	size_t program_counter;
	std::vector<Expression> lines;
	std::vector<std::pair<std::string, size_t>> labels;
};

