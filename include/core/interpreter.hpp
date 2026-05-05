#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

class Memory;

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    void execute();
    void execute(const char* code);
    void setMemory(Memory* memory);
    Memory* getMemory() const;

private:
    Memory* memory;
};

#endif // INTERPRETER_HPP