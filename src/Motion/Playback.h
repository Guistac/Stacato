
class Manoeuvre;

namespace Playback {

	void rapidToStart(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void rapidToEnd(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void rapidToPlaybackPosition(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void stopRapid(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isInRapid(const std::shared_ptr<Manoeuvre>& manoeuvre);
	float getRapidProgress(const std::shared_ptr<Manoeuvre>& manoeuvre);

	bool isPrimedToStart(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPrimedToEnd(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPrimedToPlaybackPosition(const std::shared_ptr<Manoeuvre>& manoeuvre);

	void startPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void pausePlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void resumePlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	void stopPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPlaying(const std::shared_ptr<Manoeuvre>& manoeuvre);
	bool isPaused(const std::shared_ptr<Manoeuvre>& manoeuvre);

	void stopAllManoeuvres();

	void updateActiveManoeuvreState();

}