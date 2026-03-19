#include "executor/step_executor.hpp"
#include "ast/ast.hpp"
#include "common/utils.hpp"
#include "io/io_provider.hpp"
#include <memory>
namespace grs_executor {


//Constructor - Destructor
    StepExecutor::StepExecutor(std::shared_ptr<grs_io::IOProvider> ioProvider) :
ioProvider_{std::move(ioProvider)},status_{ExecutionStatus::IDLE}, currentLine_{0}, pc_{0}, waitingForAck_{false}
{    
   if(ioProvider_==nullptr){
    ioProvider_ = std::make_shared<grs_io::LocalIOProvider>();
   }         

}

StepExecutor::~StepExecutor() = default;


    //Program Loading
    void StepExecutor::load(const std::shared_ptr<grs_ast::FunctionBlock>& program){
        program_ = program;
        statements_.clear();
        pc_=0;
        variables_.clear();
        variableTypes_.clear();
        positionStore_.clear();
        errorMessage_.clear();
        waitingForAck_ = false;

    //Coonvert the top level statements of the AST to single list
    flattenStatements(program);

    // Set current line to first statement's line (so IDE knows where execution starts)
    if (!statements_.empty() && !statements_[0]->getLineColumn().empty()) {
        currentLine_ = statements_[0]->getLineColumn().front().first;
    }

    setStatus(ExecutionStatus::IDLE);

}


void StepExecutor::flattenStatements(const std::shared_ptr<grs_ast::FunctionBlock>& block){
// Flatten nested FunctionBlocks (e.g. DEF body) so each statement is steppable.
// Complex control flow (IF, FOR) remains as single nodes — they evaluate internally.
   
    for(const auto& stmt : block->getStatements())
    {
        // If the statement is itself a FunctionBlock (Program type), flatten its contents recursively
        if(stmt->getType() == grs_ast::ASTNodeType::Program){
            auto innerBlock = std::static_pointer_cast<grs_ast::FunctionBlock>(stmt);
            flattenStatements(innerBlock);
        } else {
            statements_.push_back(stmt);
        }
    }
}

bool StepExecutor::step(){
    
    if(status_ == ExecutionStatus::COMPLETED || status_ == ExecutionStatus::ERROR){
        return false;
    }

    if(waitingForAck_){
    //we are waiting ack from robot
        return true;
    }
    if(pc_>= statements_.size()){
        setStatus(ExecutionStatus::COMPLETED);
        return false;
    }
    setStatus(ExecutionStatus::RUNNING);
    
    auto& stmt = statements_[pc_];
    
    //reload line number
    if(!stmt->getLineColumn().empty()){
        currentLine_ = stmt->getLineColumn().front().first;
    }
    try{
        stmt->accept(*this);
    }catch(const std::exception& e){
        errorMessage_ =e.what();
        setStatus(ExecutionStatus::ERROR);
        return false;
    }
    
    pc_++;

    //if you are waiting ack, the status remained WAITING_ACK
    if(waitingForAck_){
        setStatus(ExecutionStatus::WAITING_ACK);
        return true;
    }

    if(pc_ >= statements_.size()){
        setStatus(ExecutionStatus::COMPLETED);
        return false;
    }

    // Update currentLine_ to the NEXT statement (what we're about to execute).
    // This is standard debugger behavior: "stopped at line X" means X hasn't run yet.
    if(!statements_[pc_]->getLineColumn().empty()){
        currentLine_ = statements_[pc_]->getLineColumn().front().first;
    }

    setStatus(ExecutionStatus::PAUSED);
    return true;
}

void StepExecutor::run(){
    
    while(step()){
        if(status_ == ExecutionStatus::WAITING_ACK){
            break; //wait ack
        }
        
        //is the other statement breakpoint?
        if(pc_<statements_.size()){
            auto& nextStmt = statements_[pc_];
            if(!nextStmt->getLineColumn().empty()){
                int nextLine = nextStmt->getLineColumn().front().first;
                if(isBreakPoint(nextLine)){
                    setStatus(ExecutionStatus::PAUSED);
                    break;
                }
            }
        }
    }
}

void StepExecutor::pause(){
    if(status_ == ExecutionStatus::RUNNING){
        setStatus(ExecutionStatus::PAUSED);
    }
}
void StepExecutor::stop(){
    setStatus(ExecutionStatus::COMPLETED);
}

void StepExecutor::reset(){
    pc_ = 0;
    variables_.clear();
    variableTypes_.clear();
    positionStore_.clear();
    errorMessage_.clear();
    waitingForAck_= false;
    currentLine_ = 0;
    setStatus(ExecutionStatus::IDLE);
}

void StepExecutor::acknowledgeCommand(){
    waitingForAck_= false;
    if(status_ ==  ExecutionStatus::WAITING_ACK){
        setStatus(ExecutionStatus::PAUSED);
    }
}


//Breakpoints

void StepExecutor::addBreakpoint(int line){
    breakpoints_.insert(line);
}

void StepExecutor::removeBreaPoint(int line){
    breakpoints_.erase(line);
}

void StepExecutor::clearBreakPoint(){
    breakpoints_.clear();
}

bool StepExecutor::isBreakPoint(int line)const{
    return breakpoints_.count(line) > 0;
}

//Status

void StepExecutor::setStatus(ExecutionStatus status){
    status_ = status;
    if(statusCallback_){
        statusCallback_(status,currentLine_);
    }

}


//Expression Evaluation

common::ValueType StepExecutor::evaluateExpression(const std::shared_ptr<grs_ast::Expression>& expr){
    expr->accept(*this);
    return lastValue_;
}

//Visitor: FunctionBlock
//we use this for if/else branch 
void StepExecutor::visit(grs_ast::FunctionBlock& node){


for(const auto& stmt : node.getStatements()){
    if(!stmt->getLineColumn().empty()){
        currentLine_ = stmt->getLineColumn().front().first;
    }
    stmt->accept(*this);

    if(status_ == ExecutionStatus::ERROR || status_ == ExecutionStatus::COMPLETED){
        return;
    }
 }

}


// Visitor : VariableDeclaration
void StepExecutor::visit(grs_ast::VariableDeclaration& node) {
    variableTypes_[node.getName()] = node.getDataType();

    if (node.getInitializer()) {
        auto value = evaluateExpression(node.getInitializer());
        variables_[node.getName()] = value;
    } else {
        // Default değerler
        switch (node.getDataType()) {
            case grs_lexer::TokenType::INT:   variables_[node.getName()] = 0; break;
            case grs_lexer::TokenType::REAL:  variables_[node.getName()] = 0.0; break;
            case grs_lexer::TokenType::BOOL:  variables_[node.getName()] = false; break;
            case grs_lexer::TokenType::CHAR:  variables_[node.getName()] = std::string(""); break;
            default: variables_[node.getName()] = 0; break;
        }
    }
}



void StepExecutor::visit(grs_ast::MotionCommand& node) {
    RobotCommand cmd;
    cmd.type = stringToCommandType(node.getCommand());
    cmd.targetName = node.getName();
    cmd.sourceLine = currentLine_;

    // Position parametrelerini çöz
    if (positionStore_.count(node.getName())) {
        for (const auto& [key, val] : positionStore_[node.getName()]) {
            cmd.params.emplace_back(key, val);
        }
    }

    if (commandCallback_) {
        waitingForAck_ = true;
        commandCallback_(cmd);
    }
}

// ─── Visitor: OutputStatement ───

void StepExecutor::visit(grs_ast::OutputStatement& node) {
    auto value = evaluateExpression(node.getValue());

    bool boolValue = false;
    if (auto* b = std::get_if<bool>(&value)) {
        boolValue = *b;
    } else if (auto* i = std::get_if<int>(&value)) {
        boolValue = (*i != 0);
    }

    // I/O Provider'a yaz (KRL $OUT is 1-based, hardware is 0-based)
    uint8_t hwIndex = (node.getIndex() > 0) ? node.getIndex() - 1 : 0;
    if (ioProvider_) {
        ioProvider_->writeDigitalOutput(hwIndex, boolValue);
    }

    // Robot'a command olarak gönder
    RobotCommand cmd;
    cmd.type = RobotCommand::Type::OUTPUT;
    cmd.ioIndex = node.getIndex(); // KRL index (1-based) for display
    cmd.ioValue = boolValue;
    cmd.sourceLine = currentLine_;

    if (commandCallback_) {
        waitingForAck_ = true;
        commandCallback_(cmd);
    }
}

// ─── Visitor: InputExpression ───

void StepExecutor::visit(grs_ast::InputExpression& node) {
    if (ioProvider_) {
        // KRL $IN is 1-based, hardware is 0-based
        uint8_t hwIndex = (node.getIndex() > 0) ? node.getIndex() - 1 : 0;
        lastValue_ = ioProvider_->readDigitalInput(hwIndex);
    } else {
        lastValue_ = false;
    }
}

// ─── Visitor: IfStatement ───

void StepExecutor::visit(grs_ast::IfStatement& node) {
    // RUNTIME'da condition evaluate et
    auto condValue = evaluateExpression(node.getCondition());

    bool condResult = false;
    if (auto* b = std::get_if<bool>(&condValue)) {
        condResult = *b;
    } else if (auto* i = std::get_if<int>(&condValue)) {
        condResult = (*i != 0);
    } else if (auto* d = std::get_if<double>(&condValue)) {
        condResult = (*d != 0.0);
    }

    if (condResult && node.getThenBranch()) {
        // Then branch'i çalıştır — block'taki her statement step edilebilir
        node.getThenBranch()->accept(*this);
    } else if (!condResult && node.getElseBranch()) {
        node.getElseBranch()->accept(*this);
    }
}

// ─── Visitor: WaitStatement ───

void StepExecutor::visit(grs_ast::WaitStatement& node) {
    RobotCommand cmd;
    cmd.type = RobotCommand::Type::WAIT;
    cmd.waitTime = node.waitTime_;
    cmd.sourceLine = currentLine_;

    if (commandCallback_) {
        waitingForAck_ = true;
        commandCallback_(cmd);
    }
}

// ─── Visitor: BinaryExpression ───

void StepExecutor::visit(grs_ast::BinaryExpression& node) {
    // Assignment
    if (node.getOperator() == grs_lexer::TokenType::ASSIGN) {
        auto value = evaluateExpression(node.getRight());
        
        if (node.getLeft()->getType() == grs_ast::ASTNodeType::VariableExpression) {
            auto varExpr = std::static_pointer_cast<grs_ast::VariableExpression>(node.getLeft());
            variables_[varExpr->getName()] = value;
        }
        lastValue_ = value;
        return;
    }

    auto leftVal = evaluateExpression(node.getLeft());
    auto rightVal = evaluateExpression(node.getRight());

    // Aritmetic and comparison operator
    auto getDouble = [](const common::ValueType& v) -> double {
        if (auto* i = std::get_if<int>(&v)) return *i;
        if (auto* d = std::get_if<double>(&v)) return *d;
        if (auto* b = std::get_if<bool>(&v)) return *b ? 1.0 : 0.0;
        return 0.0;
    };

    double l = getDouble(leftVal);
    double r = getDouble(rightVal);

    switch (node.getOperator()) {
        case grs_lexer::TokenType::PLUS:      lastValue_ = l + r; break;
        case grs_lexer::TokenType::MINUS:     lastValue_ = l - r; break;
        case grs_lexer::TokenType::MULTIPLY:  lastValue_ = l * r; break;
        case grs_lexer::TokenType::DIVIDE:
            if (r == 0.0) { errorMessage_ = "Division by zero"; setStatus(ExecutionStatus::ERROR); return; }
            lastValue_ = l / r;
            break;
        case grs_lexer::TokenType::LESS:      lastValue_ = l < r; break;
        case grs_lexer::TokenType::GREATER:   lastValue_ = l > r; break;
        case grs_lexer::TokenType::LESSEQ:    lastValue_ = l <= r; break;
        case grs_lexer::TokenType::GREATEREQ: lastValue_ = l >= r; break;
        case grs_lexer::TokenType::EQUAL:     lastValue_ = l == r; break;
        case grs_lexer::TokenType::NOTEQUAL:  lastValue_ = l != r; break;
        case grs_lexer::TokenType::AND: {
            bool lb = false, rb = false;
            if (auto* b = std::get_if<bool>(&leftVal)) lb = *b; else lb = (l != 0.0);
            if (auto* b = std::get_if<bool>(&rightVal)) rb = *b; else rb = (r != 0.0);
            lastValue_ = lb && rb;
            break;
        }
        case grs_lexer::TokenType::OR: {
            bool lb = false, rb = false;
            if (auto* b = std::get_if<bool>(&leftVal)) lb = *b; else lb = (l != 0.0);
            if (auto* b = std::get_if<bool>(&rightVal)) rb = *b; else rb = (r != 0.0);
            lastValue_ = lb || rb;
            break;
        }
        default:
            errorMessage_ = "Unknown binary operator";
            setStatus(ExecutionStatus::ERROR);
            break;
    }
}

// ─── Visitor: UnaryExpression ───

void StepExecutor::visit(grs_ast::UnaryExpression& node) {
    auto val = evaluateExpression(node.getExpression());

    switch (node.getOperator()) {
        case grs_lexer::TokenType::MINUS: {
            if (auto* i = std::get_if<int>(&val)) lastValue_ = -(*i);
            else if (auto* d = std::get_if<double>(&val)) lastValue_ = -(*d);
            break;
        }
        case grs_lexer::TokenType::NOT: {
            if (auto* b = std::get_if<bool>(&val)) lastValue_ = !(*b);
            else if (auto* i = std::get_if<int>(&val)) lastValue_ = (*i == 0);
            break;
        }
        default:
            errorMessage_ = "Unknown unary operator";
            setStatus(ExecutionStatus::ERROR);
            break;
    }
}

// ─── Visitor: LiteraExpression ───

void StepExecutor::visit(grs_ast::LiteraExpression& node) {
    lastValue_ = node.getValue();
}

// ─── Visitor: VariableExpression ───

void StepExecutor::visit(grs_ast::VariableExpression& node) {
    if (variables_.count(node.getName())) {
        lastValue_ = variables_[node.getName()];
    } else if (positionStore_.count(node.getName())) {
        // Position/Axis/Frame variable — parser P2->x := 2 gibi ifadeleri
        // iki parçaya ayırdığı için bare "P2" buraya gelir, dummy değer döndür
        lastValue_ = 0;
    } else {
        errorMessage_ = "Undefined variable: " + node.getName();
        setStatus(ExecutionStatus::ERROR);
    }
}

// ─── Visitor: Position/Frame/Axis Declarations ───

void StepExecutor::visit(grs_ast::FrameDeclaration& node) {
    auto& store = positionStore_[node.getName()];
    for (const auto& [name, expr] : node.getArgs()) {
        store[name] = evaluateExpression(expr);
    }
}

void StepExecutor::visit(grs_ast::PositionDeclaration& node) {
    auto& store = positionStore_[node.getName()];
    for (const auto& [name, expr] : node.getArgs()) {
        store[name] = evaluateExpression(expr);
    }
}

void StepExecutor::visit(grs_ast::AxisDeclaration& node) {
    auto& store = positionStore_[node.getName()];
    for (const auto& [name, expr] : node.getArgs()) {
        store[name] = evaluateExpression(expr);
    }
}

void StepExecutor::visit(grs_ast::ExecutePosAndAxisExpression& node) {
    auto val = evaluateExpression(node.getExpr());
    positionStore_[node.getName()][node.getArg()] = val;
}

// ─── Visitor: FunctionDeclaration (placeholder) ───

void StepExecutor::visit(grs_ast::FunctionDeclaration& node) {
    // TODO: function table'a ekle
}

// ─── Helpers ───

RobotCommand::Type StepExecutor::stringToCommandType(const std::string& cmd) {
    if (cmd == "PTP") return RobotCommand::Type::PTP;
    if (cmd == "PTP_REL") return RobotCommand::Type::PTP_REL;
    if (cmd == "LIN") return RobotCommand::Type::LIN;
    if (cmd == "LIN_REL") return RobotCommand::Type::LIN_REL;
    if (cmd == "CIRC") return RobotCommand::Type::CIRC;
    if (cmd == "CIRC_REL") return RobotCommand::Type::CIRC_REL;
    if (cmd == "SPLINE") return RobotCommand::Type::SPLINE;
    if (cmd == "SPLINE_REL") return RobotCommand::Type::SPLINE_REL;
    return RobotCommand::Type::UNKNOWN;
}



}
