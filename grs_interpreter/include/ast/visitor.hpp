#ifndef VISITOR_HPP_
#define VISITOR_HPP_


namespace grs_ast{
    
class Program;
class FunctionBlock;
class MotionCommand;
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
class FunctionDeclaration;

//Visitor interface
    class ASTVisitor
    {
        public:
        virtual ~ASTVisitor( ) = default;
        virtual void visit(FunctionBlock& node) = 0;
        virtual void visit(MotionCommand& node) = 0;
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
        virtual void visit(FunctionDeclaration& node) = 0;

    };

    class ASTVisitorBase : public ASTVisitor{
        public: 
        void visit(FunctionBlock& node) {}
        void visit(MotionCommand& node) override {}
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
        void visit(FunctionDeclaration& node) override {}

    };


}



#endif //VISITOR_HPP_
