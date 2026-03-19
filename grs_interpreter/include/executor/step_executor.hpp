#ifndef STEP_EXECUTOR_HPP_
#define STEP_EXECUTOR_HPP_

#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "common/utils.hpp"
#include "io/io_provider.hpp"
#include "lexer/token.hpp"

#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace grs_executor{

    enum class ExecutionStatus{
        
        IDLE,  //program loaded, did't run yet
        RUNNING, //running mod
        PAUSED, //stopped after breakpoint or step 
        WAITING_ACK,  //waiting ack from robot
        COMPLETED,
        ERROR
    
    };

    struct RobotCommand{
        
        enum class Type{ PTP, PTP_REL, LIN, LIN_REL, CIRC, CIRC_REL,
                        SPLINE, SPLINE_REL, WAIT, OUTPUT, UNKNOWN};

        Type type = Type::UNKNOWN;
        std::string targetName;
        std::vector<std::pair<std::string,common::ValueType>> params;
        int sourceLine = 0;

        //OUTPUT specifics
        uint8_t ioIndex = 0;
        bool ioValue = false;


        //Wait spesifics
        double waitTime = 0.0;
    };

    class StepExecutor : public grs_ast::ASTVisitor{

        public:
             explicit StepExecutor(std::shared_ptr<grs_io::IOProvider> ioProvider = nullptr);
             ~StepExecutor();

             //Program loading
             void load(const std::shared_ptr<grs_ast::FunctionBlock>& program);

             //Execution Contol
             bool step();//one statement runs
             void run(); //run until the breakpoint/endpoint
             void pause();
             void stop();
             void reset();

             void acknowledgeCommand();

             //BreakPoints
             void addBreakpoint(int line);
             void removeBreaPoint(int line);
             void clearBreakPoint();
             bool isBreakPoint(int line)const;

             //State Query
             ExecutionStatus getStatus() const {return status_; }
             int getCurrentLine() const {return currentLine_;}
             std::string getErrorMessage()const{ return errorMessage_; }
    

             //Variable inspection for debug
             const std::unordered_map<std::string,common::ValueType>& getVariables() const {return variables_;}
             const std::unordered_map<std::string,grs_lexer::TokenType>& getVariableTypes() const {return variableTypes_;}

             //Visitor Methods

             void visit(grs_ast::FunctionBlock& node)override;
             void visit(grs_ast::MotionCommand& node)override;
             void visit(grs_ast::BinaryExpression& node)override;
             void visit(grs_ast::UnaryExpression& node)override;
             void visit(grs_ast::LiteraExpression& node)override;
             void visit(grs_ast::VariableExpression& node)override;
             void visit(grs_ast::VariableDeclaration& node)override;
             void visit(grs_ast::FrameDeclaration& node)override;
             void visit(grs_ast::PositionDeclaration& node)override;
             void visit(grs_ast::AxisDeclaration& node)override;
             void visit(grs_ast::ExecutePosAndAxisExpression& node)override;
             void visit(grs_ast::IfStatement& node)override;
             void visit(grs_ast::WaitStatement& node)override;
             void visit(grs_ast::FunctionDeclaration& node)override;
             void visit(grs_ast::InputExpression& node)override;
             void visit(grs_ast::OutputStatement& node)override;

             //CallBacks
             using CommandCallback = std::function<void(const RobotCommand&)>;
             inline void setCommandCallback(CommandCallback cb){commandCallback_ = std::move(cb);} 
             //When the situation chanced for notification to IDE
             using StatusCallback = std::function<void(ExecutionStatus, int line)>;
             inline void setStatusCallback(StatusCallback cb){statusCallback_ = std::move(cb);}


        private:
             ExecutionStatus status_;//must be initialized
             int currentLine_;//must be initialized
             std::string errorMessage_;

             //program is saved as statement list 
             std::shared_ptr<grs_ast::FunctionBlock> program_;
            
             //statement list and program counter 
             std::vector<std::shared_ptr<grs_ast::ASTNode>> statements_;
             size_t pc_; // must be initialized

             //Variable storage
             std::unordered_map<std::string, common::ValueType> variables_;
             std::unordered_map<std::string, grs_lexer::TokenType> variableTypes_;

             //Position/Axis/Frame storage
             std::unordered_map<std::string,std::unordered_map<std::string,common::ValueType>> positionStore_;
             
            //I/O
             std::shared_ptr<grs_io::IOProvider> ioProvider_;

             //Breakpoints
             std::unordered_set<int> breakpoints_;

             //Expression evaluation -- visitor result is written here
             common ::ValueType lastValue_;
             common::ValueType evaluateExpression(const std::shared_ptr<grs_ast::Expression>& expr);

             //CallBacks
             CommandCallback commandCallback_;
             StatusCallback statusCallback_;
             bool waitingForAck_;//must be initialized
             

             //Helpers
             void setStatus(ExecutionStatus status);
             void flattenStatements(const std::shared_ptr<grs_ast::FunctionBlock>& block);

             //it can run a block for if/for control flow and return the addition statements
             void executeBlock(const std::shared_ptr<grs_ast::ASTNode>& block);

             static RobotCommand::Type stringToCommandType(const std::string& cmd);
    };





}

#endif //STEP_EXECUTOR_HPP_
