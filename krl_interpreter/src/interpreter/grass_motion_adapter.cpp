#include "interpreter/grass_motion_adapter.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace krl_interpreter {

GrassMotionAdapter::GrassMotionAdapter() 
    : MotionController(),
      axisManager_(std::make_unique<AxisManager>()),
      configLoader_(std::make_unique<ConfigLoader>()),
      motionProfileGen_(std::make_unique<MotionProfileGenerator>()),
      initialized_(false),
      axisCount_(6) {
}

GrassMotionAdapter::~GrassMotionAdapter() {
    if (initialized_) {
        stop();
        disableAllAxes();
    }
}

bool GrassMotionAdapter::initialize(const std::string& configPath, int axisCount) {
    try {
        axisCount_ = axisCount;
        
        // Load configuration
        if (!configLoader_->loadFromFile(configPath)) {
            std::cerr << "Failed to load configuration from: " << configPath << std::endl;
            return false;
        }
        
        // Initialize axes
        if (!axisManager_->loadFromConfig(*configLoader_, axisCount_)) {
            std::cerr << "Failed to initialize axes from configuration" << std::endl;
            return false;
        }
        
        // Enable all axes
        enableAllAxes();
        
        // Update current position from axes
        updateCurrentPositionFromAxes();
        
        initialized_ = true;
        std::cout << "GRASS Motion Controller initialized successfully with " 
                  << axisCount_ << " axes" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing GRASS Motion Controller: " << e.what() << std::endl;
        return false;
    }
}

void GrassMotionAdapter::moveToPosition(const Position& target, MotionType type, const MotionParameters& params) {
    if (!initialized_) {
        throw std::runtime_error("GRASS Motion Controller not initialized");
    }
    
    checkWorkspaceLimit(target);
    
    // Apply tool offset
    Position adjustedTarget = {
        target.x + toolOffset_.x,
        target.y + toolOffset_.y,
        target.z + toolOffset_.z,
        target.a + toolOffset_.a,
        target.b + toolOffset_.b,
        target.c + toolOffset_.c
    };
    
    std::cout << "GRASS Motion: Moving to position (" 
              << adjustedTarget.x << ", " << adjustedTarget.y << ", " << adjustedTarget.z << ")" << std::endl;
    
    // Convert Cartesian to joint space
    std::vector<double> targetJoints = cartesianToJoints(adjustedTarget);
    
    // Move joints using GRASS Motion Controller
    moveJoints(targetJoints, params);
}

void GrassMotionAdapter::moveJoints(const std::vector<double>& jointValues, const MotionParameters params) {
    if (!initialized_) {
        throw std::runtime_error("GRASS Motion Controller not initialized");
    }
    
    if (jointValues.size() != static_cast<size_t>(axisCount_)) {
        throw std::runtime_error("Joint values size mismatch");
    }
    
    if (!checkAllAxesReady()) {
        throw std::runtime_error("Not all axes are ready for motion");
    }
    
    isMoving_ = true;
    
    try {
        // Create motion profile for each axis
        for (int i = 0; i < axisCount_; ++i) {
            Axis* axis = axisManager_->getAxis(i + 1); // 1-based indexing
            if (axis) {
                // Generate motion profile
                MotionProfile_t profile;
                profile.maxVelocity = static_cast<Float_t>(params.velocity);
                profile.maxAcceleration = static_cast<Float_t>(params.acceleration);
                profile.maxDeceleration = static_cast<Float_t>(params.deceleration);
                profile.maxJerk = static_cast<Float_t>(params.jerk);
                
                // Set target position
                axis->setPosition(static_cast<Float_t>(jointValues[i]), true); // with trajectory
            }
        }
        
        // Wait for motion completion
        bool allCompleted = false;
        while (!allCompleted && isMoving_) {
            allCompleted = true;
            for (int i = 0; i < axisCount_; ++i) {
                Axis* axis = axisManager_->getAxis(i + 1);
                if (axis && axis->isMoving()) {
                    allCompleted = false;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // Update current joint values
        currentjointValues_ = jointValues;
        
        // Update current position
        updateCurrentPositionFromAxes();
        
        // Call position callback if set
        if (positionCallback_) {
            positionCallback_(currentPosition_);
        }
        
        isMoving_ = false;
        std::cout << "GRASS Motion: Joint motion completed" << std::endl;
        
    } catch (const std::exception& e) {
        isMoving_ = false;
        std::cerr << "Error during joint motion: " << e.what() << std::endl;
        throw;
    }
}

void GrassMotionAdapter::stop() {
    if (initialized_) {
        // Stop all axes
        for (int i = 0; i < axisCount_; ++i) {
            Axis* axis = axisManager_->getAxis(i + 1);
            if (axis) {
                axis->stop();
            }
        }
        isMoving_ = false;
        std::cout << "GRASS Motion: All axes stopped" << std::endl;
    }
}

void GrassMotionAdapter::emergency() {
    if (initialized_) {
        std::cout << "GRASS Motion: EMERGENCY STOP!" << std::endl;
        
        // Emergency stop all axes
        for (int i = 0; i < axisCount_; ++i) {
            Axis* axis = axisManager_->getAxis(i + 1);
            if (axis) {
                axis->emergencyStop();
            }
        }
        isMoving_ = false;
    }
}

Position GrassMotionAdapter::getCurrentPosition() const {
    return currentPosition_;
}

std::vector<double> GrassMotionAdapter::getCurrentJointValues() const {
    if (!initialized_) {
        return currentjointValues_;
    }
    
    std::vector<double> joints(axisCount_);
    for (int i = 0; i < axisCount_; ++i) {
        Axis* axis = axisManager_->getAxis(i + 1);
        if (axis) {
            joints[i] = static_cast<double>(axis->getCurrentPosition());
        }
    }
    return joints;
}

bool GrassMotionAdapter::isMoving() const {
    if (!initialized_) {
        return isMoving_;
    }
    
    // Check if any axis is moving
    for (int i = 0; i < axisCount_; ++i) {
        Axis* axis = axisManager_->getAxis(i + 1);
        if (axis && axis->isMoving()) {
            return true;
        }
    }
    return false;
}

void GrassMotionAdapter::enableAllAxes() {
    if (initialized_) {
        axisManager_->enableAll();
        std::cout << "GRASS Motion: All axes enabled" << std::endl;
    }
}

void GrassMotionAdapter::disableAllAxes() {
    if (initialized_) {
        axisManager_->disableAll();
        std::cout << "GRASS Motion: All axes disabled" << std::endl;
    }
}

bool GrassMotionAdapter::isAxisConnected(int axisId) const {
    if (!initialized_) {
        return false;
    }
    
    Axis* axis = axisManager_->getAxis(axisId);
    return axis && axis->isConnected();
}

void GrassMotionAdapter::setAxisLimits(int axisId, double minPos, double maxPos) {
    if (!initialized_) {
        return;
    }
    
    Axis* axis = axisManager_->getAxis(axisId);
    if (axis) {
        axis->setPositionLimits(static_cast<Float_t>(minPos), static_cast<Float_t>(maxPos));
    }
}

Position GrassMotionAdapter::jointsToCartesian(const std::vector<double>& joints) const {
    // Simplified forward kinematics - replace with actual implementation
    Position pos;
    if (joints.size() >= 6) {
        // This is a placeholder - implement actual forward kinematics
        pos.x = joints[0] * 100.0; // Scale factor
        pos.y = joints[1] * 100.0;
        pos.z = joints[2] * 100.0;
        pos.a = joints[3];
        pos.b = joints[4];
        pos.c = joints[5];
    }
    return pos;
}

std::vector<double> GrassMotionAdapter::cartesianToJoints(const Position& position) const {
    // Simplified inverse kinematics - replace with actual implementation
    std::vector<double> joints(axisCount_);
    
    // This is a placeholder - implement actual inverse kinematics
    joints[0] = position.x / 100.0; // Scale factor
    joints[1] = position.y / 100.0;
    joints[2] = position.z / 100.0;
    joints[3] = position.a;
    joints[4] = position.b;
    joints[5] = position.c;
    
    return joints;
}

void GrassMotionAdapter::updateCurrentPositionFromAxes() {
    if (!initialized_) {
        return;
    }
    
    // Get current joint values from axes
    std::vector<double> joints = getCurrentJointValues();
    
    // Convert to Cartesian coordinates
    currentPosition_ = jointsToCartesian(joints);
    currentjointValues_ = joints;
}

bool GrassMotionAdapter::checkAllAxesReady() const {
    if (!initialized_) {
        return false;
    }
    
    for (int i = 0; i < axisCount_; ++i) {
        Axis* axis = axisManager_->getAxis(i + 1);
        if (!axis || !axis->isConnected() || !axis->isEnabled()) {
            return false;
        }
    }
    return true;
}

} // namespace krl_interpreter
