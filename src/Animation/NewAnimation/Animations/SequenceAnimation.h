#pragma once

#include "../Animation.h"
#include "Legato/Editor/Parameters.h"

namespace AnimationSystem{

	class SequenceAnimation : public Animation{
		
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(SequenceAnimation)
		
	public:
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Animation::onCopyFrom(source);
		}
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		virtual AnimationType getType() override { return AnimationType::SEQUENCE; }
		
		virtual bool canStartPlayback() override { return false; }
		virtual void startPlayback() override {}
		virtual void stopPlayback() override {}
		
		virtual void parameterGui() override;
		
	private:
		
		//-curve interpolation type parameter
		//-curve start point (with time, velocity and ramps)
		//-curve target parameter (with time velocity and ramps)
		//-curve duration parameter
		
		std::shared_ptr<Legato::Parameter> curveStartParameter;
		std::shared_ptr<Legato::Parameter> curveTargetParameter;
		std::shared_ptr<Legato::Parameter> curveDeltaParameter;
		std::shared_ptr<Legato::NumberParameter<double>> rampInParameter;
		std::shared_ptr<Legato::NumberParameter<double>> rampOutParameter;
		Legato::TimeParam curveDurationParameter;
		
		virtual void onSetAnimatable() override;
		
	};

}
