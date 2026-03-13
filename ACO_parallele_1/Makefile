include Make_linux.inc
#include Make_msys2.inc
#include Make_osx.inc

CXXFLAGS = -std=c++17
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
CXXFLAGS2 = CXXFLAGS
else
CXXFLAGS2 = ${CXXFLAGS} -O2 -march=native -Wall 
CXXFLAGS += -O3 -march=native -Wall
endif

ALL= ant_simu.exe 

default:	help

all: $(ALL)

benchmark: ant_simu.exe
	@echo "Démarrage des tests de performance avec MPI..."
	@for p in 1 2 4 8 12; do \
		echo "========================================"; \
		echo "Exécution avec $$p processus MPI..."; \
		mkdir -p test_$${p}_cores; \
		export OMP_NUM_THREADS=1; \
		mpirun --oversubscribe -np $$p ./ant_simu.exe; \
	done
	@echo "========================================"
	@echo "Tests terminés ! Veuillez vérifier vos fichiers CSV."

clean:
	@rm -fr *.o *.exe *~

.cpp.o:
	$(MPICXX) $(CXXFLAGS2) -c $^ -o $@	

ant_simu.exe : colonie.o fractal_land.o renderer.o window.o ant_simu.o
	$(MPICXX) $(CXXFLAGS2) $^ -o $@ $(LIB)	

help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(MPICXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css --metadata pagetitle="OS202 - TD1" -o $@
