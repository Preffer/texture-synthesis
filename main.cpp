#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/tiff_io.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>

using namespace std;
using namespace boost;
using namespace gil;
namespace po = program_options;

typedef unsigned char uchar;
typedef std::array<uchar, 3> Pixel;
typedef std::array<uchar, 15> Feature;
typedef vector<Feature> FeaturePool;

const size_t pixelSize = sizeof(Pixel);
FeaturePool featurePool;

Pixel match(const Feature& f);
float distance(const Feature& f1, const Feature& f2);

int main(int argc, char* argv[]) {
	string inFileName, outFileName;
	int outWidth, outHeight;

	po::options_description desc("Options");
	desc.add_options()
		("in,i", po::value<string>(&inFileName)->required(), "Input Image")
		("out,o", po::value<string>(&outFileName)->required(), "Output Image")
		("width,w", po::value<int>(&outWidth)->required(), "Output Width")
		("height,h", po::value<int>(&outHeight)->required(), "Output Height")
		("help,H", "Display this help info");

	po::variables_map vm;
	try{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << endl;
			return EXIT_SUCCESS;
		}
		po::notify(vm);
	} catch(po::error& e) {
		cerr << "Error: " << e.what() << endl << endl;
		cout << desc << endl;
		return EXIT_FAILURE;
	}

	rgb8_image_t in;
	string inFileExt = inFileName.substr(inFileName.rfind('.') + 1);

	if (inFileExt == "png") {
		png_read_image(inFileName, in);
	} else {
		if (inFileExt == "jpg" || inFileExt == "jpeg") {
			jpeg_read_image(inFileName, in);
		} else {
			if (inFileExt == "tif" || inFileExt == "tiff") {
				tiff_read_image(inFileName, in);
			} else {
				cout << format("Unsupported format: %1%\nSupported formats: png, jpg, tif") % inFileExt << endl;
				return EXIT_FAILURE;
			}
		}
	}

	const rgb8c_view_t inView = const_view(in);
	const int inX = inView.width() - 1;
	const int inY = inView.height() - 1;

	cout << format("%1%: [%2% * %3%]") % inFileName % inView.width() % inView.height() << endl;

	for (int y = 0; y <= inY; y++) {
		for (int x = 0; x <= inX; x++) {
			int top = y ? y-1 : inY;
			int left = x ? x-1 : inX;
			int right = (x == inX) ? 0 : x + 1;

			Feature f;
			auto ptr = f.data();

			memcpy(ptr, &inView(left, top), pixelSize);
			memcpy(ptr += pixelSize, &inView(x, top), pixelSize);
			memcpy(ptr += pixelSize, &inView(right, top), pixelSize);
			memcpy(ptr += pixelSize, &inView(left, y), pixelSize);
			memcpy(ptr += pixelSize, &inView(x, y), pixelSize);

			featurePool.push_back(f);
		}
	}

	rgb8_image_t out(outWidth, outHeight);
	rgb8_view_t outView = view(out);
	resize_view(inView, outView, bilinear_sampler());

	const int outX = outView.width() - 1;
	const int outY = outView.height() - 1;

	for (int y = 0; y <= outY; y++) {
		cout << format("Processing %1% / %2%") % y % outY << endl;
		for (int x = 0; x <= outX; x++) {
			int top = y ? y-1 : outY;
			int left = x ? x-1 : outX;
			int right = (x == outX) ? 0 : x + 1;

			Feature f;
			auto ptr = f.data();

			memcpy(ptr, &outView(left, top), pixelSize);
			memcpy(ptr += pixelSize, &outView(x, top), pixelSize);
			memcpy(ptr += pixelSize, &outView(right, top), pixelSize);
			memcpy(ptr += pixelSize, &outView(left, y), pixelSize);

			Pixel p = match(f);
			memcpy(&outView(x, y), &p, pixelSize);
		}
	}

	string outFileExt = outFileName.substr(outFileName.rfind('.') + 1);
	if (outFileExt == "png") {
		png_write_view(outFileName, outView);
	} else {
		if (outFileExt == "jpg" || outFileExt == "jpeg") {
			jpeg_write_view(outFileName, outView);
		} else {
			if (outFileExt == "tif" || outFileExt == "tiff") {
				tiff_write_view(outFileName, outView);
			} else {
				cout << format("Unsupported format: %1%\nSupported formats: png, jpg, tif\nFallback to png") % outFileExt << endl;
				outFileName.replace(outFileName.rfind('.') + 1, outFileExt.length(), "png");
				png_write_view(outFileName, outView);
			}
		}
	}
	cout << format("%1%: [%2% * %3%]") % outFileName % outView.width() % outView.height() << endl;

	return EXIT_SUCCESS;
}

Pixel match(const Feature& f) {
	float minDistance = FLT_MAX;;
	Pixel minPixel;

	#pragma omp parallel for
	for (auto it = featurePool.begin(); it < featurePool.end(); it++) {
		const Feature& thisF = *it;
		float d = distance(thisF, f);
		if (d < minDistance) {
			minDistance = d;
			memcpy(minPixel.data(), &thisF[12], pixelSize);
		}
	}

	return minPixel;
}

float distance(const Feature& f1, const Feature& f2) {
	return(f1[0] - f2[0]) * (f1[0] - f2[0])
		+ (f1[1] - f2[1]) * (f1[1] - f2[1])
		+ (f1[2] - f2[2]) * (f1[2] - f2[2])
		+ (f1[3] - f2[3]) * (f1[3] - f2[3])
		+ (f1[4] - f2[4]) * (f1[4] - f2[4])
		+ (f1[5] - f2[5]) * (f1[5] - f2[5])
		+ (f1[6] - f2[6]) * (f1[6] - f2[6])
		+ (f1[7] - f2[7]) * (f1[7] - f2[7])
		+ (f1[8] - f2[8]) * (f1[8] - f2[8])
		+ (f1[9] - f2[9]) * (f1[9] - f2[9])
		+ (f1[10] - f2[10]) * (f1[10] - f2[10])
		+ (f1[11] - f2[11]) * (f1[11] - f2[11]);
}
