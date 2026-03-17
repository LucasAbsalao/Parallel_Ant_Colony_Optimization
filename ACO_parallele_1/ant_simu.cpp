# include <vector>
# include <iostream>
# include <random>
# include <memory>
# include <chrono>
# include <mpi.h>
# include <fstream>
# include "fractal_land.hpp"
# include "pheronome.hpp"
# include "renderer.hpp"
# include "window.hpp"
# include "rand_generator.hpp"
# include "colonie.hpp"

static const int iterations_until_print = 10;
size_t global_food_quantity = 0;
static std::chrono::duration<double, std::milli>  ants_advance_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_communicator_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_evaporation_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_evaporation_communicator_total_duration(0);
static std::chrono::duration<double, std::milli>  pheromone_update_total_duration(0);


void advance_time( const fractal_land& land, pheronome& phen, 
                   const position_t& pos_nest, const position_t& pos_food,
                   Colonie& ants, std::size_t& cpteur,
                   std::size_t iterations_count, MPI_Comm glob_com, int nbp, std::ofstream& csv_file)
{
    //-------------------------------------- AVANCEMENT DES FOURMIS --------------------------------------
    auto t1 = std::chrono::high_resolution_clock::now();

    // Chaque processus fait avancer uniquement sa propre portion de fourmis.
    // Les phéromones déposées sont d'abord stockées dans la mémoire locale.
    ants.advance_all(phen, land, pos_food, pos_nest, cpteur);

    auto t2 = std::chrono::high_resolution_clock::now();

    // Synchronisation globale de la carte des phéromones :
    // - MPI_IN_PLACE : Économise la RAM (pas besoin d'allouer un buffer d'envoi séparé).
    // - MPI_MAX : Fusionne les cartes locales en conservant la trace de phéromone 
    //             la plus forte déposée sur chaque cellule à travers tous les processus.
    MPI_Allreduce(
        MPI_IN_PLACE,  
        (double*) phen.m_buffer_pheronome.data(),  
        phen.m_buffer_pheronome.size()*2,  
        MPI_DOUBLE,         
        MPI_MAX,            
        glob_com              
    );
    auto t3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ants_advance_duration = t2-t1;
    std::chrono::duration<double, std::milli> pheromone_communicator_duration = t3-t2;
    ants_advance_total_duration+=ants_advance_duration;
    pheromone_communicator_total_duration += pheromone_communicator_duration;



    //-------------------------------------- EVAPORATION DES PHEROMONES --------------------------------------
    t1 = std::chrono::high_resolution_clock::now();
    phen.parallel_do_evaporation();
    t2 = std::chrono::high_resolution_clock::now(); 
    int count_per_process = (phen.get_m_dim() / nbp) * phen.get_m_stride() * 2;

    // Synchronisation avec MPI_IN_PLACE :
    // Pour éviter de gaspiller de la mémoire (RAM) avec un buffer d'envoi temporaire,
    // nous utilisons MPI_IN_PLACE. Le grand vecteur 'm_buffer_pheronome' agit 
    // simultanément comme buffer d'envoi (source) et buffer de réception (destination).
    MPI_Allgather(
        MPI_IN_PLACE,              
        0,                          
        MPI_DATATYPE_NULL,         
        (double*) phen.m_buffer_pheronome.data() + (phen.get_m_stride() * 2),
        count_per_process,      
        MPI_DOUBLE,
        glob_com
    );
    t3 = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double, std::milli> pheromone_evaporation_duration = t2-t1;
    std::chrono::duration<double, std::milli> pheromone_evaporation_communication_duration = t3-t2;
    pheromone_evaporation_total_duration += pheromone_evaporation_duration;
    pheromone_evaporation_communicator_total_duration += pheromone_evaporation_communication_duration;

    
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
                 << pheromone_communicator_total_duration.count()/iterations_until_print << ","
                 << pheromone_evaporation_total_duration.count() / iterations_until_print << ","
                 << pheromone_evaporation_communicator_total_duration.count()/iterations_until_print << ","
                 << pheromone_update_total_duration.count() / iterations_until_print;
                 
        ants_advance_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_communicator_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_evaporation_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_evaporation_communicator_total_duration = std::chrono::duration<double, std::milli>::zero();
        pheromone_update_total_duration = std::chrono::duration<double, std::milli>::zero();
    }
}

int main(int nargs, char* argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now(); // Le temp quand le programme a commencé a executer
    MPI_Init(&nargs, &argv);
    
    //Communication
    MPI_Comm glob_com;
    MPI_Comm_dup(MPI_COMM_WORLD, &glob_com);

    //Nombre de processus
    int nbp;
    MPI_Comm_size(glob_com, &nbp);

    //Le rank du processus
    int rank;
    MPI_Comm_rank(glob_com, &rank);

    //Le nom du do processus
    char name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(name, &name_len);

    if(rank==0){
        SDL_Init( SDL_INIT_VIDEO );
    }

    std::string path = "test_" + std::to_string(nbp) + "_cores/";
    //Les fichiers pour analyse des données de temps
    std::ofstream csv_iterations(path+"iteration"+std::to_string(rank)+".csv");
    csv_iterations << "Iteration,Temps_Fourmis_ms,Temps_Pheromone_Communicator_ms,Temps_Evaporation_ms,Temps_Evaporation_Communicator_ms,Temps_Update_ms,Temps_Computation\n";
    std::ofstream csv_resume;
    if(nbp==1){
        csv_resume.open("resume.csv");
        csv_resume << "Threads,Temps_Creation_Fourmis,Iteration_Nourriture,Temps_Computation,Temps_Total\n";
    }
    else{
        csv_resume.open("resume.csv", std::ios::app);
    }

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
    colony.parallel_create_ants(land.dimensions(), seed, rank*nb_ants/nbp, (rank+1)*nb_ants/nbp);
    //Colonie colony(original_colony, rank*fourmis/nbp, (rank+1)*fourmis/nbp)
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ant_creation_duration = t2-t1;
    std::cout << "Time to create ants: " << ant_creation_duration.count() << "ms\n";
    
    // On crée toutes les fourmis dans la fourmilière.
    pheronome phen(land.dimensions(), pos_food, pos_nest, rank, nbp, alpha, beta);
    
    std::unique_ptr<Window> win = nullptr;
    std::unique_ptr<Renderer> renderer = nullptr;
    
    if(rank==0){
        win = std::make_unique<Window>("Ant Simulation", 2*land.dimensions()+10, land.dimensions()+266);
        renderer = std::make_unique<Renderer>( land, phen, pos_nest, pos_food, colony );
    }

    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    
    //Temps pur de simulation (sans affichage et initialisation)
    std::chrono::duration<double,std::milli> pure_computation_time(0);
    size_t stop_after_finding_food = 0;
    while (cont_loop) {
        ++it;
        if(rank==0){
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    cont_loop = false;
            }
        }
        if(stop_after_finding_food>0 && it>stop_after_finding_food){
            cont_loop = false;
        }
        
        //La simulation pure (l'avance des fourmis et actualisation des pheromones)
        auto t_start_sim = std::chrono::high_resolution_clock::now();
        advance_time( land, phen, pos_nest, pos_food, colony, food_quantity, it, glob_com, nbp, csv_iterations);
        auto t_end_sim = std::chrono::high_resolution_clock::now();
        pure_computation_time += (t_end_sim - t_start_sim);
        if (it % iterations_until_print == 0) {
            csv_iterations << "," << pure_computation_time.count() << '\n';
        }

        // Renderization et contrôle de la fênetre de visualisation
        if (rank==0){
            renderer->display( *win, global_food_quantity);
            win->blit();
        }

        MPI_Allreduce(&food_quantity, &global_food_quantity, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, glob_com);

        if ( not_food_in_nest && global_food_quantity > 0 ) {
            auto time_of_first_food = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration_until_first_food = time_of_first_food - start_time;

            std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
            std::cout << "Temps pur de calcul (sans affichage): " << pure_computation_time.count() << "ms\n";
            std::cout << "Temps total (avec affichage et init):" << duration_until_first_food.count() << "ms\n";

            int num_cores = nbp;
            csv_resume << num_cores << "," 
                       << ant_creation_duration.count() << ","
                       << it << "," 
                       << pure_computation_time.count() << ","
                       << duration_until_first_food.count() << "\n";

            stop_after_finding_food = it+600;
            not_food_in_nest = false;
        }
        //SDL_Delay(10);
    }
    MPI_Finalize();
    if(rank==0){
        SDL_Quit();
    }
    return 0;
}