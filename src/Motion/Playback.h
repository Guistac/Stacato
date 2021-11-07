
class Manoeuvre;

namespace Playback {

	void startPriming(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void stopPriming(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPriming(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPrimed(const std::shared_ptr<Manoeuvre>& manoeuvre);
	float getPrimingProgress(const std::shared_ptr<Manoeuvre>& manoeuvre);

	void startPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void stopPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPlaying(const std::shared_ptr<Manoeuvre>& manoeuvre);

	void stopAllManoeuvres();

	void updateActiveManoeuvreState();

}