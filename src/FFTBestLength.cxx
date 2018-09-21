#include "WireCellUtil/FFTBestLength.h"

#include <boost/assign/std/vector.hpp>
#include <iostream>

using namespace boost::assign;
using namespace WireCell;

// flag = 0, does not care if window_length is odd or even
// flag = 1, if the window_length is odd, the returned value will be odd
//           if the window_length is even, the returned value will be even

int WireCell::cal_fft_best_length(int window_length, int flag){
  if (window_length <=0) {
    std::cerr << "FFT window length is smaller than zero! " << std::endl;
    return -1;
  }

  std::vector<int> edges;
  if (flag == 0){
    edges += 33, 77, 128, 130, 200, 256, 280, 320, 400, 448, 512, 560, 605, 686, 768, 800, 847, 882, 960, 980, 1280, 1331, 1372, 1536, 1600, 1792, 2048, 2560, 2662, 2744, 3200, 4096, 4116, 4480, 4802, 5120, 5324, 5544, 5600, 5632, 6174, 6272, 6468, 7168, 7840, 8192, 8232, 8960, 9317, 10240, 10648, 11979, 14336, 15972, 16000, 16384; 
  }else{
    if (window_length%2==0){
      // even
      edges += 40, 72, 126, 130, 200, 256, 280, 320, 400, 448, 512, 560, 616, 686, 768, 800, 896, 1024, 1120, 1280, 1372, 1536, 1600, 1792, 2048, 2560, 2662, 2744, 3200, 4096, 4116, 4480, 4802, 5120, 5324, 5544, 5600, 5632, 6174, 6272, 6468, 7168, 7840, 8192, 8232, 8960, 10240, 10648, 11200, 11858, 11880, 14336, 15972, 16000, 16384;
    }else{
      // odd
      edges += 33, 77, 85, 119, 147, 187, 209, 245, 247, 297, 315, 375, 405, 455, 525, 567, 637, 693, 715, 729, 819, 891, 945, 1089, 1331, 1573, 1859, 2197, 2299, 2431, 2541, 2625, 2695, 3025, 3267, 3993, 4095, 4125, 4719, 4725, 4851, 5145, 5445, 5929, 6655, 7007, 7623, 7865, 9317, 9801, 11979, 14157, 14175, 15379, 16335;
    }
  }

  // std::cout << flag << " " << window_length << " " << edges.back() << std::endl;
  
  if (window_length <= edges.back()){
    for (auto it = edges.begin(); it!=edges.end(); it++){
      if ((*it) >= window_length)
	return *it;
    }
  }
  
  std::cerr << "FFT window length is not good! " << std::endl; 
  return -1;
}
