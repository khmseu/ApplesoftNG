#ifndef AS_INTERPRETER_HPP
#define AS_INTERPRETER_HPP

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

#endif // AS_INTERPRETER_HPP