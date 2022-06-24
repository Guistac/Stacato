
class Manoeuvre;

namespace PlaybackManager {

	//watched manoeuvres
	void push(std::shared_ptr<Manoeuvre> manoeuvre);
	void pop(std::shared_ptr<Manoeuvre> manoeuvre);

	//general
	std::vector<std::shared_ptr<Manoeuvre>>& getActiveManoeuvres();
	bool isAnyManoeuvreActive();
	void stopAllManoeuvres();

	long long getTime_microseconds();

	//playback handling
	void update();
}
