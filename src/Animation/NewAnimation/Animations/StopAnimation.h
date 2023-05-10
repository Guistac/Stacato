#pragma once

#include "../Animation.h"
#include "Legato/Editor/Parameters.h"

namespace AnimationSystem{

	class StopAnimation : public Animation{
		
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StopAnimation)
		
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
		
		virtual AnimationType getType() override { return AnimationType::STOP; }
		
		virtual bool canStartPlayback() override { return false; }
		virtual void startPlayback() override {}
		virtual void stopPlayback() override {}
		
		virtual void parameterGui() override{}
		
	private:
		
		//-curve interpolation type parameter
		//-constraints to build curve on playback start:
		//	-stopping ramp
		
		
		std::shared_ptr<Legato::Parameter> stopRampParameter;
		
	};

}
