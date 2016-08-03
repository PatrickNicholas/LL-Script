#pragma once

#include <vector>
#include <list>
#include <memory>
#include <string>

namespace script
{
namespace ir
{
    class Instruction;
}
    // һ��������
    class BasicBlock
    {
        friend class ir::Instruction;
    public:
        BasicBlock(int id, std::string name) 
            : ID_(id), head_(nullptr), end_(nullptr), name_(name)
        {}

        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }

        ir::Instruction *begin() { return head_; }
        ir::Instruction *end() { return end_; }
        unsigned getID() const { return ID_; }

        void push(ir::Instruction *instr);
        void unique();

    protected:
        unsigned ID_;
        std::string name_;
        ir::Instruction *head_, *end_;
        std::list<BasicBlock*> precursors_;   // ��¼�û���������ǰ��
        std::list<BasicBlock*> successors_;  // ��̻�����
    };

    class CFG
    {
    public:
        CFG();
        ~CFG();

        BasicBlock *createBasicBlock(std::string name);
        void setEntry(BasicBlock *entry);
        void setEnd(BasicBlock *end);
    protected:
        unsigned numBlockIDs_;
        BasicBlock *start_; // ��ʼ������
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;
    };
}


