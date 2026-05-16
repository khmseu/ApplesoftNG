#include "core/interpreter.hpp"
#include "core/memory.hpp"
#include "platform/console.hpp"

#include <string>

Interpreter::Interpreter() : memory(0) {}

Interpreter::~Interpreter() {}

void Interpreter::execute() {
  print("READY.");
  execute(readLine().c_str());
}

void Interpreter::execute(const char *code) {
  if (code == 0) {
    return;
  }

  std::string line(code);
  const std::string prefix = "PRINT ";
  if (line.compare(0, prefix.size(), prefix) == 0) {
    print(line.substr(prefix.size()));
  }
}

void Interpreter::setMemory(Memory *memory) { this->memory = memory; }

Memory *Interpreter::getMemory() const { return memory; }