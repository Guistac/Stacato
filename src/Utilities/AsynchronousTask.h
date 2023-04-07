#pragma once

class AsynchronousTask{
public:
	
	void execute(){
		if(b_isRunning) return;
		b_isRunning = true;
		std::thread taskHandler([this](){
			onExecution();
			b_isRunning = false;
		});
		taskHandler.detach();
	}
	
	bool isRunning(){
		return b_isRunning;
	}
	
	std::string getStatusString(){
		std::lock_guard<std::mutex> lock(mutex);
		return statusString;
	}
	
	virtual void onExecution() = 0;
	virtual bool canStart() = 0;
	
protected:
	void setStatusString(std::string status){
		std::lock_guard<std::mutex> lock(mutex);
		statusString = status;
	}
private:
	std::string statusString;
	bool b_isRunning = false;
	std::mutex mutex;
};
