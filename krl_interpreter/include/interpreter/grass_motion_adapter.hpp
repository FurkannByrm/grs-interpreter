// #ifndef GRASS_MOTION_ADAPTER_HPP_
// #define GRASS_MOTION_ADAPTER_HPP_

// #include "interpreter/motion_controller.hpp"
// #include "axis_manager.h"
// #include "config_loader.h"
// #include "motion_profile_generator.h"

// namespace krl_interpreter {

// /**
//  * @brief Adapter class to integrate GRASS Motion Controller with KRL Interpreter
//  */
// class GrassMotionAdapter : public MotionController {
// public:
//     GrassMotionAdapter();
//     ~GrassMotionAdapter() override;

//     // Initialize with GRASS configuration
//     bool initialize(const std::string& configPath, int axisCount = 6);
    
//     // Override base MotionController methods
//     void moveToPosition(const Position& target, MotionType type, const MotionParameters& params) override;
//     void moveJoints(const std::vector<double>& jointValues, const MotionParameters params) override;
//     void stop() override;
//     void emergency() override;
    
//     Position getCurrentPosition() const override;
//     std::vector<double> getCurrentJointValues() const override;
//     bool isMoving() const override;

//     // GRASS-specific functionality
//     void enableAllAxes();
//     void disableAllAxes();
//     bool isAxisConnected(int axisId) const;
//     void setAxisLimits(int axisId, double minPos, double maxPos);
    
// private:
//     std::unique_ptr<AxisManager> axisManager_;
//     std::unique_ptr<ConfigLoader> configLoader_;
//     std::unique_ptr<MotionProfileGenerator> motionProfileGen_;
    
//     bool initialized_;
//     int axisCount_;
    
//     // Helper methods
//     Position jointsToCartesian(const std::vector<double>& joints) const;
//     std::vector<double> cartesianToJoints(const Position& position) const;
//     void updateCurrentPositionFromAxes();
//     bool checkAllAxesReady() const;
// };

// } // namespace krl_interpreter

// #endif // GRASS_MOTION_ADAPTER_HPP_
