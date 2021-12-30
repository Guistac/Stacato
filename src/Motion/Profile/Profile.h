#pragma once

namespace Motion{

//profile generator for 2nd degree motion profiles with constant acceleration
//TODO: start work on profile generator of 3rd degree with constant jerk

class Profile{
public:
	
	//used to initialize the profile
	void setPosition(double _position) { position = _position; }
	void setVelocity(double _velocity) { velocity = _velocity; }
	void setAcceleration(double _acceleration) { acceleration = _acceleration; }
	
	double getPosition() { return position; }
	double getVelocity(){ return velocity; }
	double getAcceleration() {return acceleration; }
	
	//Match a fixed target velocity using linear velocity interpolation
	void matchVelocity(double deltaT_seconds, double targetVelocity, double fixedAcceleration){
		double previousVelocity = velocity;
		if(velocity != targetVelocity){
			double deltaV = deltaT_seconds * std::abs(fixedAcceleration);
			if(velocity < targetVelocity) {
				//accelerate but don't overshoot target velocity
				velocity = std::min(velocity + deltaV, targetVelocity);
				acceleration = std::abs(fixedAcceleration);
			}
			else {
				//accelerate but don't overshoot target velocity
				velocity = std::max(velocity - deltaV, targetVelocity);
				acceleration = -std::abs(fixedAcceleration);
			}
		}else acceleration = 0.0;
		double averageVelocity = (previousVelocity + velocity) / 2.0;
		double deltaP = averageVelocity * deltaT_seconds;
		position += deltaP;
	}

	//Match a fixed target position
	void matchFixedPosition(double deltaT_seconds, double targetPosition, double fixedAcceleration, double maxVelocity){

	}
	
	//Match a moving target position
	void matchMovingPosition(double deltaT_seconds, double targetPosition, double targetVelocity, double targetAcceleration, double fixedAcceleration, double maxVelocity){
		
	}
	
private:
	
	double position = 0.0;
	double velocity = 0.0;
	double acceleration = 0.0;
};


}
