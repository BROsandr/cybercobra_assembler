#pragma once

#include "cobra.hpp"

#include <cstddef>

#include <string>
#include <map>

class Decoder {
  public:
    enum class Concrete_instruction {
        instr_add      ,
        instr_sub      ,
        instr_xor      ,
        instr_or       ,
        instr_and      ,
        instr_sra      ,
        instr_srl      ,
        instr_sll      ,
        instr_blt      ,
        instr_bltu     ,
        instr_bge      ,
        instr_bgeu     ,
        instr_beq      ,
        instr_bne      ,
        instr_slts     ,
        instr_sltu     ,
        instr_const    ,
        instr_periphery,
        instr_jump     ,

        instr_number_
    };

    enum class Instruction_type {
      type_computational,
      type_branch       ,
      type_const        ,
      type_periphery    ,
      type_jump         ,
    };

    struct Instruction_info {
      unsigned int         rs1        {};
      unsigned int         rs2        {};
      Imm                  imm        {};
      unsigned int         rd         {};
      Concrete_instruction instruction{};
      Instruction_type     type       {};
    };

    Instruction_info decode(const std::string &line) const;

    private:
      static const std::map<std::string, Concrete_instruction> str_instr_mapping;
      Concrete_instruction decode_concrete_instruction(std::istream &line) const;
};
