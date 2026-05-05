#include <gtest/gtest.h>
#include "core/interpreter.hpp"
#include "core/memory.hpp"

class CoreTests : public ::testing::Test {
protected:
    Interpreter* interpreter;
    Memory* memory;

    void SetUp() override {
        memory = new Memory();
        interpreter = new Interpreter(memory);
    }

    void TearDown() override {
        delete interpreter;
        delete memory;
    }
};

TEST_F(CoreTests, TestMemoryAllocation) {
    EXPECT_NO_THROW(memory->allocate(1024));
    EXPECT_EQ(memory->getSize(), 1024);
}

TEST_F(CoreTests, TestInterpreterExecution) {
    // Assuming we have a method to load code into the interpreter
    interpreter->loadCode("PRINT 'Hello, World!'");
    EXPECT_NO_THROW(interpreter->execute());
}

TEST_F(CoreTests, TestInvalidMemoryAccess) {
    EXPECT_THROW(memory->access(-1), std::out_of_range);
    EXPECT_THROW(memory->access(1024), std::out_of_range);
}