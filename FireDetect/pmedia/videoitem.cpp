#include "videoitem.h"
#include "pmedia/glrenderer.h"

namespace pm {
	typedef struct camera_property_t
	{
		cv::Size frameSize;
		int fourcc;
		double fps;

	} camera_property_t;

	void getProperties(cv::VideoCapture &cap, camera_property_t &prop)
	{
		prop.frameSize = cv::Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		prop.fourcc = (int)cap.get(CV_CAP_PROP_FOURCC);
		prop.fps = cap.get(CV_CAP_PROP_FPS);
	}

#if 0
	static void setMaxSize(cv::VideoCapture &cap, camera_property_t &prop)
	{
		static int widthList[] =  { 1920, 1280, 800, 640, 320, 0};
		static int heightList[] = { 1080, 720,  600, 480, 240, 0};
		int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
		int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);

		for (int i = 0; widthList[i] > 0; i ++) {
			if (widthList[i] <= width)
				break;

			try {
				cap.set(CV_CAP_PROP_FRAME_WIDTH, widthList[i]);
				cap.set(CV_CAP_PROP_FRAME_HEIGHT, heightList[i]);
				getProperties(cap, prop);
				if (prop.frameSize.width == widthList[i])
					break;
			} catch (...) {
				break;
			}
		}
	}
#else
	static void setMaxSize(cv::VideoCapture &cap, camera_property_t &prop)
	{
		const int width = 1920;
		const int height = 1080;

		getProperties(cap, prop);
		if (width == prop.frameSize.width && height == prop.frameSize.height)
			return;

		try {
			cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
			cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
			getProperties(cap, prop);
		}
		catch (...) {
			//qDebug() << "Couldn't change the webcam's resolution.";
		}
	}
#endif

	static bool convVideo2Image(pm::VideoFrame *pict, cv::Mat &image)
	{
		bool res = false;
		if (pict && pict->buff) {
			image.create(pict->height, pict->width, CV_8UC3);
			uchar *src, *dst;
			int lineSize = pict->width * 3;
			for (int y = 0; y < pict->height; y++) {
				src = (uchar*)&pict->buff[lineSize*y];
				dst = image.ptr<uchar>(y);
				memcpy(dst, src, lineSize);
			}
			res = true;
		}
		return res;
	}

	VideoItem::VideoItem()
	{
		m_paused = false;
#ifdef USE_FFMEDIA
		m_pts = -1;
#endif
		m_mutex = SDL_CreateMutex();
	}

	VideoItem::~VideoItem()
	{
		if (m_mutex)
			SDL_DestroyMutex(m_mutex);
		release();
	}

	bool VideoItem::open(const VideoItemData &data)
	{
		bool res = false;
		if (isOpened())
			release();

		m_data = data;

		if (data.type() == pm::VIDEO_TYPE_WEBCAM) {
			res = m_cap.open(data.camId());
			if (res) {
				camera_property_t prop;
#if 1
				setMaxSize(m_cap, prop);
#endif
			}
		}
		else {
#ifdef USE_FFMEDIA
			m_pts = -1;
			m_ffplayer.setOpt(pm::OPT_SYNC_TIME, 0);
			res = m_ffplayer.open(data.videoName().c_str());
			if (res)
				res = m_ffplayer.play();
#else
			res = m_cap.open(data.videoName());
#endif
		}
		return res;
	}

	void VideoItem::release()
	{
		if (m_cap.isOpened())
			m_cap.release();
#ifdef USE_FFMEDIA
		if (m_ffplayer.isOpened())
			m_ffplayer.release();
#endif
		m_paused = false;
	}

	bool VideoItem::isOpened()
	{
		SDL_MutexLocker l(m_mutex);
		bool res = m_cap.isOpened();
#ifdef USE_FFMEDIA
		if (!res)
			res = m_ffplayer.isOpened();
#endif
		return res;
	}

	bool VideoItem::isPlaying()
	{
		SDL_MutexLocker l(m_mutex);
		bool res = false;
		if (m_cap.isOpened()) {
			res = !m_paused;
		}
#ifdef USE_FFMEDIA
		if (!res) {
			res = m_ffplayer.isPlaying();
		}
#endif
		return res;
	}

	bool VideoItem::isPaused()
	{
		SDL_MutexLocker l(m_mutex);
		bool res = false;
		if (m_cap.isOpened()) {
			res = m_paused;
		}
#ifdef USE_FFMEDIA
		if (!res) {
			res = m_ffplayer.isPaused();
		}
#endif
		return res;
	}

	bool VideoItem::read(cv::Mat &image)
	{
		SDL_MutexLocker l(m_mutex);
		bool res = false;

		if (m_cap.isOpened()) {
			if (!m_paused)
				res = m_cap.read(image);
			else
				res = false;
		}
#ifdef USE_FFMEDIA
		else if (m_ffplayer.isOpened()) {
			pm::VideoFrame *pict = m_ffplayer.updateFrame();
			if (pict && pict->buff && pict->pkt_pts > m_pts) {
				m_pts = pict->pkt_pts;
				convVideo2Image(pict, image);
				res = true;
			}
		}
#endif
		return res;
	}

	void VideoItem::togglePause()
	{
		bool paused = isPaused();

		SDL_MutexLocker l(m_mutex);

		if (m_cap.isOpened()) {
			m_paused = !paused;
		}
#ifdef USE_FFMEDIA
		else if (m_ffplayer.isOpened()) {
			m_ffplayer.pause();
			m_paused = m_ffplayer.isPaused();
		}
#endif
	}
}
