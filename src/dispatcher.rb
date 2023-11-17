require 'fileutils'
require 'optparse'
require 'ostruct'
require 'yaml'
require 'json'

def parse_argv
  options = OpenStruct.new
  OptionParser.new do |opts|
    opts.banner = "Usage: dispatcher.rb --isa=FILENAME --gen=GENERATED_DIR"

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
  return json_data.instructions
end

class DispatcherGenerator
    def initialize(gen_dir)
      @gen_dir = gen_dir
      unless File.directory?(@gen_dir)
        FileUtils.mkdir_p(@gen_dir)
      end
    end

    def generate_dispatch_table_case(instructions)
      dispatch_table_case = String.new
      instr_cnt = 0
      new_line_offset = 38
      for instruction in instructions
        instruction.mnemonic.gsub! '.', ""
        dispatch_table_case << "&&#{instruction.mnemonic.upcase}"
        dispatch_table_case << ","
        dispatch_table_case << " "*(11 - instruction.mnemonic.length)
        instr_cnt += 1
        if instr_cnt == 6
          dispatch_table_case << "\n"
          dispatch_table_case << " "*new_line_offset
          instr_cnt = 0
        end
      end
      dispatch_table_case << "&&BASIC_BLOCK_END"
      return dispatch_table_case
    end

    def generate_dispatch_table(instructions)
      dispatch_table = <<-EOT
    static void *dispatch_table[] = { #{generate_dispatch_table_case(instructions)} };
EOT
      return dispatch_table.chop!;
    end

    def generate_dispatch_case(instructions)
      dispatch_case = String.new
      for instruction in instructions
        instr_name = instruction.mnemonic.upcase;
        dispatch_case += <<-EOT
#{instr_name}:
    Executor#{instr_name}(hart_, *instr_iter);
    DISPATCH();
EOT
      end

      dispatch_case += <<-EOT
BASIC_BLOCK_END:
    return;

    UNREACHABLE();
EOT
      return dispatch_case.chop!
    end

    def generate_dispatcher(instructions)
      dispatcher_file = File.new(@gen_dir + '/Dispatcher.cpp', 'w') 
      dispatcher = <<-EOT
#include "Dispatcher.h"
#include "Executor-inl.h"

namespace RISCV {

void Dispatcher::dispatchExecute(BasicBlock::BodyEntry instr_iter) {
#{generate_dispatch_table(instructions)}

#define DISPATCH()                          \\
    ++instr_iter;                           \\
    goto *dispatch_table[instr_iter->type]

    goto *dispatch_table[instr_iter->type];

#{generate_dispatch_case(instructions)}
}

}  // namespace RISCV

EOT
    dispatcher_file.write(dispatcher)
    dispatcher_file.close
    end
end 

def main
  options = parse_argv()
  instructions = parse_isa_file(options.isa)
  generator = DispatcherGenerator.new(options.gen_dir)
  generator.generate_dispatcher(instructions)
end

main()
