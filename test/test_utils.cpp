#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstdint>

#include "utils.hpp"

using std::uint8_t;

TEST_CASE("test state struct", "[initialization]")
{
    struct chip8::state state;
    
    REQUIRE(state.V.size() == 16);
    REQUIRE(state.pc == 0x0200);
    REQUIRE(state.sp == 0x0EA0);
}

TEST_CASE("test opcode handlers", "[opcode_handlers]")
{
    struct chip8::state state;

    SECTION("handle_00E0")
    {
	state.memory[state.pc] = 0x00;
	state.memory[state.pc+1] = 0xE0;

	for (int i = 0; i < chip8::display_size; ++i)
	    *(state.display+i) = 0x01;

	for (int i = 0; i < chip8::display_size; ++i)
	    REQUIRE(*(state.display+i) == 0x01);

	auto handle = chip8::get_handler(state);
	handle(state);

	for (int i = 0; i < chip8::display_size; ++i)
	    REQUIRE(*(state.display+i) == 0x00);

	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_00EE")
    {
	state.memory[state.pc] = 0x00;
	state.memory[state.pc+1] = 0xEE;

	state.memory[state.sp] = 0x0A;
	state.memory[state.sp+1] = 0xAA;
	state.sp += 2;

	state.memory[0x0AAA] = 0xFE;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.pc == 0x0AAA);
	REQUIRE(state.sp == 0x0EA0);
	REQUIRE(state.memory.at(state.pc) == 0xFE);
    }

    SECTION("handle_1nnn")
    {
	state.memory[state.pc] = 0x13;
	state.memory[state.pc+1] = 0x32;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.pc == 0x0332);
    }

    SECTION("handle_2nnn")
    {
	state.memory[state.pc] = 0x23;
	state.memory[state.pc+1] = 0x32;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.pc == 0x0332);
	REQUIRE(state.sp == 0x0EA0+2);
	REQUIRE(state.memory.at(state.sp-2) == 0x02);
	REQUIRE(state.memory.at(state.sp-1) == 0x00);
    }

    SECTION("handle_3xkk")
    {
	uint8_t reg = 6;
	uint8_t val = 0x54;
	
	state.memory[state.pc] = 0x30 + reg;
	state.memory[state.pc+1] = val;
	state.V[reg] = val;

	REQUIRE(state.V.at(reg) == val);

	SECTION("register value and kk equal")
	{
	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+4);
	}

	SECTION("register value and kk not equal")
	{
	    state.memory[state.pc+1] += 1;

	    REQUIRE(state.memory.at(state.pc+1) == val+1);

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_4xkk")
    {
	uint8_t reg = 6;
	uint8_t val = 0x54;
	
	state.memory[state.pc] = 0x40 + reg;
	state.memory[state.pc+1] = val;
	state.V[reg] = val;

	REQUIRE(state.V.at(reg) == val);

	SECTION("register value and kk not equal")
	{
	    state.memory[state.pc+1] += 1;

	    REQUIRE(state.memory.at(state.pc+1) == val+1);

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+4);
	}

	SECTION("register value and kk equal")
	{
	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_5xy0")
    {
	uint8_t reg1 = 6;
	uint8_t reg2 = 0xB;
	uint8_t val = 0x54;
	
	state.memory[state.pc] = 0x50 + reg1;
	state.memory[state.pc+1] = reg2 << 4;
	state.V[reg1] = val;
	state.V[reg2] = val;

	REQUIRE(state.V.at(reg1) == val);
	REQUIRE(state.V.at(reg2) == val);

	SECTION("register values equal")
	{
	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+4);
	}

	SECTION("register values not equal")
	{
	    state.V[reg2] += 1;

	    REQUIRE(state.V.at(reg2) == val+1);

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_6xkk")
    {
	uint8_t reg = 0;
	uint8_t val = 0xFF;

	state.memory[state.pc] = 0x60 + reg;
	state.memory[state.pc+1] = val;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg) == val);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_7xkk")
    {
	uint8_t reg = 0xF;
	uint8_t val = 0xFE;

	state.memory[state.pc] = 0x70 + reg;
	state.memory[state.pc+1] = val;
	state.V[reg] = 0x01;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg) == 0xFF);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_8xy0")
    {
	uint8_t reg1 = 0x2;
	uint8_t reg2 = 0x4;
	uint8_t val = 0xAB;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = reg2 << 4;
	state.V[reg2] = val;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == val);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_8xy1")
    {
	uint8_t reg1 = 0x2;
	uint8_t reg2 = 0x4;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x1;
	state.V[reg1] = 0x09;
	state.V[reg2] = 0x0A;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == 0x0B);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_8xy2")
    {
	uint8_t reg1 = 0xD;
	uint8_t reg2 = 0xC;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x2;
	state.V[reg1] = 0x0B;
	state.V[reg2] = 0x0C;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == 0x08);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_8xy3")
    {
	uint8_t reg1 = 0xD;
	uint8_t reg2 = 0xC;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x3;
	state.V[reg1] = 0x09;
	state.V[reg2] = 0x0A;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == 0x03);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_8xy4")
    {
	uint8_t reg1 = 0xD;
	uint8_t reg2 = 0xC;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x4;

	SECTION("register sum > 255")
	{
	state.V[reg1] = 0xFF;
	state.V[reg2] = 0xFF;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == 0xFE);
	REQUIRE(state.V.at(0xF) == 1);
	REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("register sum <= 255")
	{
	state.V[reg1] = 0x0E;
	state.V[reg2] = 0x01;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.V.at(reg1) == 0x0F);
	REQUIRE(state.V.at(0xF) == 0);
	REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_8xy5")
    {
	uint8_t reg1 = 0xD;
	uint8_t reg2 = 0xC;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x5;

	SECTION("V[x] > V[y]")
	{
	    state.V[reg1] = 0x0F;
	    state.V[reg2] = 0x09;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg1) == 0x06);
	    REQUIRE(state.V.at(0xF) == 1);
	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("V[x] <= V[y]")
	{
	    state.V[reg1] = 0x09;
	    state.V[reg2] = 0x0F;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    // TODO: What should be stored in V[x] since it's the result of subtracting a larger uint from a smaller one?
	    // REQUIRE(state.V.at(reg1) == ???);
	    REQUIRE(state.V.at(0xF) == 0);
	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_8xy6")
    {
	uint8_t reg = 0xD;

	state.memory[state.pc] = 0x80 + reg;
	state.memory[state.pc+1] = 0x06;

	SECTION("lsb == 1")
	{
	    state.V[reg] = 0x3;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg) == 0x1);
	    REQUIRE(state.V.at(0xF) == 1);
	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("lsb != 1")
	{
	    state.V[reg] = 0x2;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg) == 0x1);
	    REQUIRE(state.V.at(0xF) == 0);
	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_8xy7")
    {
	uint8_t reg1 = 0xD;
	uint8_t reg2 = 0xC;

	state.memory[state.pc] = 0x80 + reg1;
	state.memory[state.pc+1] = (reg2 << 4) | 0x7;

	SECTION("V[y] > V[x]")
	{
	    state.V[reg1] = 0x09;
	    state.V[reg2] = 0x0F;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg1) == 0x06);
	    REQUIRE(state.V.at(0xF) == 1);
	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("V[y] <= V[x]")
	{
	    state.V[reg1] = 0x0F;
	    state.V[reg2] = 0x09;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    // TODO: What should be stored in V[x] since it's the result of subtracting a larger uint from a smaller one?
	    // REQUIRE(state.V.at(reg1) == ???);
	    REQUIRE(state.V.at(0xF) == 0);
	    REQUIRE(state.pc == 0x0200+2);
	}
    }

    SECTION("handle_8xyE")
    {
	uint8_t reg = 0xD;

	state.memory[state.pc] = 0x80 + reg;
	state.memory[state.pc+1] = 0x0E;

	SECTION("msb == 1")
	{
	    state.V[reg] = 0x81;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg) == 0x2);
	    REQUIRE(state.V.at(0xF) == 1);
	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("msb != 1")
	{
	    state.V[reg] = 0x40;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(reg) == 0x80);
	    REQUIRE(state.V.at(0xF) == 0);
	    REQUIRE(state.pc == 0x0200+2);
	}
    }
    
    SECTION("handle_9xy0")
    {
	uint8_t reg1 = 0x2;
	uint8_t reg2 = 0x0;
	uint8_t val = 0xFF;

	state.memory[state.pc] = 0x90 + reg1;
	state.memory[state.pc+1] = reg2 << 4;

	SECTION("register values equal")
	{
	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("register values not equal")
	{
	    state.V[reg1] = val;
	    
	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.pc == 0x0200+4);
	}	
    }

    SECTION("handle_Annn")
    {
	state.memory[state.pc] = 0xA5;
	state.memory[state.pc+1] = 0x55;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.I == 0x0555);
	REQUIRE(state.pc == 0x0200+2);
    }

    SECTION("handle_Bnnn")
    {
	state.memory[state.pc] = 0xB5;
	state.memory[state.pc+1] = 0x55;
	state.V[0] = 0x03;

	auto handle = chip8::get_handler(state);
	handle(state);

	REQUIRE(state.pc == 0x0558);
    }

    SECTION("handle_Cxkk")
    {
	state.memory[state.pc] = 0x9D;

	SECTION("value 0x00")
	{
	    state.memory[state.pc+1] = 0x00;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(0xD) == 0x00);
	    REQUIRE(state.pc == 0x0200+2);
	}

	SECTION("value 0xFF")
	{
	    state.memory[state.pc+1] = 0xFF;

	    auto handle = chip8::get_handler(state);
	    handle(state);

	    REQUIRE(state.V.at(0xD) >= 0x00);
	    REQUIRE(state.V.at(0xD) <= 0xFF);
	    REQUIRE(state.pc == 0x0200+2);
	}
    }
}

