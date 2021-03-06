/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          http://www.mrpt.org/                          |
   |                                                                        |
   | Copyright (c) 2005-2018, Individual contributors, see AUTHORS file     |
   | See: http://www.mrpt.org/Authors - All rights reserved.                |
   | Released under BSD License. See details in http://www.mrpt.org/License |
   +------------------------------------------------------------------------+ */

#include <mrpt/img/CImage.h>
#include <mrpt/img/TColor.h>
#include <mrpt/serialization/CArchive.h>
#include <mrpt/io/CMemoryStream.h>
#include <mrpt/math/CMatrixTemplateNumeric.h>
#include <CTraitsTest.h>
#include <gtest/gtest.h>
#include <test_mrpt_common.h>

// Universal include for all versions of OpenCV
#include <mrpt/otherlibs/do_opencv_includes.h>

template class mrpt::CTraitsTest<mrpt::img::CImage>;

using namespace std::string_literals;
const auto tstImgFileColor =
	mrpt::UNITTEST_BASEDIR + "/samples/img_basic_example/frame_color.jpg"s;

TEST(CImage, CtorDefault)
{
	mrpt::img::CImage img;
	EXPECT_THROW(img.isColor(), std::exception);
}

#if MRPT_HAS_OPENCV

TEST(CImage, CtorSized)
{
	using namespace mrpt::img;
	{
		CImage img(64, 48, CH_RGB);
		EXPECT_EQ(img.getWidth(), 64U);
		EXPECT_EQ(img.getHeight(), 48U);
		EXPECT_EQ(img.getChannelCount(), 3U);
		EXPECT_EQ(img.getPixelDepth(), PixelDepth::D8U);
		EXPECT_TRUE(img.isColor());
	}
	{
		CImage img(64, 48, CH_GRAY);
		EXPECT_EQ(img.getWidth(), 64U);
		EXPECT_EQ(img.getHeight(), 48U);
		EXPECT_EQ(img.getChannelCount(), 1U);
		EXPECT_EQ(img.getPixelDepth(), PixelDepth::D8U);
		EXPECT_FALSE(img.isColor());
	}
}

TEST(CImage, GetSetPixel)
{
	using namespace mrpt::img;
	CImage img(20, 10, CH_GRAY);
	img.setPixel(10, 2, TColor(0x80, 0x80, 0x80));
	EXPECT_EQ(img.at<uint8_t>(10, 2), 0x80);

	img.setPixel(11, 2, TColor(0x0, 0x0, 0x0));
	EXPECT_EQ(img.at<uint8_t>(11, 2), 0x00);

	img.setPixel(12, 2, TColor(0xff, 0xff, 0xff));
	EXPECT_EQ(img.at<uint8_t>(12, 2), 0xff);

	img.at<uint8_t>(13, 2) = 0x70;
	EXPECT_EQ(img.at<uint8_t>(13, 2), 0x70);

	auto* line = img.ptrLine<uint8_t>(5);
	for (uint8_t i = 0; i < 20; i++)
	{
		line[i] = i;
		EXPECT_EQ(img.at<uint8_t>(i, 5), i);
	}

	mrpt::math::CMatrixFloat M;
	img.getAsMatrix(M, true, 0, 0, -1, -1, false /* dont normalize (0,1) */);
	for (uint8_t i = 0; i < 20; i++)
	{
		EXPECT_NEAR(static_cast<double>(M(5, i)), i, 1e-8);
	}
}

TEST(CImage, CopyMoveSwap)
{
	using namespace mrpt::img;
	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// Shallow copy:
		CImage b = a;
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);

		a.at<uint8_t>(1, 3) = 0x81;
		EXPECT_EQ(b.at<uint8_t>(1, 3), 0x81);

		// Deep copy:
		CImage c = a.makeDeepCopy();
		EXPECT_EQ(c.at<uint8_t>(1, 2), 0x80);

		c.at<uint8_t>(1, 3) = 0x0;
		a.at<uint8_t>(1, 3) = 0x81;
		EXPECT_NE(c.at<uint8_t>(1, 3), 0x81);
	}

	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// Shallow copy ctor:
		CImage b(a, mrpt::img::SHALLOW_COPY);
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);

		a.at<uint8_t>(1, 3) = 0x81;
		EXPECT_EQ(b.at<uint8_t>(1, 3), 0x81);

		// Deep copy ctor:
		CImage c(a, mrpt::img::DEEP_COPY);
		EXPECT_EQ(c.at<uint8_t>(1, 2), 0x80);

		c.at<uint8_t>(1, 3) = 0x0;
		a.at<uint8_t>(1, 3) = 0x81;
		EXPECT_NE(c.at<uint8_t>(1, 3), 0x81);
	}

	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// Deep copy:
		CImage b = a.makeDeepCopy();
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);

		a.clear();
		a.resize(30, 30, CH_RGB);
		b.at<uint8_t>(1, 3) = 0x0;
		a.at<uint8_t>(1, 3) = 0x81;
		EXPECT_NE(b.at<uint8_t>(1, 3), 0x81);
	}

	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// move:
		CImage b = std::move(a);
		EXPECT_EQ(b.getWidth(), 20U);
		EXPECT_EQ(b.getHeight(), 10U);
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);
	}

	{
		CImage a(20, 10, CH_GRAY), b;
		a.at<uint8_t>(1, 2) = 0x80;
		// swap:
		a.swap(b);
		EXPECT_EQ(b.getWidth(), 20U);
		EXPECT_EQ(b.getHeight(), 10U);
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);
	}
}

TEST(CImage, ExternalImage)
{
	using namespace mrpt::img;
	{
		CImage a;
		a.setExternalStorage(tstImgFileColor);
		// Test automatic load-on-the-fly:
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}

	{
		CImage a;
		a.setExternalStorage("./foo_61717181.png");
		// Test exception on not found
		EXPECT_THROW(a.getWidth(), mrpt::img::CExceptionExternalImageNotFound);
	}
}

TEST(CImage, ConvertGray)
{
	using namespace mrpt::img;
	{
		CImage a;
		bool load_ok = a.loadFromFile(tstImgFileColor);
		EXPECT_TRUE(load_ok);

		CImage b = a.grayscale();
		EXPECT_EQ(b.getWidth(), a.getWidth());
		EXPECT_EQ(b.getHeight(), a.getHeight());
		EXPECT_FALSE(b.isColor());
	}
}

TEST(CImage, CtorRefOrGray)
{
	using namespace mrpt::img;
	{
		CImage a;
		bool load_ok = a.loadFromFile(tstImgFileColor);
		EXPECT_TRUE(load_ok);

		const CImage b(a, FAST_REF_OR_CONVERT_TO_GRAY);
		EXPECT_EQ(b.getWidth(), a.getWidth());
		EXPECT_EQ(b.getHeight(), a.getHeight());
		EXPECT_FALSE(b.isColor());
	}
	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;

		const CImage b(a, FAST_REF_OR_CONVERT_TO_GRAY);
		EXPECT_EQ(b.getWidth(), a.getWidth());
		EXPECT_EQ(b.getHeight(), a.getHeight());
		EXPECT_FALSE(b.isColor());
		EXPECT_EQ(b.at<uint8_t>(1, 2), 0x80);
	}
}

TEST(CImage, HalfAndDouble)
{
	using namespace mrpt::img;

	CImage a(32, 10, CH_GRAY);
	a.at<uint8_t>(0, 0) = 0x80;
	a.at<uint8_t>(0, 1) = 0x80;
	a.at<uint8_t>(1, 0) = 0x80;
	a.at<uint8_t>(1, 1) = 0x80;

	// Half:
	{
		const CImage imgH = a.scaleHalf(mrpt::img::IMG_INTERP_NN);
		EXPECT_EQ(imgH.getWidth(), a.getWidth() / 2);
		EXPECT_EQ(imgH.getHeight(), a.getHeight() / 2);
		EXPECT_EQ(imgH.isColor(), a.isColor());
		EXPECT_EQ(imgH.at<uint8_t>(0, 0), a.at<uint8_t>(0, 0));
	}
	// Double:
	{
		const CImage imgD = a.scaleDouble(mrpt::img::IMG_INTERP_NN);
		EXPECT_EQ(imgD.getWidth(), a.getWidth() * 2);
		EXPECT_EQ(imgD.getHeight(), a.getHeight() * 2);
		EXPECT_EQ(imgD.isColor(), a.isColor());
	}
}
TEST(CImage, getChannelsOrder)
{
	using namespace mrpt::img;
	{
		CImage a;
		bool load_ok = a.loadFromFile(tstImgFileColor);
		EXPECT_TRUE(load_ok);
		EXPECT_EQ(std::string("BGR"), a.getChannelsOrder());
	}
	{
		CImage a(32, 10, CH_GRAY);
		EXPECT_EQ(std::string("GRAY"), a.getChannelsOrder());
	}
}

TEST(CImage, ChangeCvMatCopies)
{
	using namespace mrpt::img;

	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// change shallow copy:
		cv::Mat m = a.asCvMat<cv::Mat>(SHALLOW_COPY);
		m.at<uint8_t>(2, 1) = 0x70;
		// Expect change in source:
		EXPECT_EQ(a.at<uint8_t>(1, 2), 0x70);

		// size:
		cv::Mat& m2 = a.asCvMatRef();
		cv::Mat& m3 = a.asCvMatRef();
		EXPECT_EQ(&m2, &m3);

		m2 = cv::Mat(40, 40, CV_8UC1);

		cv::Mat& m4 = a.asCvMatRef();
		EXPECT_EQ(&m2, &m4);

		EXPECT_EQ(a.getWidth(), 40U);
		EXPECT_EQ(a.getHeight(), 40U);
	}
	{
		CImage a(20, 10, CH_GRAY);
		a.at<uint8_t>(1, 2) = 0x80;
		// change deep copy:
		cv::Mat m = a.asCvMat<cv::Mat>(DEEP_COPY);
		m.at<uint8_t>(2, 1) = 0x70;
		// Expect NO change in source:
		EXPECT_EQ(a.at<uint8_t>(1, 2), 0x80);

		// size:
		m = cv::Mat(40, 40, CV_8UC1);
		EXPECT_EQ(a.getWidth(), 20U);
		EXPECT_EQ(a.getHeight(), 10U);
	}
}

TEST(CImage, ScaleImage)
{
	using namespace mrpt::img;
	CImage a;
	bool load_ok = a.loadFromFile(tstImgFileColor);
	EXPECT_TRUE(load_ok);

	{
		CImage b;
		a.scaleImage(b, 600, 400);
		EXPECT_EQ(b.getWidth(), 600U);
		EXPECT_EQ(b.getHeight(), 400U);
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}
	{
		CImage b;
		a.scaleHalf(b, IMG_INTERP_NN);
		EXPECT_EQ(b.getWidth(), 160U);
		EXPECT_EQ(b.getHeight(), 120U);
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}
	{
		CImage b;
		a.scaleHalf(b, IMG_INTERP_LINEAR);
		EXPECT_EQ(b.getWidth(), 160U);
		EXPECT_EQ(b.getHeight(), 120U);
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}
	{
		CImage b;
		a.scaleDouble(b, IMG_INTERP_NN);
		EXPECT_EQ(b.getWidth(), 640U);
		EXPECT_EQ(b.getHeight(), 480U);
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}
	{
		CImage b;
		a.scaleDouble(b, IMG_INTERP_LINEAR);
		EXPECT_EQ(b.getWidth(), 640U);
		EXPECT_EQ(b.getHeight(), 480U);
		EXPECT_EQ(a.getWidth(), 320U);
		EXPECT_EQ(a.getHeight(), 240U);
	}
}

TEST(CImage, Serialize)
{
	using namespace mrpt::img;
	CImage a;
	bool load_ok = a.loadFromFile(tstImgFileColor);
	EXPECT_TRUE(load_ok);

	mrpt::math::CMatrixFloat am;
	a.getAsMatrix(am, true, 0, 0, -1, -1, false /* dont normalize to [0,1] */);

	mrpt::io::CMemoryStream buf;
	auto arch = mrpt::serialization::archiveFrom(buf);
	arch << a;
	buf.Seek(0);
	CImage b;
	arch >> b;

	mrpt::math::CMatrixFloat bm;
	b.getAsMatrix(bm, true, 0, 0, -1, -1, false /* dont normalize to [0,1] */);

	EXPECT_EQ(am, bm);
}

TEST(CImage, KLT_response)
{
	using namespace mrpt::img;

	{
		CImage a(100, 90, CH_GRAY);
		a.filledRectangle(0, 0, 99, 99, TColor(0x10));
		a.filledRectangle(40, 30, 41, 31, TColor(0x20));
		a.saveToFile("a.png");

		for (int w = 2; w < 12; w++)
		{
			const auto resp = a.KLT_response(40, 30, w);
			EXPECT_GT(resp, 0.5f);
		}
	}
}

#endif  // MRPT_HAS_OPENCV
