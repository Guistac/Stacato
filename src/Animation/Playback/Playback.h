
//class Manoeuvre;
class Animation;

namespace PlaybackManager {

	//watched manoeuvres
	//void push(std::shared_ptr<Manoeuvre> manoeuvre);
	//void pop(std::shared_ptr<Manoeuvre> manoeuvre);

	void push(std::shared_ptr<Animation> animation);
	void pop(std::shared_ptr<Animation> animation);

	//general
	//std::vector<std::shared_ptr<Manoeuvre>> getActiveManoeuvres();
	//bool isAnsyManoeuvreActive();
	//void stopAllManoeuvres();
	//void stopAllAnimations();

	std::vector<std::shared_ptr<Animation>> getActiveAnimations();
	bool isAnyAnimationActive();
	void stopAllAnimations();


	long long getTime_microseconds();

	//playback handling
	void update();
}
