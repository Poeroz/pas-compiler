default:
	$(MAKE) full

full: main.cpp args.h args.cpp argparser.h argparser.cpp lexer.h lexer.cpp parser.h parser.cpp
	g++ args.cpp argparser.cpp lexer.cpp parser.cpp main.cpp -o pas-compiler -Wall -std=c++17 -O2

src_only:
	$(RM) pas-complier