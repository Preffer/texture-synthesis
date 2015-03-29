#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/tiff_io.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

using namespace std;
using namespace boost;
using namespace gil;

namespace po = program_options;
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

const int boxSize = 5;
const int boxBorder = boxSize / 2;
const int pixelCount = (boxSize + 1) * (boxSize - 1) / 2;
const int pixelSize = sizeof(rgb8_pixel_t);

typedef unsigned char uchar;
typedef std::array<uchar, pixelSize> Pixel;
typedef bg::model::point<uchar, pixelCount, bg::cs::cartesian> Feature;
typedef pair<Feature, Pixel> NodeType;


int overflow(int value, int max);

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

	map<string, int> formatMap;
	formatMap["png"] = 1;
	formatMap["jpg"] = 2;
	formatMap["jpeg"] = 2;
	formatMap["tif"] = 3;
	formatMap["tiff"] = 3;

	rgb8_image_t in;
	string inFileExt = inFileName.substr(inFileName.rfind('.') + 1);

	switch (formatMap[inFileExt]) {
		case 1:
			png_read_image(inFileName, in);
			break;
		case 2:
			jpeg_read_image(inFileName, in);
			break;
		case 3:
			tiff_read_image(inFileName, in);
			break;
		default:
			cout << format("Unsupported format: %1%\nSupported formats: png, jpg, tif") % inFileExt << endl;
			return EXIT_FAILURE;
	}

	const rgb8c_view_t inView = const_view(in);
	cout << format("%1%: [%2% * %3%]") % inFileName % inView.width() % inView.height() << endl;

	bgi::rtree< NodeType, bgi::linear<16> > featureTree;

	int inX = inView.width() - 1;
	int inY = inView.height() - 1;

	for (int y = 0; y <= inY; y++) {
		map<int, int> ypos;
		for (int i = -boxBorder; i <= 0; i++) {
			ypos[i] = overflow(y + i, inY);
		}

		for (int x = 0; x <= inX; x++) {
			map<int, int> xpos;
			for (int i = -boxBorder; i <= boxBorder; i++) {
				xpos[i] = overflow(x + i, inX);
			}

			Feature f;
			rgb8_pixel_t* ptr = (rgb8_pixel_t*) &f;

			for (int row = -boxBorder; row < 0; row++) {
				for (int col = -boxBorder; col <= boxBorder; col++, ptr++) {
					memcpy(ptr, &inView(xpos[col], ypos[row]), pixelSize);
				}
			}

			for (int col = -boxBorder; col < 0; col++, ptr++) {
				memcpy(ptr, &inView(xpos[col], y), pixelSize);
			}

			Pixel p;
			memcpy(p.data(), &inView(x, y), pixelSize);

			featureTree.insert(NodeType(f, p));
		}
	}

	rgb8_image_t out(outWidth, outHeight);
	rgb8_view_t outView = view(out);
	resize_view(inView, outView, bilinear_sampler());

	int outX = outView.width() - 1;
	int outY = outView.height() - 1;

	for (int y = 0; y <= outY; y++) {
		cout << format("Processing %1% / %2%") % y % outY << endl;
		map<int, int> ypos;
		for (int i = -boxBorder; i <= 0; i++) {
			ypos[i] = overflow(y + i, outY);
		}
		for (int x = 0; x <= outX; x++) {
			map<int, int> xpos;
			for (int i = -boxBorder; i <= boxBorder; i++) {
				xpos[i] = overflow(x + i, outX);
			}

			Feature f;
			rgb8_pixel_t* ptr = (rgb8_pixel_t*) &f;

			for (int row = -boxBorder; row < 0; row++) {
				for (int col = -boxBorder; col <= boxBorder; col++, ptr++) {
					memcpy(ptr, &outView(xpos[col], ypos[row]), pixelSize);
				}
			}

			for (int col = -boxBorder; col < 0; col++, ptr++) {
				memcpy(ptr, &outView(xpos[col], y), pixelSize);
			}

			vector<NodeType> found;
			featureTree.query(bgi::nearest(f, 1), std::back_inserter(found));

			memcpy(&outView(x, y), found.front().second.data(), pixelSize);
		}
	}

	string outFileExt = outFileName.substr(outFileName.rfind('.') + 1);

	switch (formatMap[outFileExt]) {
		case 1:
			png_write_view(outFileName, outView);
			break;
		case 2:
			jpeg_write_view(outFileName, outView);
			break;
		case 3:
			tiff_write_view(outFileName, outView);
			break;
		default:
			cout << format("Unsupported format: %1%\nSupported formats: png, jpg, tif\nFallback to png") % outFileExt << endl;
			outFileName.replace(outFileName.rfind('.') + 1, outFileExt.length(), "png");
			png_write_view(outFileName, outView);
	}

	cout << format("%1%: [%2% * %3%]") % outFileName % outView.width() % outView.height() << endl;

	return EXIT_SUCCESS;
}

int overflow(int value, int max) {
	if (value < 0) {
		return value + max;
	}
	if (value > max) {
		return value - max;
	}
	return value;
}
