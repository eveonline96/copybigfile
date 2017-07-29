.PHONY:clean all
exec:	
	gcc -o  cf  -lpthread copyfile.cpp  
clean:
	rm -f *.o
	rm -f *~
	rm *.a


