# Copied from https://stackoverflow.com/questions/19928965/mingw-makefile-with-or-without-msys-del-vs-rm
# for windows/linux cross compatibility
ifeq ($(OS),Windows_NT) 
RM = del /Q /F
CP = copy /Y
ifdef ComSpec
SHELL := $(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
else
RM = rm -rf
CP = cp -f
endif
# End copied code

OPTS = -Wall -Wextra -g -lm -std=c11

.PHONY:
	3sq% 3irr%

3sq%: voronoi2
ifdef gui
	./voronoi2 3 data/dataset_$*.csv data/polygon_square.txt output.txt | /mnt/c/Windows/py.exe visualisation.py $@
else
	./voronoi2 3 data/dataset_$*.csv data/polygon_square.txt output.txt
endif

3ir%: voronoi2
ifdef gui
	./voronoi2 3 data/dataset_$*.csv data/polygon_irregular.txt output.txt | /mnt/c/Windows/py.exe visualisation.py $@
else
	./voronoi2 3 data/dataset_$*.csv data/polygon_irregular.txt output.txt	
endif

4sq%: voronoi2
ifdef gui
	./voronoi2 4 data/dataset_$*.csv data/polygon_square.txt output.txt | /mnt/c/Windows/py.exe visualisation.py $@
else
	./voronoi2 4 data/dataset_$*.csv data/polygon_square.txt output.txt
endif

4ir%: voronoi2
ifdef gui
	./voronoi2 4 data/dataset_$*.csv data/polygon_irregular.txt output.txt | /mnt/c/Windows/py.exe visualisation.py $@
else
	./voronoi2 4 data/dataset_$*.csv data/polygon_irregular.txt output.txt	
endif

voronoi2: main.o newshape.o stage.o utils.o
	gcc $(OPTS) -o voronoi2 $^

%.o: %.c %.h
	gcc $(OPTS) -c -o $@ $<

clean:
	-$(RM) voronoi2.exe
	-$(RM) voronoi2
	-$(RM) *.o
