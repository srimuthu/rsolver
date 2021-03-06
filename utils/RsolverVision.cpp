#include <thread>
#include <chrono>
#include "RsolverVision.h"
#include "RsolverHelpers.h"

namespace Rsolver {
	RsolverVision::RsolverVision(int deviceId)
		:m_deviceId(deviceId)
		,m_width(g_defaultWidth)
		,m_height(g_defaultHeight)
	{
		SetCaptureDimensions(m_width, m_height);
		CreateCubiesPositions();
		InitializeDefaultColorBoundaries();
	}

	RsolverVision::~RsolverVision()
	{
		m_videoCapture.release();
	}

	cv::Mat RsolverVision::CaptureImageFromSensor()
	{
		cv::Mat originalFrame;
		OpenVideoCapture();
		int stabilizationFrames = 5;
		int i = 0;
		while (i < stabilizationFrames)
		{
			m_videoCapture >> originalFrame;
			if (originalFrame.empty())
			{
				throw(std::runtime_error("Retrieved image from sensor is empty!"));
			}
			cv::waitKey(5);
			i++;
		}
		m_videoCapture.release();
		return originalFrame;
	}

	CubeFaceInfo RsolverVision::GetCubeFaceInfoColorsFromImage(const cv::Mat & faceImage)
	{
		CubeFaceInfo cubeFaceInfo;
		for (auto i = 0; i < g_cubiesPerFace; i++)
		{
			cv::Mat cubie = GetCubieAtIndex(faceImage, i);
			cubeFaceInfo.faceColorVector.emplace_back(DetectColorOfCubie(cubie));
		}
		cubeFaceInfo.cubeFace = Helpers::GetDefaultCubeFaceForColor(cubeFaceInfo.faceColorVector.at(g_centerCubieIndex));
		return cubeFaceInfo;
	}

	void RsolverVision::CalibrateCubeCameraDistanceGUI(bool testCapture)
	{
		cv::Mat originalFrame, overlayFrame;
		int keycount = 0;
		OpenVideoCapture();
		while (true)
		{
			m_videoCapture >> originalFrame;
			if (originalFrame.empty())
			{
				throw(std::runtime_error("Retrieved image from sensor is empty!"));
			}
			originalFrame.copyTo(overlayFrame);
			OverlayCubiesExpectedPositionOnImage(overlayFrame);
			cv::imshow("Calibrate cube camera distance", overlayFrame);
			int key = cv::waitKey(5);
			key = (key == 255) ? -1 : key;
			if (key >= 0)
			{
				if (testCapture)
				{
					std::string filename = "cube_" + std::to_string(keycount) + ".png";
					keycount++;
					std::cout << "capturing" << filename << std::endl;
					cv::imwrite(filename, originalFrame);
				}
				else
				{
					keycount = g_numFaces;
				}
				if (keycount >= g_numFaces)	break;
			}
		}
		m_videoCapture.release();
	}

	void RsolverVision::OpenVideoCapture()
	{
		m_videoCapture.open(m_deviceId);
		if (!m_videoCapture.isOpened())
		{
			throw(std::runtime_error("Unable to open video capture"));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	void RsolverVision::SetCaptureDimensions(int width, int height)
	{
		m_videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, width);
		m_videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
	}

	void RsolverVision::CreateCubiesPositions()
	{
		auto overlay = CubeOverlayDefaults();
		cv::Rect cubie;
		for (auto i = 0; i < 3; i++)
		{
			for (auto j = 0; j < 3; j++)
			{
				cubie = cv::Rect(overlay.xPosTopLeft + (j*overlay.cubieSizeInPix) + (overlay.toleranceInPix / 2),
					overlay.yPosTopLeft + (i*overlay.cubieSizeInPix) + (overlay.toleranceInPix / 2),
					overlay.cubieSizeInPix - overlay.toleranceInPix,
					overlay.cubieSizeInPix - overlay.toleranceInPix);
				m_cubiesPos.emplace_back(cubie);
			}
		}
	}

	void RsolverVision::OverlayCubiesExpectedPositionOnImage(cv::Mat & image)
	{
		for (auto const& cubie : m_cubiesPos)
		{
			cv::rectangle(image, cubie, cv::Scalar(255, 0, 0), 4);
		}
	}

	void RsolverVision::InitializeDefaultColorBoundaries()
	{
		for (auto i = 0; i < g_numFaces; i++)
		{
			ColorBoundaries colorBoundaries(static_cast<Colors>(i));
			m_colorBoundariesVec.emplace_back(colorBoundaries);
		}
	}

	LabVal RsolverVision::GetLabValFromCubie(const cv::Mat & image)
	{
		cv::Mat imageLab;
		cv::cvtColor(image, imageLab, cv::COLOR_BGR2Lab);
		std::vector<cv::Mat> labPlanes(3);
		cv::split(imageLab, labPlanes);

		int histSize = 256;
		float range[] = { 0, 256 };
		const float* histRange = { range };
		bool uniform = true, accumulate = false;

		cv::Mat bHist, gHist, rHist;
		cv::calcHist(&labPlanes[0], 1, 0, cv::Mat(), bHist, 1, &histSize, &histRange, uniform, accumulate);
		cv::calcHist(&labPlanes[1], 1, 0, cv::Mat(), gHist, 1, &histSize, &histRange, uniform, accumulate);
		cv::calcHist(&labPlanes[2], 1, 0, cv::Mat(), rHist, 1, &histSize, &histRange, uniform, accumulate);
		LabVal labVal;
		double minVal, maxVal;
		cv::Point minLoc, maxLoc;
		cv::minMaxLoc(bHist, &minVal, &maxVal, &minLoc, &maxLoc);
		labVal.lVal = maxLoc.y;
		cv::minMaxLoc(gHist, &minVal, &maxVal, &minLoc, &maxLoc);
		labVal.aVal = maxLoc.y;
		cv::minMaxLoc(rHist, &minVal, &maxVal, &minLoc, &maxLoc);
		labVal.bVal = maxLoc.y;

		return labVal;
	}

	std::vector<ColorBoundaries> RsolverVision::GetColorBoundariesVector()
	{
		return m_colorBoundariesVec;
	}

	cv::Mat RsolverVision::GetCubieAtIndex(const cv::Mat& inputImage, int index)
	{
		if (index >= g_cubiesPerFace)
		{
			throw(std::runtime_error("Invalid cubie index, cubie index must be less than " + std::to_string(g_cubiesPerFace)));
		}
		cv::Mat copyOfInputImage;
		inputImage.copyTo(copyOfInputImage);
		cv::Mat croppedCubie = copyOfInputImage(m_cubiesPos.at(index));
		cv::Mat croppedCubieGB;
		cv::GaussianBlur(croppedCubie, croppedCubieGB, cv::Size(7, 7), 0, 0);
		return croppedCubieGB;
	}

	Colors RsolverVision::DetectColorOfCubie(const cv::Mat & cubie)
	{
		LabVal labVal = GetLabValFromCubie(cubie);
		for (auto const& cb : m_colorBoundariesVec)
		{
			// Compare A and B components in LAB colorspace
			if ((labVal.aVal <= (cb.labVal.aVal + g_histTolerance)) && (labVal.aVal >= (cb.labVal.aVal - g_histTolerance)) &&
				(labVal.bVal <= (cb.labVal.bVal + g_histTolerance)) && (labVal.bVal >= (cb.labVal.bVal - g_histTolerance)))
			{
				return cb.color;
			}
		}
		throw(std::runtime_error("Unable to identify the color of the cubie! Try recalibration"));
	}

	void RsolverVision::PerformBoundariesCalibrationByFaceColor(const cv::Mat & faceImage, Colors color)
	{
		cv::Mat centerCubie = GetCubieAtIndex(faceImage, g_centerCubieIndex);
		// Given the face color (of the center cubie) calibrate boundaries for that color
		for (auto& cb : m_colorBoundariesVec)
		{
			if (cb.color == color)
			{
				cb.labVal = GetLabValFromCubie(centerCubie);
			}
		}
	}
}

