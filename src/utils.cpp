#include "utils.hpp"

#include <iostream>
#include <map>
#include <random>
#include <stdexcept>

using std::uint8_t;
using std::uint16_t;

chip8::opcode_handler chip8::get_handler(const struct chip8::state& state)
{
    static const std::map<uint16_t, chip8::opcode_handler> handler_map =
	{
	    {0x00E0, handle_00E0},
	    {0x00EE, handle_00EE},
	    {0x1000, handle_1nnn},
	    {0x2000, handle_2nnn},
	    {0x3000, handle_3xkk},
	    {0x4000, handle_4xkk},
	    {0x5000, handle_5xy0},
	    {0x6000, handle_6xkk},
	    {0x7000, handle_7xkk},
	    {0x8000, handle_8xy0},
	    {0x8001, handle_8xy1},
	    {0x8002, handle_8xy2},
	    {0x8003, handle_8xy3},
	    {0x8004, handle_8xy4},
	    {0x8005, handle_8xy5},
	    {0x8006, handle_8xy6},
	    {0x8007, handle_8xy7},
	    {0x9000, handle_9xy0},
	    {0xA000, handle_Annn},
	    {0xB000, handle_Bnnn},
	    {0xC000, handle_Cxkk},
	    {0xD000, handle_Dxyn},
	    {0xE000, handle_opcodes_with_leading_E},
	    {0xF000, handle_opcodes_with_leading_F},
	};
    
    uint16_t key = (state.memory.at(state.pc) & 0xF0) << 8;
    
    if (((key & 0xF000) >> 12) == 0x0)
	key |= state.memory.at(state.pc+1);
    else if (((key & 0xF000) >> 12) == 0x8)
	key |= state.memory.at(state.pc+1) & 0x0F;

    auto handler = handler_map.at(key);

    return handler;
}

void chip8::handle_00E0(struct chip8::state& state)
{
    for (int i = 0; i < chip8::display_size; ++i)
	*(state.display+i) = 0x00;
	
    state.pc += 2;
}

void chip8::handle_00EE(struct chip8::state& state)
{
    if (state.sp == chip8::stack_bottom)
	throw std::out_of_range("Attempted to pop an empty stack");

    state.sp -= 2;

    uint16_t addr_top = (state.memory.at(state.sp) & 0x0F) << 8;
    state.pc = addr_top | state.memory.at(state.sp+1);
}

void chip8::handle_1nnn(struct chip8::state& state)
{
    uint16_t addr_top = (state.memory.at(state.pc) & 0x0F) << 8;
    state.pc = addr_top | state.memory.at(state.pc+1);
}

void chip8::handle_2nnn(struct chip8::state& state)
{
    if (state.sp == chip8::stack_top)
	throw std::out_of_range("Attempted to push to a full stack");

    state.memory[state.sp] = (state.pc & 0xFF00) >> 8;
    state.memory[state.sp+1] = state.pc & 0x00FF;

    state.sp += 2;

    uint16_t addr_top = (state.memory.at(state.pc) & 0x0F) << 8;
    state.pc = addr_top | state.memory.at(state.pc+1);
}

void chip8::handle_3xkk(struct chip8::state& state)
{
    auto reg = (state.memory.at(state.pc) & 0x0F);
    auto val = state.memory.at(state.pc+1);

    if (state.V.at(reg) == val)
	state.pc += 2;

    state.pc += 2;
}

void chip8::handle_4xkk(struct chip8::state& state)
{
    auto reg = (state.memory.at(state.pc) & 0x0F);
    auto val = state.memory.at(state.pc+1);

    if (state.V.at(reg) != val)
	state.pc += 2;

    state.pc += 2;
}

void chip8::handle_5xy0(struct chip8::state& state)
{
    auto reg1 = (state.memory.at(state.pc) & 0x0F);
    auto reg2 = ((state.memory.at(state.pc+1) & 0xF0) >> 4);

    if (state.V.at(reg1) == state.V.at(reg2))
	state.pc += 2;

    state.pc += 2;
}

void chip8::handle_6xkk(struct chip8::state& state)
{
    auto reg = (state.memory.at(state.pc) & 0x0F);
    auto val = state.memory.at(state.pc+1);

    state.V[reg] = val;
    state.pc += 2;
}

void chip8::handle_7xkk(struct chip8::state& state)
{
    auto reg = state.memory.at(state.pc) & 0x0F;
    auto val = state.V.at(reg) + state.memory.at(state.pc+1);

    state.V[reg] = val;
    state.pc += 2;
}

void chip8::handle_8xy0(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    state.V[reg1] = state.V[reg2];
    state.pc += 2;
}

void chip8::handle_8xy1(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    state.V[reg1] |= state.V.at(reg2);
    state.pc += 2;
}

void chip8::handle_8xy2(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    state.V[reg1] &= state.V.at(reg2);
    state.pc += 2;
}

void chip8::handle_8xy3(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    state.V[reg1] ^= state.V.at(reg2);
    state.pc += 2;
}

void chip8::handle_8xy4(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;
    uint16_t res = state.V.at(reg1) + state.V.at(reg2);

    if (res & 0xFF00)
	state.V[0xF] = 1;
    else
	state.V[0xF] = 0;

    state.V[reg1] = res & 0xFF;
    state.pc += 2;
}

void chip8::handle_8xy5(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    if (state.V.at(reg1) > state.V.at(reg2))
	state.V[0xF] = 1;
    else
	state.V[0xF] = 0;

    state.V[reg1] -= state.V.at(reg2);
    state.pc += 2;
}

void chip8::handle_8xy6(struct chip8::state& state)
{
    auto reg = state.memory.at(state.pc) & 0xF;

    if (state.V.at(reg) & 0x1)
	state.V[0xF] = 1;
    else
	state.V[0xF] = 0;

    state.V[reg] >>= 1;
    state.pc += 2;
}

void chip8::handle_8xy7(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    if (state.V.at(reg2) > state.V.at(reg1))
	state.V[0xF] = 1;
    else
	state.V[0xF] = 0;

    state.V[reg1] = state.V.at(reg2) - state.V.at(reg1);
    state.pc += 2;
}

void chip8::handle_9xy0(struct chip8::state& state)
{
    auto reg1 = state.memory.at(state.pc) & 0x0F;
    auto reg2 = (state.memory.at(state.pc+1) & 0xF0) >> 4;

    if (state.V.at(reg1) != state.V.at(reg2))
	state.pc += 2;

    state.pc += 2;
}

void chip8::handle_Annn(struct chip8::state& state)
{
    uint16_t addr_top = (state.memory.at(state.pc) & 0x0F) << 8;
    
    state.I = addr_top | state.memory.at(state.pc+1);
    state.pc += 2;
}

void chip8::handle_Bnnn(struct chip8::state& state)
{
    uint16_t addr_top = (state.memory.at(state.pc) & 0x0F) << 8;
    auto addr = addr_top | state.memory.at(state.pc+1);
    
    state.pc = addr + state.V.at(0);
}

void chip8::handle_Cxkk(struct chip8::state& state)
{
    static std::random_device rd;
    static std::uniform_int_distribution<uint8_t> dist(0, 255);

    auto reg = state.memory.at(state.pc) & 0x0F;
    auto val = state.memory.at(state.pc+1);
    auto rand = dist(rd);

    state.V[reg] = rand & val;
    state.pc += 2;
}

void chip8::handle_Dxyn(struct chip8::state& state)
{
#ifndef CHIP8_TEST
    return;
#else
    std::cout << "handle_Dxyn\n";
#endif
}

void chip8::handle_opcodes_with_leading_E(struct chip8::state& state)
{
#ifndef CHIP8_TEST
    return;
#else
    std::cout << "handle_opcodes_with_leading_E\n";
#endif
}

void chip8::handle_opcodes_with_leading_F(struct chip8::state& state)
{
#ifndef CHIP8_TEST
    return;
#else
    std::cout << "handle_opcodes_with_leading_F\n";
#endif
}
