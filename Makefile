default: instrument
	
instrumentor.so: instrumentor.cpp
	g++ -g -fPIC -shared instrumentor.cpp -o instrumentor.so -ldl

clean:
	rm instrumentor.so test_app

test_app: test_app.c
	gcc -g test_app.c -o test_app

instrument: instrumentor.so test_app
	LD_PRELOAD=$(PWD)/instrumentor.so ./test_app
