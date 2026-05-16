#include "core/memory.hpp"

Memory::Memory(size_t size) {
  memory = new uint8_t[size];
  memorySize = size;
}

Memory::~Memory() { delete[] memory; }

uint8_t Memory::read(size_t address) const {
  if (address < memorySize) {
    return memory[address];
  }
  // Handle out-of-bounds access (could throw an exception or return a default
  // value)
  return 0;
}

void Memory::write(size_t address, uint8_t value) {
  if (address < memorySize) {
    memory[address] = value;
  }
  // Handle out-of-bounds access (could throw an exception or ignore)
}

size_t Memory::getSize() const { return memorySize; }