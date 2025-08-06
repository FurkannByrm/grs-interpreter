#ifndef VISITOR_HPP_
#define VISITOR_HPP_


namespace krl_ast{
    
class Program;
class Command;
class Expression;
class BinaryExpression;
class UnaryExpression;
class LiteraExpression;
class VariableExpression;
class VariableDeclaration;
class FrameDeclaration;
class PositionDeclaration;
class AxisDeclaration;
class IfStatement;
class WaitStatement;

//Visitor interface
    class ASTVisitor
    {
        public:
        virtual ~ASTVisitor( ) = default;
        virtual void visit(Program& node) = 0;
        virtual void visit(Command& node) = 0;
        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(LiteraExpression& node) = 0;
        virtual void visit(VariableExpression& node) = 0;
        virtual void visit(VariableDeclaration& node) = 0;
        virtual void visit(FrameDeclaration& node) = 0;
        virtual void visit(PositionDeclaration& node) = 0;
        virtual void visit(AxisDeclaration& node) = 0;
        virtual void visit(IfStatement& node) = 0;
        virtual void visit(WaitStatement& node) = 0;
        // virtual void visit(MotionCommand& node) = 0;

    };

    class ASTVisitorBase : public ASTVisitor{
        public: 
        void visit(Program& node) override {}
        void visit(Command& node) override {}
        void visit(BinaryExpression& node) override {}
        void visit(UnaryExpression& node) override {}
        void visit(LiteraExpression& node) override {}
        void visit(VariableExpression& node) override {}
        void visit(VariableDeclaration& node) override {}
        void visit(FrameDeclaration& node) override{}
        void visit(PositionDeclaration& node) override{}
        void visit(AxisDeclaration& node) override{}
        void visit(IfStatement& node) override {}
        void visit(WaitStatement& node) override {}
        // void visit(MotionCommand& node) override {}


    };


}



#endif //VISITOR_HPP_
