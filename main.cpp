#include <iostream>
#include <cstring>
#include <boost/format.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

using namespace std;
using namespace boost;
using namespace gil;

typedef unsigned char uchar;
typedef array<uchar, 3> Pixel;
typedef array<Pixel, 4> Feature;
typedef vector<Feature> FeaturePool;

ostream& operator<< (ostream& out, const rgb8_pixel_t& p) {
	out << "[" << (int)p[0] << ", " << (int)p[1] << ", " << (int)p[2] << "]";
	return out;
}

ostream& operator<< (ostream& out, const Pixel& p) {
	out << "[" << (int)p[0] << ", " << (int)p[1] << ", " << (int)p[2] << "]";
	return out;
}

ostream& operator<< (ostream& out, const Feature& f) {
	out << "<" << f[0] << ", " << f[1] << ", " << f[2] << ", " << f[3] << ">";
	return out;
}

ostream& operator<< (ostream& out, const FeaturePool& p) {
	for (const Feature& f : p) {
		out << f << endl;
	}
	return out;
}

const size_t pixelSize = sizeof(Pixel);

int main(int argc, char* argv[]) {
	rgb8_image_t src;
	jpeg_read_image("test.jpg", src);

	rgb8_view_t srcView = view(src);
	FeaturePool featurePool;

	for (int y = 0; y < srcView.height(); y++) {
		for (int x = 0; x < srcView.width(); x++) {
			Feature f;
			memcpy(f[0].data(), &srcView(x, y), pixelSize);
			memcpy(f[1].data(), &srcView(x, y), pixelSize);
			memcpy(f[2].data(), &srcView(x, y), pixelSize);
			memcpy(f[3].data(), &srcView(x, y), pixelSize);
			featurePool.push_back(f);
		}
	}

	cout << featurePool << endl;
	return EXIT_SUCCESS;
}
