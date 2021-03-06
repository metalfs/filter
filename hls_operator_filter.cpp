#include <metal/stream.h>

//#define LIMIT(LO,VAL,HI) ( ((VAL)>(LO))? (((VAL)<(HI))?(VAL):(HI)) : (LO) )

#define VALUE_BYTES 8
#define VALUE_BITS (VALUE_BYTES*8)
#define VALUE_COUNT (STREAM_BYTES/VALUE_BYTES)
//#define VUP(IDX) LIMIT(0, (VALUE_BITS * (IDX) + VALUE_BITS - 1), (STREAM_BYTES*8))
//#define VLO(IDX) LIMIT(0, (VALUE_BITS * (IDX)), (STREAM_BYTES*8))
#define VUP(IDX) (VALUE_BITS * (IDX) + VALUE_BITS - 1)
#define VLO(IDX) (VALUE_BITS * (IDX))

typedef ap_uint<6> ValueCount; //Assume up to 64 Values (accomodating DoubleRow, this supports up to 32 * 64 = 2048bit streams)
typedef ap_uint<VALUE_BITS> Value;
typedef ap_uint<VALUE_BITS*VALUE_COUNT> Row;
typedef ap_uint<VALUE_BITS*VALUE_COUNT*2> DoubleRow;
typedef ap_uint<VALUE_COUNT> Mask;
typedef ap_uint<1> ap_bool;

typedef struct {
  Row data;
  Mask mask;
  ap_bool last;
} MaskedStreamElement;
typedef hls::stream<MaskedStreamElement> MaskedStream;

typedef struct {
  Row data;
  ValueCount count;
  ap_bool last;
} ShiftedStreamElement;
typedef hls::stream<ShiftedStreamElement> ShiftedStream;

// Sets keep bits (on VALUE_BYTES granularity) according to filter criteria
// Ignores keep on input stream
void filter_stream(mtl_stream &in, MaskedStream &out, Value lower_bound, Value upper_bound) {
  mtl_stream_element input;
  MaskedStreamElement output;

  do {
#pragma HLS PIPELINE
    input = in.read();

    output.data = input.data;
    for (int i = 0; i < VALUE_COUNT; ++i) {
#pragma HLS UNROLL
      Value value = input.data(VUP(i), VLO(i));
      if (lower_bound <= value && value <= upper_bound) {
        output.mask(i,i) = 1;
      } else {
        output.mask(i,i) = 0;
      }
    }
    output.last = input.last;

    out.write(output);
  } while (!output.last);
}

void print_row(Row row) {
	std::cout << "Row [ ";
    for (int i = 0; i < VALUE_COUNT; ++i) {
    	std::cout << (uint64_t)row(VUP(i), VLO(i)) << " ";
    }
    std::cout << "]" << std::endl;
}

// Assumes VALUE_COUNT > 1
// Squashes masked values and shifts kept values to the right
void shift_multi_stream(MaskedStream &in, ShiftedStream &out) {
  MaskedStreamElement input;
  ShiftedStreamElement output;

  do {
#pragma HLS PIPELINE
    input = in.read();

    Row shifter[VALUE_COUNT+1];
    shifter[0] = input.data;
    ValueCount count[VALUE_COUNT+1];
    count[0] = 0;
    for (int i = 0; i < VALUE_COUNT; ++i) {
#pragma HLS UNROLL
      std::cout << "Iteration " << i << " Mask " << (input.mask(i,i) ? "keep" : "omit") << " Count " << count[i] << std::endl;
      print_row(shifter[i]);
      if (input.mask(i,i) == 1) {
        count[i+1] = count[i] + 1;
        shifter[i+1] = shifter[i];
      } else {
        count[i+1] = count[i];
        if (count[i] > 0) {
          std::cout << "row(" << (count[i]-1) << ".." << 0 << ") <= row(" << (count[i]-1) << ".." << 0 << ")" << std::endl;
          shifter[i+1](VUP(count[i]-1), VLO(0)) = shifter[i](VUP(count[i]-1), VLO(0));
        }
        if (count[i] < (VALUE_COUNT-1)) {
          std::cout << "row(" << (VALUE_COUNT-2) << ".." << (count[i]) << ") <= row(" << (VALUE_COUNT-1) << ".." << (count[i]+1) << ")" << std::endl;
          shifter[i+1](VUP(VALUE_COUNT-2), VLO(count[i])) = shifter[i](VUP(VALUE_COUNT-1), VLO(count[i]+1));
        }
        std::cout << "row(" << (VALUE_COUNT-1) << ".." << (VALUE_COUNT-1) << ") <= 0" << std::endl;
        shifter[i+1](VUP(VALUE_COUNT-1), VLO(VALUE_COUNT-1)) = 0;
        print_row(shifter[i+1]);
      }
      std::cout << std::endl;
    }
    output.data = shifter[VALUE_COUNT];
    output.count = count[VALUE_COUNT];
    output.last = input.last;

    out.write(output);
  } while (!output.last);
}

// Assumes VALUE_COUNT > 1
// Combines adjacent shifted data blocks to a contiguous data stream
// Only the last output element may have a 0 bit in keep
void compact_multi_stream(ShiftedStream &in, mtl_stream &out) {
  ShiftedStreamElement input;
  mtl_stream_element output;
  ap_bool is_done = 0;

  DoubleRow  buffer = 0;
  ValueCount counter = 0;
  do {
#pragma HLS PIPELINE
    input = in.read();

    buffer(VUP(counter+input.count-1), VLO(counter)) = input.data(VUP(input.count-1), VLO(0));
    counter += input.count;

    if (counter >= VALUE_COUNT) {
      is_done = input.last && (counter == VALUE_COUNT);

      output.data = buffer(VUP(VALUE_COUNT-1), VLO(0));
      output.keep = (mtl_stream_keep)-1;
      output.last = is_done;
      out.write(output);

      buffer(VUP(VALUE_COUNT-1), VLO(0)) = buffer(VUP(2*VALUE_COUNT-1), VLO(VALUE_COUNT));
      counter -= VALUE_COUNT;
    }
  } while (!input.last);

  if (!is_done) {
    output.data = buffer(VUP(VALUE_COUNT-1), VLO(0));
    output.keep = (mtl_stream_keep)((1 << (8*counter)) - 1);
    output.last = 1;
    out.write(output);
  }
}

// Assumes VALUE_COUNT == 1
// Only forwards unmasked stream elements as well as the last element
// Only the last output element may have a 0 bit in keep
void compact_single_stream(MaskedStream &in, mtl_stream &out) {
  MaskedStreamElement input;
  mtl_stream_element output;

  do {
#pragma HLS PIPELINE
    input = in.read();

    output.last = input.last;
    if (input.mask != 0) {
      output.data = input.data;
      output.keep = (mtl_stream_keep)-1;
      out.write(output);
    } else if (input.last) {
      output.data = 0;
      output.keep = (mtl_stream_keep)0;
      out.write(output);
    }
  } while (!output.last);
}

void fix_empty_stream(mtl_stream &in, mtl_stream &out) {
  mtl_stream_element input;
  mtl_stream_element buffer;
  buffer.data = 0;
  buffer.keep = 1; // in case of empty streams, that ensures that at least one 0 byte is sent to output
  ap_bool has_buffer = 0;

  do {
#pragma HLS PIPELINE
    input = in.read();

    if (input.keep != 0) {
      if (has_buffer) {
        out.write(buffer);
      }
      buffer = input;
      has_buffer = 1;
    }

    if (input.last && input.keep == 0) {
      buffer.last = 1;
      has_buffer = 1;
    }
  } while (!input.last);

  if (has_buffer) {
    out.write(buffer);
  }
}

void hls_operator_filter(mtl_stream &in, mtl_stream &out, Value lower_bound, Value upper_bound) {
#pragma HLS INTERFACE axis port=in name=axis_input
#pragma HLS INTERFACE axis port=out name=axis_output
#pragma HLS INTERFACE s_axilite port=lower_bound bundle=control offset=0x100
#pragma HLS INTERFACE s_axilite port=upper_bound bundle=control offset=0x110
#pragma HLS INTERFACE s_axilite port=return bundle=control

  MaskedStream masked;
  ShiftedStream shifted;
  mtl_stream compact;

#pragma HLS DATAFLOW

  filter_stream(in, masked, lower_bound, upper_bound);
#if VALUE_COUNT > 1
  shift_multi_stream(masked, shifted);
  compact_multi_stream(shifted, compact);
#else
  compact_single_stream(masked, compact);
#endif
  fix_empty_stream(compact, out);
}

#include "testbench.cpp"
