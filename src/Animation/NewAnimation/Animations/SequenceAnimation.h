#pragma once

#include "../Animation.h"
#include "Legato/Editor/Parameters.h"

namespace AnimationSystem{

	class SequenceAnimation : public Animation{
		
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(SequenceAnimation)
		
	public:
		
		virtual void onConstruction() override{
			Animation::onConstruction();
		}
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Animation::onCopyFrom(source);
		}
		virtual bool onSerialization() override{
			bool success = Animation::onSerialization();
			return success;
		}
		virtual bool onDeserialization() override{
			bool success = Animation::onDeserialization();
			return success;
		}
		
		virtual AnimationType getType() override { return AnimationType::SEQUENCE; }
		
		virtual bool canStartPlayback() override { return false; }
		virtual void startPlayback() override {}
		virtual void stopPlayback() override {}
		
		virtual void parameterGui() override{}
		
	private:
		
		//-curve interpolation type parameter
		//-curve start point (with time, velocity and ramps)
		//-curve target parameter (with time velocity and ramps)
		//-curve duration parameter
		
		std::shared_ptr<Legato::Parameter> curveStartParameter;
		std::shared_ptr<Legato::Parameter> curveTargetParameter;
		std::shared_ptr<Legato::Parameter> curveDeltaParameter;
		Legato::TimeParam curveDurationParameter;
		
	};

}
