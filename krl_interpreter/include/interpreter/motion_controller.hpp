#ifndef MOTION_CONTROLLER_HPP_
#define MOTION_CONTROLLER_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace krl_interpreter{

struct Position{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double a = 0.0; //roll 
    double b = 0.0; //pich 
    double c = 0.0; //yaw
};
struct MotionParameters {
    double velocity = 100.0;  // mm/s
    double acceleration = 1000.0;  // mm/s^2
    double deceleration = 1000.0;  // mm/s^2
    double jerk = 5000.0;  // mm/s^3
};
enum class MotionType{

    PTP,
    LIN,
    CIRC,
    SPL

};

    class MotionController{ 

        public:
        MotionController();
        ~MotionController();

        void moveToPosition(const Position& target, MotionType type, const MotionParameters& params);
        void moveJoints(const std::vector<double>& jointValues, const MotionParameters params);
        void wait(double seconds);
        void stop();
        void emergency();

        Position getCurrentPosition() const;
        std::vector<double> getCurrentJointValues() const;
        bool isMoving() const;

        void setToolOffset(const Position& offset);
        void setWorkspaceLimit(const Position& min, Position& max);
        void setPositionCallback(std::function<void(const Position&)> callback);
        
        private:

        Position currentPosition_;
        std::vector<double> currentjointValues_;
        Position toolOffset_;
        Position workspaceMin_;
        Position workspaceMax_;
        bool isMoving_;
        std::function<void(const Position&)> positionCallback_;
        void checkWorkspaceLimit(const Position& position);


    };




} // namespace krl_interpreter



#endif //MOTION_CONTROLLER_HPP_