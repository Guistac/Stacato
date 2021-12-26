#pragma once

struct ProgressIndicator{
	
	float progress = 0.0;
	char progressString[512] = "";
	
	void setStart(const char* progressMessage){
		b_finished = false;
		b_succeeded = false;
		progress = 0.0;
		strcpy(progressString, progressMessage);
	}
	
	void setProgress(float _progress, const char* progressMessage){
		b_finished = false;
		b_succeeded = false;
		progress = 0.0;
		strcpy(progressString, progressMessage);
	}
	
	void setCompletion(const char* completionMessage){
		b_finished = true;
		b_succeeded = true;
		progress = 1.0;
		strcpy(progressString, completionMessage);
		finishTime_seconds = Timing::getProgramTime_seconds();
	}
	
	void setFailure(const char* errorMessage){
		b_finished = true;
		b_succeeded = false;
		progress = 0.0;
		strcpy(progressString, errorMessage);
	}
	
	const char* getProgressString(){
		return progressString;
	}
	
	double getTimeSinceCompletion(){
		if(b_succeeded && b_finished) return Timing::getProgramTime_seconds() - finishTime_seconds;
		else return 0.0;
	}
	
	bool failed(){
		return b_finished && !b_succeeded;
	}
	
	bool succeeded(){
		return b_finished && b_succeeded;
	}
	
	bool b_finished = false;
	bool b_succeeded = false;
	double finishTime_seconds = 0.0;
};
