sampleobjects = buffer_manager.o file_manager.o sample_run.o
.PHONY: linearsearch binarysearch deletion join1 join2

sample:
	$(MAKE) sample_run.o
	$(MAKE) buffer_manager.o
	$(MAKE) file_manager.o
	$(MAKE) sample_run
	./sample_run

linearsearch: linear_search.cpp buffer_manager.cpp file_manager.cpp
	$(MAKE) remove_testing
	g++ linear_search.cpp buffer_manager.cpp file_manager.cpp -o linearsearch -std=c++11

binarysearch: binary_search.cpp buffer_manager.cpp file_manager.cpp
	$(MAKE) remove_testing
	g++ binary_search.cpp buffer_manager.cpp file_manager.cpp -o binarysearch -std=c++11

deletion: deletion.cpp buffer_manager.cpp file_manager.cpp
	$(MAKE) remove_testing
	g++ deletion.cpp buffer_manager.cpp file_manager.cpp -o deletion -std=c++11

join1: join1.cpp buffer_manager.cpp file_manager.cpp
	$(MAKE) remove_testing
	g++ join1.cpp buffer_manager.cpp file_manager.cpp -o join1 -std=c++11

join2: join2.cpp buffer_manager.cpp file_manager.cpp
	$(MAKE) remove_testing
	g++ join2.cpp buffer_manager.cpp file_manager.cpp -o join2 -std=c++11

sample_run: $(sampleobjects)
	g++ -std=c++11 -o sample_run $(sampleobjects)

sample_run.o: sample_run.cpp
	g++ -std=c++11 -c sample_run.cpp

buffer_manager.o: buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o: file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

remove_testing:
	rm -rf testing

test_run:
	$(MAKE) remove_testing
	$(MAKE) linearsearch
	$(MAKE) binarysearch
	$(MAKE) deletion
	$(MAKE) join1
	$(MAKE) join2
	./linearsearch TestCases/TC_search/sorted_input TestCases/TC_search/query_search.txt testing
	$(MAKE) remove_testing
	./binarysearch TestCases/TC_search/sorted_input TestCases/TC_search/query_search.txt testing
	./deletion TestCases/TC_delete/sorted_input TestCases/TC_delete/query_delete.txt
	$(MAKE) remove_testing
	./join1 TestCases/TC_join1/input1_join1 TestCases/TC_join1/input2_join1 testing
	$(MAKE) remove_testing
	./join2 TestCases/TC_join2/input1_join2 TestCases/TC_join2/input2_join2_updated testing
	$(MAKE) remove_testing
	$(MAKE) clean

clean:
	rm -f *.o
	rm -f sample_run
	rm -rf linearsearch
	rm -rf binarysearch
	rm -rf deletion
	rm -rf join1
	rm -rf join2
