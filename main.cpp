#include <iostream>
#include <boost/format.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

using namespace std;
using namespace boost;
using namespace gil;

int main(int argc, char* argv[]) {

	rgb8_image_t src;
	jpeg_read_image("test.jpg", src);

	rgb8_view_t srcView = view(src);

	for (int y = 0; y < srcView.height(); y++) {
		for (int x = 0; x < srcView.width()-1; x++) {
			rgb8_pixel_t srcPixel = srcView(x, y);
			cout << boost::format("(%1%, %2%): [%3%, %4%, %5%]") % x % y % (int)srcPixel[0] % (int)srcPixel[1] % (int)srcPixel[2] << endl;
		}
	}

	return EXIT_SUCCESS;
}
