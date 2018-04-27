#ifndef CHIP8_UTILS_H_
#define CHPI8_UTILS_H_

#include <array>
#include <cstdint>

namespace chip8
{
    const std::uint16_t stack_bottom = 0x0EA0;
    const std::uint16_t stack_top = stack_bottom + 0xFF;
    const std::uint16_t display_start = 0x0F00;
    const int display_size = 64*32/8;
    
    struct state
    {
	std::array<std::uint8_t, 4096> memory{0};
	std::array<std::uint8_t, 0xF+1> V{0};
	std::uint16_t I{0};
	std::uint8_t delay_timer{0};
	std::uint8_t sound_timer{0};
	std::uint16_t pc{0x200};
	std::uint16_t sp{stack_bottom};
	std::uint8_t *display = &(memory.at(display_start));
    };

    using opcode_handler = void (*)(struct state&);

    opcode_handler get_handler(const struct state&);

    void handle_00E0(struct state&);
    void handle_00EE(struct state&);
    void handle_1nnn(struct state&);
    void handle_2nnn(struct state&);
    void handle_3xkk(struct state&);
    void handle_4xkk(struct state&);
    void handle_5xy0(struct state&);
    void handle_6xkk(struct state&);
    void handle_7xkk(struct state&);
    void handle_8xy0(struct state&);
    void handle_8xy1(struct state&);
    void handle_8xy2(struct state&);
    void handle_8xy3(struct state&);
    void handle_8xy4(struct state&);
    void handle_8xy5(struct state&);
    void handle_8xy6(struct state&);
    void handle_8xy7(struct state&);
    void handle_9xy0(struct state&);
    void handle_Annn(struct state&);
    void handle_Bnnn(struct state&);
    void handle_Cxkk(struct state&);
    void handle_Dxyn(struct state&);
    void handle_opcodes_with_leading_E(struct state&);
    void handle_opcodes_with_leading_F(struct state&);
}

#endif // CHIP8_UTILS_H_
