#ifndef __SYNTHESIS__

#define TEST_ROWS 64

int main() {
  mtl_stream in;
  mtl_stream out;


  mtl_stream_element element;
  for (int i = 0; i < TEST_ROWS; ++i) {
	element.data = 0;
    for (int j = 0; j < VALUE_COUNT; ++j) {
    	element.data(VUP(j), VLO(j)) = (13*i) % 16;
    }
    element.keep = (mtl_stream_keep)-1;
    element.last = (i == TEST_ROWS-1);
    in.write(element);
  }

  hls_operator_filter(in, out, 2, 3);

  do {
    element = out.read();
    if (element.last) {
    	std::cout << "LAST ";
    } else {
    	std::cout << "     ";
    }
    std::cout << std::hex << element.data << " | " << element.keep << std::endl;
  } while (!element.last);

  return 0;
}

#endif
