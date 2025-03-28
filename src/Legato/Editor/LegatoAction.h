#pragma once

namespace Legato{


	class Action{
	public:
		Action(std::string name_) : name(name_){}
		
		std::string& getName(){
			return name;
		}
		
		void addSideEffect(std::shared_ptr<Action> sideEffect){
			sideEffects.push_back(sideEffect);
		}
		std::vector<std::shared_ptr<Action>>& getSideEffects(){
			return sideEffects;
		}
		
		void execute();
		
		virtual void onExecute() = 0;
		virtual void onUndo() = 0;
		virtual void onRedo(){ onExecute(); }
		
	private:
		std::string name;
		std::vector<std::shared_ptr<Action>> sideEffects;
	};


}
