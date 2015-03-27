#include <iostream>
#include <cfloat>
#include <cstring>
#include <boost/format.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>

using namespace std;
using namespace boost;
using namespace gil;

typedef unsigned char uchar;
typedef array<uchar, 3> Pixel;
typedef array<Pixel, 5> Feature;
typedef vector<Feature> FeaturePool;

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
FeaturePool featurePool;

Pixel match(const Feature& f);
float distance(const Feature& f1, const Feature& f2);

int main(int argc, char* argv[]) {
	rgb8_image_t in;
	jpeg_read_image("in.jpg", in);

	const rgb8c_view_t inView = const_view(in);
	const int inX = inView.width() - 1;
	const int inY = inView.height() - 1;

	for (int y = 0; y <= inY; y++) {
		for (int x = 0; x <= inX; x++) {
			int top = y ? y-1 : inY;
			int left = x ? x-1 : inX;
			int right = (x == inX) ? 0 : x + 1;

			Feature f;
			memcpy(f[0].data(), &inView(left, top), pixelSize);
			memcpy(f[1].data(), &inView(x, top), pixelSize);
			memcpy(f[2].data(), &inView(right, top), pixelSize);
			memcpy(f[3].data(), &inView(left, y), pixelSize);
			memcpy(f[4].data(), &inView(x, y), pixelSize);
			featurePool.push_back(f);
		}
	}

	cout << format("Read image complete, size: [%1% * %2%]") % inView.width() % inView.height() << endl;

	rgb8_image_t out(40, 30);
	rgb8_view_t outView = view(out);
	resize_view(inView, outView, bilinear_sampler());

	const int outX = outView.width() - 1;
	const int outY = outView.height() - 1;

	for (int y = 0; y <= outY; y++) {
		for (int x = 0; x <= outX; x++) {
			int top = y ? y-1 : outY;
			int left = x ? x-1 : outX;
			int right = (x == outX) ? 0 : x + 1;

			Feature f;
			memcpy(f[0].data(), &outView(left, top), pixelSize);
			memcpy(f[1].data(), &outView(x, top), pixelSize);
			memcpy(f[2].data(), &outView(right, top), pixelSize);
			memcpy(f[3].data(), &outView(left, y), pixelSize);

			Pixel p = match(f);
			memcpy(&outView(x, y), &p, pixelSize);
		}
	}

	jpeg_write_view("out.jpg", outView);
	cout << format("Write image complete, size: [%1% * %2%]") % outView.width() % outView.height() << endl;

	return EXIT_SUCCESS;
}

Pixel match(const Feature& f) {
	float minDistance = FLT_MAX;;
	Pixel minPixel;

	for (const Feature& thisF : featurePool) {
		float d = distance(thisF, f);
		if (d < minDistance) {
			minDistance = d;
			minPixel = thisF[4];
		}
	}

	return minPixel;
}

float distance(const Feature& f1, const Feature& f2) {
	return(f1[0][0] - f2[0][0]) * (f1[0][0] - f2[0][0])
		+ (f1[0][1] - f2[0][1]) * (f1[0][1] - f2[0][1])
		+ (f1[0][2] - f2[0][2]) * (f1[0][2] - f2[0][2])
		+ (f1[1][0] - f2[1][0]) * (f1[1][0] - f2[1][0])
		+ (f1[1][1] - f2[1][1]) * (f1[1][1] - f2[1][1])
		+ (f1[1][2] - f2[1][2]) * (f1[1][2] - f2[1][2])
		+ (f1[2][0] - f2[2][0]) * (f1[2][0] - f2[2][0])
		+ (f1[2][1] - f2[2][1]) * (f1[2][1] - f2[2][1])
		+ (f1[2][2] - f2[2][2]) * (f1[2][2] - f2[2][2])
		+ (f1[3][0] - f2[3][0]) * (f1[3][0] - f2[3][0])
		+ (f1[3][1] - f2[3][1]) * (f1[3][1] - f2[3][1])
		+ (f1[3][2] - f2[3][2]) * (f1[3][2] - f2[3][2]);
}
