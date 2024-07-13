SOURCE := server.cpp
SOURCE2 := client.cpp
CXXFLAGS := -std=c++20
EXE := server
EXE2:= client

.PHONY := game clean

game:
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(EXE)
	$(CXX) $(CXXFLAGS) $(SOURCE2) -o $(EXE2)

clean:
	rm -f $(EXE)
	rm -f $(EXE2)
