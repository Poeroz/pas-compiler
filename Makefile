default:
	$(MAKE) full

full: args.h main.cpp argparser.h argparser.cpp
	g++ argparser.cpp main.cpp -o pas-compiler -Wall -std=c++17 -O2

src_only:
	$(RM) pas-complier