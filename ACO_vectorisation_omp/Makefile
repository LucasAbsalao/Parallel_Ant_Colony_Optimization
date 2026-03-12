include Make_linux.inc
#include Make_msys2.inc
#include Make_osx.inc

CXXFLAGS = -std=c++17
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
CXXFLAGS2 = CXXFLAGS
else
CXXFLAGS2 = ${CXXFLAGS} -O2 -fopenmp -march=native -Wall 
CXXFLAGS += -O3 -fopenmp -march=native -Wall
endif

ALL= ant_simu.exe 

default:	help

all: $(ALL)

benchmark: ant_simu.exe
	@echo "Démarrage des tests de performance avec OpenMP..."
	@for t in 1 2 4 8 12 16; do \
		echo "========================================"; \
		echo "Exécution avec $$t thread(s)..."; \
		export OMP_NUM_THREADS=$$t; \
		./ant_simu.exe; \
	done
	@echo "========================================"
	@echo "Tests terminés ! Veuillez vérifier vos fichiers CSV."

clean:
	@rm -fr *.o *.exe *~

.cpp.o:
	$(CXX) $(CXXFLAGS2) -c $^ -o $@	

ant_simu.exe : colonie.o fractal_land.o renderer.o window.o ant_simu.o
	$(CXX) $(CXXFLAGS2) $^ -o $@ $(LIB)	

help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css --metadata pagetitle="OS202 - TD1" -o $@
