#include "interpreter/motion_controller.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>


namespace krl_interpreter{

MotionController::MotionController() : isMoving_{false}{

    currentPosition_ = currentPosition_ = { 0, 0, 0, 0, 0, 0};
    currentjointValues_ = {0, 0, 0, 0, 0, 0};
    toolOffset_         = { 0, 0, 0, 0, 0, 0};
    workspaceMin_       = {-1000, -1000, -1000, -180, -180, -180};
    workspaceMax_       = {1000, 1000, 1000, 180, 180, 180};
}

MotionController::~MotionController(){
    stop();
}


void MotionController::moveToPosition(const Position& target, MotionType type, const MotionParameters& params){
    checkWorkspaceLimit(target);

    Position adjustedTarget = {
        target.x + toolOffset_.x,
        target.y + toolOffset_.y,
        target.z + toolOffset_.z,
        target.a + toolOffset_.a,
        target.b + toolOffset_.b,
        target.c + toolOffset_.c
    };

std::cout<< " Motion started: "
         << " From ("<<currentPosition_.x<<", "<<currentPosition_.y<<", "<<currentPosition_.z<<")"
         << " To ("<<adjustedTarget.x<<", "<<adjustedTarget.y<<","<<adjustedTarget.z<<")"<<std::endl;

isMoving_ = true;

double distance = std::sqrt(
    std::pow(adjustedTarget.x - currentPosition_.x,2) + 
    std::pow(adjustedTarget.y - currentPosition_.y,2) +
    std::pow(adjustedTarget.z - currentPosition_.z,2)
);

double time = distance / params.velocity;

std::cout<<"Moving.. (simulated for"<< time << " seconds)"<<std::endl;
std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time * 1000)));

currentPosition_ = adjustedTarget;

if(positionCallback_){
    positionCallback_(currentPosition_);
}

isMoving_ = false;
std::cout<<"Motion complated."<<std::endl;
}

void MotionController::moveJoints(const std::vector<double>& jointValues, const MotionParameters params){
 if(jointValues.size() != currentjointValues_.size()){
    std::cerr<< "Error: Joint values size mismatch "<<std::endl;
    return;
}   
std::cout<<"joint motion started"<<std::endl;
isMoving_ = true;

double maxMovement = 0;
for(size_t i=0; i < jointValues.size(); ++i){
    maxMovement = std::max(maxMovement, std::abs(jointValues[i] - currentjointValues_[i]));
}

double time = maxMovement / params.velocity;
std::cout<<"Moving joints.. (simulated for"<<time<< "seconds)"<<std::endl;
std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time * 1000)));

currentjointValues_ = jointValues;
isMoving_ = false;

std::cout<<" Joint motion complete"<<std::endl;

}

void MotionController::wait(double seconds){
    std::cout<<"waiting for"<<seconds<<" seconds"<<std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

void MotionController::stop(){
    if(isMoving_){
        std::cout<<"stopping motion..."<<std::endl;
        isMoving_ = false;
    }
}

void MotionController::emergency(){
    std::cout<<"EMERGENCY STOP"<<std::endl;
    stop();
}

Position MotionController::getCurrentPosition() const{
    return currentPosition_;
}
std::vector<double> MotionController::getCurrentJointValues()const{
    return currentjointValues_;
}
bool MotionController::isMoving()const{
    return isMoving_;
}
void MotionController::setToolOffset(const Position& offset){
    toolOffset_ = offset;
}

void MotionController::setWorkspaceLimit(const Position& min, Position& max)
{    workspaceMax_ = max;
    workspaceMin_ = min;  
}

void MotionController::setPositionCallback(std::function<void(const Position&)> callback){
positionCallback_ = callback;
}

void MotionController::checkWorkspaceLimit(const Position& position){
    if(position.x < workspaceMin_.x || position.x > workspaceMax_.x ||
       position.y < workspaceMin_.y || position.y > workspaceMax_.y ||
       position.z < workspaceMin_.z || position.z > workspaceMax_.z ||
       position.a < workspaceMin_.a || position.a > workspaceMax_.a ||
       position.b < workspaceMin_.b || position.b > workspaceMax_.b ||
       position.c < workspaceMin_.c || position.c > workspaceMax_.c)
       {
        std::cerr<< "Error: Position out of workspace limits"<<std::endl;
        throw std::runtime_error("Position out of workspace limits");
       }

}

}