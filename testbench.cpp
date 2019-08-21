#ifndef __SYNTHESIS__

int main() {
  mtl_stream in;
  mtl_stream out;

  FILE * infile = fopen("../../../../testl.bin", "r");
  size_t readBytes;
  mtl_stream_element element;
  do {
    readBytes = fread(&(element.data), 1, 64, infile);
    element.keep = (mtl_stream_keep)-1;
    element.last = (readBytes != 8);
    in.write(element);
  } while (!element.last);

  hls_operator_filter(in, out, 2, 3);

  FILE * outfile = fopen("../../../../testl.filtered.bin", "w");
  do {
    element = out.read();
    fwrite(&(element.data), 1, 64, outfile);
  } while (!element.last);

  return 0;
}

#endif
