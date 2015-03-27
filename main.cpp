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
typedef array<Pixel, 5> Feature;
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
	out << "<" << f[0] << ", " << f[1] << ", " << f[2] << ", " << f[3] << ", " << f[4] << ">";
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

	const rgb8c_view_t srcView = view(src);
	const int srcX = srcView.width() - 1;
	const int srcY = srcView.height() - 1;

	FeaturePool featurePool;
	for (int y = 0; y <= srcY; y++) {
		for (int x = 0; x <= srcX; x++) {
			int top = y ? y-1 : srcY;
			int left = x ? x-1 : srcX;
			int right = (x == srcX) ? 0 : x + 1;

			Feature f;
			memcpy(f[0].data(), &srcView(left, top), pixelSize);
			memcpy(f[1].data(), &srcView(x, top), pixelSize);
			memcpy(f[2].data(), &srcView(right, top), pixelSize);
			memcpy(f[3].data(), &srcView(left, y), pixelSize);
			memcpy(f[4].data(), &srcView(x, y), pixelSize);
			featurePool.push_back(f);
		}
	}

	cout << featurePool.size() << endl;

	return EXIT_SUCCESS;
}
