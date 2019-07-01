main: main.o libAdd.a libMul.so libMod.so
	g++ main.o -L. -lAdd -L. -lMul -Wl,-rpath,. -ldl -o $@

libAdd.a: add.o
	ar rcs $@ $<

libMul.so: mul.o
	g++ -std=c++14 -shared -o $@ $<

libMod.so: mod.o
	g++ -std=c++14 -shared -o $@ $<

main.o: main.cpp
	g++ -std=c++14 -c $< -o $@

add.o: add.cpp add.h 
	g++ -std=c++14 -c $< -o $@
	
%.o: %.cpp %.h 
	g++ -std=c++14 -fPIC -c $< -o $@ 

clean:
	rm -rf main *.a *.o *.so
