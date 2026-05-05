#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <cstdint>

class Memory {
public:
    Memory(size_t size);
    ~Memory();

    uint8_t read(size_t address) const;
    void write(size_t address, uint8_t value);
    size_t getSize() const;

private:
    uint8_t* memory;
    size_t memorySize;
};

#endif // MEMORY_HPP