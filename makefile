cc=g++
prom=spider
source=main.cpp

$(prom):$(source)
	$(cc) $(source) -lcurl -lpthread --std=c++11 -o $(prom)
	mkdir Explicit
	mkdir Questionable
	mkdir Safe

clean:
	rm -rf Explicit Questionable Safe spider
