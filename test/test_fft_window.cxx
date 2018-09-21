#include "WireCellUtil/FFTBestLength.h"

#include <iostream>

using namespace WireCell;

int main(int argc, char** argv){
  std::cout << cal_fft_best_length(9592) << " " << cal_fft_best_length(9892,1) << " " << cal_fft_best_length(9595) << " " << cal_fft_best_length(9595,1) << std::endl;
  return 0;
}
