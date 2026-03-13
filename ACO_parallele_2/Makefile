include Make_osx.inc
#include Make_linux.inc
#include Make_msys2.inc

# ADICIONE O -fopenmp AQUI NESTA LINHA:
CXXFLAGS = -std=c++17 -fopenmp -I/opt/homebrew/include

ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
CXXFLAGS2 = $(CXXFLAGS)
else
CXXFLAGS2 = $(CXXFLAGS) -O2 -march=native -Wall 
CXXFLAGS += -O3 -march=native -Wall
endif

ALL= ant_simu.exe 

default:	help

all: $(ALL)

benchmark: ant_simu.exe
	@echo "Démarrage des tests de performance avec OpenMP..."
	@for t in 1 2 4 8 12 16; do \
		echo "========================================"; \
		echo "Exécution avec $$t thread(s)..."; \
		./ant_simu.exe $$t; \
	done
	@echo "========================================"
	@echo "Tests terminés ! Veuillez vérifier vos fichiers CSV."

clean:
	@rm -fr *.o *.exe *~

.cpp.o:
	$(CXX) $(CXXFLAGS2) -I/opt/homebrew/include -c $^ -o $@

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
