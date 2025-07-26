#ifndef MOTION_CONTROLLER_HPP_
#define MOTION_CONTROLLER_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "../common/utils.hpp"

namespace krl_interpreter{

using namespace common;

    class MotionController{ 

        public:
        MotionController();
        virtual ~MotionController();

        virtual void moveToPosition(const Position& target, MotionType type, const MotionParameters& params);
        virtual void moveJoints(const std::vector<double>& jointValues, const MotionParameters params);
        virtual void wait(double seconds);
        virtual void stop();
        virtual void emergency();

        virtual Position getCurrentPosition() const;
        virtual std::vector<double> getCurrentJointValues() const;
        virtual bool isMoving() const;

        virtual void setToolOffset(const Position& offset);
        virtual void setWorkspaceLimit(const Position& min, Position& max);
        virtual void setPositionCallback(std::function<void(const Position&)> callback);
        
        protected:

        Position currentPosition_;
        Frame setFramePosition_;
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