#ifndef PM_VIDEOITEM_H
#define PM_VIDEOITEM_H

#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define USE_FFMEDIA
#ifdef USE_FFMEDIA
#include "pmedia/player.h"
#include "pmedia/SDL_helper.h"
#endif

#ifdef main
#undef main
#endif

namespace pm {
	enum VIDEO_TYPE {
		VIDEO_TYPE_WEBCAM, VIDEO_TYPE_STREAM, VIDEO_TYPE_FILE
	};

	class VideoItemData
	{
	public:
		int type() const { return m_type; }
		int camId() const { return m_camId; }
		std::string videoName() const { return m_videoName; }

		void setType(int val) { m_type = val; }
		void setCamId(int val) { m_camId = val; }
		void setVideoName(std::string &val) { m_videoName = val; }

	protected:
		int m_type;
		int m_camId;
		std::string m_videoName;
	};

	class VideoItem
	{
	public:
		VideoItem();
		~VideoItem();

		bool open(const VideoItemData &vi);
		void release();
		bool isOpened();
		bool isPlaying();
		bool isPaused();

		bool read(cv::Mat &image);
		void togglePause();

	private:
		VideoItemData m_data;
		cv::VideoCapture m_cap;

		SDL_mutex *m_mutex;
		bool m_paused;
#ifdef USE_FFMEDIA
		pm::Player m_ffplayer;
		int64 m_pts;
#endif
	};

}

#endif //PM_VIDEOITEM_H
