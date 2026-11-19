Dom's hw3 files
===============
HW_utils.cpp, HW_spectrum.cpp, HW_swapPhase.cpp, HW_filter.cpp
    App functions (problems 6, 7). Copy them into hw3/ to build them into the app.
    HW_utils.cpp holds our own recursive FFT and separable 2-D FFT used by
    HW_spectrum and HW_swapPhase. Swap Phase and Filter match the sample app exactly.
cmdline/
    dft1D.cpp, fft1D.cpp (problems 3, 4).  Build: cd cmdline && make
    Usage: ./dft1D in dir out   (dir 0 = forward, 1 = inverse); fft1D needs N a power of 2.
    data/ holds the input lists (L.txt, prob5*.txt) and their transforms.
answers/
    prob1.txt, prob2.txt, prob3.txt, prob4.txt, prob5.txt, prob7.txt
    prob5.xlsx: magnitude and phase plots for problem 5 (one sheet per part).
