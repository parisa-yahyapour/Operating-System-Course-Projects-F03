VoiceFilters.out: main.o utils.o filters.o
	g++ --std=c++11 main.o utils.o filters.o -o VoiceFilters.out -lsndfile -lpthread
main.o: utils.hpp main.cpp statics.hpp
	g++ -c --std=c++11 main.cpp
utils.o: utils.cpp utils.hpp
	g++ -c --std=c++11 utils.cpp
filters.o: filters.hpp filters.cpp statics.hpp
	g++ -c --std=c++11 filters.cpp
clean:
	rm *.o VoiceFilters.out