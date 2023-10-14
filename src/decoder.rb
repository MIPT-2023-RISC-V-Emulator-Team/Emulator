require 'fileutils'
require 'optparse'
require 'ostruct'
require 'yaml'
require 'json'

def parse_argv
  options = OpenStruct.new
  OptionParser.new do |opts|
    opts.banner = "Usage: decoder.rb --isa=FILENAME --gen=GENERATED_DIR"

    opts.on("-iFILENAME", "--isa=FILENAME", "Set file with ISA") do |f|
      options.isa = f
    end

    opts.on("-gGENERATED_DIR", "--gen=GENERATED_DIR", "Set directory where decoder will be generated") do |dir|
      options.gen_dir = dir
    end

    opts.on("-h", "--help", "Prints this help") do
      puts opts
      exit
    end
  end.parse!(into: options)
  return options
end

def parse_isa_file(isa_filename)
  yaml_file = File.read(isa_filename)
  yaml_data = YAML.safe_load(yaml_file, aliases: true)
  json_data = JSON.parse(yaml_data.to_json, object_class: OpenStruct)
  return json_data.fields, json_data.instructions, json_data.decodertree
end

class DecoderGenerator
  def initialize(gen_dirr)
    @gen_dir = gen_dirr
    unless File.directory?(@gen_dir)
      FileUtils.mkdir_p(@gen_dir)
    end
  end

  def generate_signed_bit(field)
    signed_bit = String.new
    if field.name.include? "imm"
      signed_bit = " "*4 + "static constexpr uint8_t SIGNEDBIT = " \
                   + field.location.bits.max_by(&:from).from.to_s + ";"
    end
    return signed_bit
  end

  def generate_field_value(field)
    value = String.new
    shift = String.new
    for bit in field.location.bits
      shift_bits = bit.lsb - bit.to
      partitial_bits = "getPartialBits<#{bit.lsb.to_s}, #{bit.msb.to_s}>(instr)"
      if shift_bits > 0
        value << " "*8 + "shiftRight<#{shift_bits.to_s()}>(#{partitial_bits})"
      else
        value << " "*8 + "shiftLeft<#{(-shift_bits).to_s()}>(#{partitial_bits})"
      end

      if bit != field.location.bits[-1]
        value << " +\n"
      else
        value << ";"
      end
    end
    return value
  end

  def generate_field_class(decoded_name, field)
    reg_types = Array.new(["rd", "rs1", "rs2", "rs3", "rm"])
    return_type = 'uint32_t'
    if reg_types.include? field.name
        return_type = 'RegisterType'
    end

    field_class = <<-EOT
struct #{decoded_name.capitalize} {
    static constexpr uint32_t MASK = 0x#{field.location.debug_hex_mask};
#{generate_signed_bit(field)}
    static inline #{return_type} getValue(uint32_t instr)
    {
        uint32_t #{decoded_name} =
#{generate_field_value(field)}
        return #{if return_type != 'RegisterType'
                  decoded_name
                 else
                   "static_cast<RegisterType>(#{decoded_name})"
                 end};
    }
};

EOT
    return field_class
  end

  def generate_field(fields)
    klass = String.new
    for key in fields.to_h.keys
      klass << generate_field_class(key.name, fields[key])
    end
    return klass.chop!
  end

  def generate_fields(fields)
    fields_file = File.new(@gen_dir + '/Fields.h', 'w') 

    pre_header = <<-EOT
#ifndef GENERATED_FIELDS_H
#define GENERATED_FIELDS_H

#include "Common.h"

namespace RISCV {
#{generate_field(fields)}
}  // namespace RISCV

#endif  // GENERATED_FIELDS_H
EOT

    fields_file.write(pre_header)

    fields_file.close
  end

  def generate_instruction_type(instructions)
    instr_type = String.new
    for instruction in instructions
      instruction.mnemonic.gsub! '.', ""
      instr_type << " "*4 + instruction.mnemonic.upcase + ",\n"
    end
    return instr_type
  end

  def generate_instruction_fields(fields)
    instr_field = String.new
    for field in fields
      field_name = field
      if field_name.include? 'imm'
        field_name = 'imm'
        instr_field << " "*8 + "decInstr.immSignBitNum = #{field.capitalize}::SIGNEDBIT;\n"
      end
      instr_field << " "*8 + "decInstr.#{field_name} = " \
                     "#{field.capitalize}::getValue(encInstr);\n"
    end
    instr_field << " "*8 + "return decInstr;"
    return instr_field
  end

  def generate_instruction_class(instruction)
    instr_class = <<-EOT
struct Instruction#{instruction.mnemonic.upcase} {
    static constexpr uint32_t OPCODE = #{instruction.fixedvalue};
    static inline DecodedInstruction decodeInstruction(EncodedInstruction encInstr) {
        DecodedInstruction decInstr;
        decInstr.type = InstructionType::#{instruction.mnemonic.upcase};
        decInstr.exec = Executor#{instruction.mnemonic.upcase};
#{generate_instruction_fields(instruction.fields)}
    }
};

EOT
    return instr_class
  end

  def generate_instruction(instructions)
    instr = String.new
    for instruction in instructions
      instr << generate_instruction_class(instruction)
    end
    return instr.chop!
  end

  def generate_instruction_classes(instructions)
    instructions_file = File.new(@gen_dir + '/Instructions.h', 'w') 
    instr_enum = <<-EOT
#ifndef GENERATED_INSTRUCTIONS_H
#define GENERATED_INSTRUCTIONS_H

#include <cstdint>
#include "Common.h"
#include "generated/Fields.h"

namespace RISCV {

#{generate_instruction(instructions)}
}  // namespace RISCV

#endif  // GENERATED_INSTRUCTIONS_H
EOT

    instructions_file.write(instr_enum)
    instructions_file.close
  end

  def generate_instruction_types(instructions)
    instructions_file = File.new(@gen_dir + '/InstructionTypes.h', 'w') 
    instr_enum = <<-EOT
#ifndef GENERATED_INSTRUCTION_TYPES_H
#define GENERATED_INSTRUCTION_TYPES_H

#include <cstdint>

namespace RISCV {

enum InstructionType : uint8_t {
#{generate_instruction_type(instructions)}
    INSTRUCTION_COUNT,

    INSTRUCTION_INVALID = INSTRUCTION_COUNT
};

}  // namespace RISCV

#endif  // GENERATED_INSTRUCTION_TYPES_H
EOT

    instructions_file.write(instr_enum)
    instructions_file.close
  end

  def generate_instructions(instructions)
    generate_instruction_types(instructions)
    generate_instruction_classes(instructions)
  end

  def get_instruction_opcode(low, high)
    mask = ((1 << (high - low + 1)) - 1) << low;
    return 0xFFFFFFFF & mask;
  end

  def generate_single_case(instr_name, mask, whitespace)
    instr_name.gsub! '.', ""
    single_case = " "*whitespace + "case getOpcodeBits<Instruction#{instr_name.upcase}::OPCODE, #{mask}>():\n" + \
                  " "*(whitespace + 4) + "return Instruction#{instr_name.upcase}::decodeInstruction(encInstr);\n"
    return single_case
  end

  def generate_decoder_switch(decodertree, whitespace)
    opcode = get_instruction_opcode(decodertree.range.lsb, decodertree.range.msb)
    decoder_switch = " "*whitespace + "switch (encInstr & 0x#{opcode.to_s(16)}) {\n"

    for key in decodertree.nodes.to_h.keys
      node = decodertree.nodes[key]
      if node.range == nil
        decoder_switch << generate_single_case(node.mnemonic, "0x#{opcode.to_s(16)}", whitespace + 4)
      else
        value = key.name.to_i()
        decoder_switch << " "*(whitespace + 4) + "case 0x#{value.to_s(16)}:\n"
        decoder_switch << generate_decoder_switch(node, whitespace + 8)
      end
    end
    decoder_switch << " "*(whitespace + 4) + "default:\n" +
                      " "*(whitespace + 8) + "UNREACHABLE();\n"
    decoder_switch << " "*whitespace + "}\n"
    return decoder_switch
  end

  def generate_decoder_body(decodertree)
    return generate_decoder_switch(decodertree, 4).chop!
  end

  def generate_decoder(decodertree)
    decoder_file = File.new(@gen_dir + '/Decoder.cpp', 'w') 
    decode_method = <<-EOT
#include "Decoder.h"
#include "macros.h"
#include "generated/Instructions.h"

namespace RISCV {

DecodedInstruction Decoder::decodeInstruction(const EncodedInstruction encInstr) const {
    DecodedInstruction decInstr;
#{generate_decoder_body(decodertree)}
    UNREACHABLE();
}

}  // namespace RISCV

EOT
    decoder_file.write(decode_method)
    decoder_file.close
  end
end

def generate_decoder(gen_dir, fields, instructions, decodertree)
  decoder_gen = DecoderGenerator.new(gen_dir)
  decoder_gen.generate_fields(fields)
  decoder_gen.generate_instructions(instructions)
  decoder_gen.generate_decoder(decodertree)
end

def main
  options = parse_argv()
  fields, instructions, decodertree = parse_isa_file(options.isa)
  generate_decoder(options.gen_dir, fields, instructions, decodertree)
end

main()
