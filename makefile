all: meineAnalyse

.outputFolder:
	mkdir -p out

meineAnalyse: .outputFolder
	clang -std=c++17 -lstdc++ -lm -I include/ meineAnalyse.cpp -Wall -Wextra -Werror -o out/meineAnalyse
