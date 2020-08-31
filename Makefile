default: test_instrumentor test_app test_instrumentor_debug instrumentor.so
	
instrumentor.so: instrumentor.cpp
	g++ -g -fPIC -shared instrumentor.cpp -o instrumentor.so -ldl

clean:
	rm instrumentor.so ./test_app ./test_instrumentor

test_app: test_app.c
	gcc -g test_app.c -o test_app -lpthread

test_instrumentor: test_instrumentor.cpp instrumentor.cpp
	g++ -g test_instrumentor.cpp instrumentor.cpp -o test_instrumentor -ldl

test_instrumentor_debug: test_instrumentor.cpp instrumentor.cpp
	g++ -g test_instrumentor.cpp instrumentor.cpp -DDEBUG -o test_instrumentor_debug -ldl

instrument_app: instrumentor.so test_app
	LD_PRELOAD=$(PWD)/instrumentor.so ./test_app

instrument_test: test_instrumentor
	./test_instrumentor
