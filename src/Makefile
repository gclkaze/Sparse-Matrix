allWarnings:
	g++ main.cpp -o a.exe -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow  -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused
all:
	g++ main.cpp -o a.exe 
compile:
	g++ -O2 main.cpp -o a.exe -pedantic  -Wswitch-default -Wundef -Werror -Wno-unused
opt:
	g++ -O2 main.cpp -o a.exe
clean:
	rm *.gch
	rm nodes/*.gch
