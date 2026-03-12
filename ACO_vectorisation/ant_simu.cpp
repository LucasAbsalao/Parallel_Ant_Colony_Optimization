# include <vector>
# include <iostream>
# include <random>
# include <chrono>
# include <fstream>
# include "fractal_land.hpp"
# include "pheronome.hpp"
# include "renderer.hpp"
# include "window.hpp"
# include "rand_generator.hpp"
# include "colonie.hpp"

static const int iterations_until_print = 10;
static std::chrono::duration<double, std::milli>  ants_advance_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_evaporation_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_update_total_duration(0);


void advance_time( const fractal_land& land, pheronome& phen, 
                   const position_t& pos_nest, const position_t& pos_food,
                   Colonie& ants, std::size_t& cpteur,
                   std::size_t iterations_count, std::ofstream& csv_file)
{
    //-------------------------------------- AVANCEMENT DES FOURMIS --------------------------------------
    auto t1 = std::chrono::high_resolution_clock::now();
    for ( size_t i = 0; i < (size_t)ants.get_qtt_ants(); ++i )
        ants.advance(i, phen, land, pos_food, pos_nest, cpteur);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ants_advance_duration = t2-t1;    
    ants_advance_total_duration+=ants_advance_duration;



    //-------------------------------------- EVAPORATION DES PHEROMONES --------------------------------------
    t1 = std::chrono::high_resolution_clock::now();
    phen.do_evaporation();
    t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> pheromone_evaporation_duration = t2-t1;
    pheromone_evaporation_total_duration += pheromone_evaporation_duration;

    
    //-------------------------------------- ACTUALISATION DES PHEROMONES --------------------------------------
    t1 = std::chrono::high_resolution_clock::now();
    phen.update();
    t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> pheromone_update_duration = t2-t1;
    pheromone_update_total_duration += pheromone_update_duration;

    if (iterations_count%iterations_until_print==0){
        std::cout << "Iteration " << iterations_count << " sauve les metriques...\n";

        csv_file << iterations_count << "," 
                 << ants_advance_total_duration.count() / iterations_until_print << ","
                 << pheromone_evaporation_total_duration.count() / iterations_until_print << ","
                 << pheromone_update_total_duration.count() / iterations_until_print << "\n";
                 
        ants_advance_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_evaporation_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_update_total_duration = std::chrono::duration<double, std::milli>::zero();
    }
}

int main(int nargs, char* argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now(); // Le temp quand le programme a commencé a executer
    SDL_Init( SDL_INIT_VIDEO );

    //Les fichiers pour analyse des données de temps
    std::ofstream csv_iterations("iteration.csv");
    csv_iterations << "Iteration,Temps_Fourmis_ms,Temps_Evaporation_ms,Temps_Update_ms\n";
    std::ofstream csv_resume("resume.csv"); //std::ios::app);
    csv_resume << "Threads,Temps_Creation_Fourmis,Iteration_Nourriture,Temps_Computation,Temps_Total\n";

    std::size_t seed = 2026; // Graine pour la génération aléatoire ( reproductible )
    const int nb_ants = 5000; // Nombre de fourmis
    const double eps = 0.8;  // Coefficient d'exploration
    const double alpha=0.7; // Coefficient de chaos
    //const double beta=0.9999; // Coefficient d'évaporation
    const double beta=0.999; // Coefficient d'évaporation
    // Location du nid
    position_t pos_nest{256,256};
    // Location de la nourriture
    position_t pos_food{500,500};
    //const int i_food = 500, j_food = 500;    
    // Génération du territoire 512 x 512 ( 2*(2^8) par direction )
    fractal_land land(8,2,1.,1024);
    double max_val = 0.0;
    double min_val = 0.0;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j ) {
            max_val = std::max(max_val, land(i,j));
            min_val = std::min(min_val, land(i,j));
        }
    double delta = max_val - min_val;
    /* On redimensionne les valeurs de fractal_land de sorte que les valeurs
    soient comprises entre zéro et un */
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j )  {
            land(i,j) = (land(i,j)-min_val)/delta;
        }
    // Création de la colonie et définition du coefficient d'exploration de toutes les fourmis.
    Colonie colony(nb_ants, eps);


    //Créer les fourmis et mesurer le temps
    auto t1 = std::chrono::high_resolution_clock::now();
    colony.create_ants(land.dimensions(),seed);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ant_creation_duration = t2-t1;
    std::cout << "Time to create ants: " << ant_creation_duration.count() << "ms\n";

    // On crée toutes les fourmis dans la fourmilière.
    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    Window win("Ant Simulation", 2*land.dimensions()+10, land.dimensions()+266);
    Renderer renderer( land, phen, pos_nest, pos_food, colony );
    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    
    //Temps pur de simulation (sans affichage et initialisation)
    std::chrono::duration<double,std::milli> pure_computation_time(0);

    while (cont_loop) {
        ++it;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                cont_loop = false;
        }

        //La simulation pure (l'avance des fourmis et actualisation des pheromones)
        auto t_start_sim = std::chrono::high_resolution_clock::now();
        advance_time( land, phen, pos_nest, pos_food, colony, food_quantity, it, csv_iterations);
        auto t_end_sim = std::chrono::high_resolution_clock::now();
        pure_computation_time += (t_end_sim - t_start_sim);

        // Renderization et contrôle de la fênetre de visualisation
        renderer.display( win, food_quantity );
        win.blit();

        if ( not_food_in_nest && food_quantity > 0 ) {
            auto time_of_first_food = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration_until_first_food = time_of_first_food - start_time;

            std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
            std::cout << "Temps pur de calcul (sans affichage): " << pure_computation_time.count() << "ms\n";
            std::cout << "Temps total (avec affichage et init):" << duration_until_first_food.count() << "ms\n";

            int num_threads = 1;
            csv_resume << num_threads << "," 
                       << ant_creation_duration.count() << ","
                       << it << "," 
                       << pure_computation_time.count() << ","
                       << duration_until_first_food.count() << "\n";

            not_food_in_nest = false;
        }
        //SDL_Delay(10);
    }
    SDL_Quit();
    return 0;
}