#ifndef __CFG_H__
#define __CFG_H__

#include <cassert>
#include <map>
#include <vector>
#include <list>
#include <memory>

#include "Quad.h"

namespace script
{
    class QuadContext;

    // һ��������
    class BasicBlock
    {
    public:
        BasicBlock(int id) : ID_(id), head_(nullptr), end_(nullptr) {}
        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        void set(Quad *head, Quad *end) { head_ = head; end_ = end; }

        void unique();
    private:
        unsigned ID_;
        Quad *head_, *end_;
        std::list<Quad*> phis_; 
        std::list<BasicBlock*> precursors_;   // ��¼�û���������ǰ��
        std::list<BasicBlock*> successors_;  // ��̻�����
    };

    class CFG
    {
    public:
        CFG();
        ~CFG();

        static std::unique_ptr<CFG> buildCFG(QuadContext *context);
        static void buildTarget(QuadContext *context, CFG *cfg);
        static void removeDeadBlock(CFG *cfg);

        BasicBlock *createBlock();

    private:
        unsigned numBlockIDs_;
        BasicBlock *start_; // ��ʼ������
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;
        std::map<Quad*, Quad*> labelTarget_;
    };
}

#endif // !__CFG_H__

