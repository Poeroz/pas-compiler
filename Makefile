default:
	$(MAKE) full

full: main.cpp args.h args.cpp argparser.h argparser.cpp lexer.h lexer.cpp lalrparser.h lalrparser.cpp
	g++ args.cpp argparser.cpp lexer.cpp lalrparser.cpp main.cpp -o pas-compiler -Wall -std=c++17 -O2

src_only:
	$(RM) pas-complier