#pragma once

namespace Legato{


	class Action{
	public:
		
		std::string& getName(){ return name; }
		
		void addSideEffect(std::shared_ptr<Action> sideEffect){
			sideEffects.push_back(sideEffect);
		}
		bool hasSideEffects(){
			return !sideEffects.empty();
		}
		std::vector<std::shared_ptr<Action>>& getSideEffects(){
			return sideEffects;
		}
		
		virtual void onExecute() = 0;
		virtual void onUndo() = 0;
		virtual void onRedo(){ onExecute(); }
		
	protected:
		std::string name;
		
	private:
		std::vector<std::shared_ptr<Action>> sideEffects;
	};


}
