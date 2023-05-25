
class Animation;
class Manoeuvre;

namespace PlaybackManager {

	//starting watching those
	void push(std::shared_ptr<Animation> animation);
	void push(std::shared_ptr<Manoeuvre> manoeuvre);

	//general
	std::vector<std::shared_ptr<Animation>> getActiveAnimations();
	std::vector<std::shared_ptr<Manoeuvre>> getActiveManoeuvres();
	bool isAnyAnimationActive();
	void stopAllAnimations();
	long long getTime_microseconds();

	//playback handling
	void update();

	void setPlaybackSpeedMultiplier(double multiplier);
	void resetPlaybackSpeedMultiplier();
	double getPlaybackSpeedMultiplier();
	double getMinPlaybackSpeedMultiplier();
	double getMaxPlaybackSpeedMutliplier();
}
