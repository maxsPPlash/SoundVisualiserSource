#include "OpenCVVideoStream.h"

using namespace cv;

OpenCVVideoStream::OpenCVVideoStream() {
}

OpenCVVideoStream::~OpenCVVideoStream() {
}

bool OpenCVVideoStream::Initialise(const char* path) {
	video.setExceptionMode(true);
	try
	{
		video.open(path);
	}
	catch( cv::Exception& e )
	{
		const char* err_msg = e.what();
		std::cout << "exception caught: " << err_msg << std::endl;
	}
	int width  = (int)video.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)video.get(cv::CAP_PROP_FRAME_HEIGHT);

	std::cout << width << height << std::endl;

	return video.isOpened();
}

bool OpenCVVideoStream::Update(float cur_time) {
	if (!video.read(frame))
		return false;
//	video >> frame;
	return true;
}

bool OpenCVVideoStream::GetFrame(const unsigned char *& data, int &size) {
	data = frame.datastart;
	size = frame.total();

	return true;
}

bool OpenCVVideoStream::CopyFrame(void *data, unsigned int size) {
	cv::Mat frame_rgb;
	{
		cv::cvtColor(frame, frame_rgb, cv::COLOR_BGR2RGBA);

//		D3D11_MAPPED_SUBRESOURCE mappedTex;
//		r = m_pD3D11Ctx->Map(m_pSurfaceRGBA, subResource, D3D11_MAP_WRITE_DISCARD, 0, &mappedTex);
//		if (FAILED(r))
//		{
//			throw std::runtime_error("surface mapping failed!");
//		}

		cv::Mat m(288, 352, CV_8UC4, data, size);
		frame_rgb.copyTo(m);

//		m_pD3D11Ctx->Unmap(m_pSurfaceRGBA, subResource);
	}

	return true;
}