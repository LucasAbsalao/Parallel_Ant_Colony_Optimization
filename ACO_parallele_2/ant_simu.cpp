#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "fractal_land.hpp"
#include "colonie.hpp"
#include "pheronome.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include "rand_generator.hpp"

// Structure to store performance data for each core
struct CoreStats {
    std::size_t iteration;
    double active_time_ms;
    int num_ants;
    std::size_t food_count;
};

// Global simulation buffers
std::vector<std::vector<AntData>> global_mig_left;
std::vector<std::vector<AntData>> global_mig_right;
std::vector<std::vector<pheronome::pheronome_t>> global_edge_left;
std::vector<std::vector<pheronome::pheronome_t>> global_edge_right;
int global_migrants_in_step = 0; 

// Global statistics buffer (one vector per core)
std::vector<std::vector<CoreStats>> global_core_stats;

void advance_time( const fractal_land& land, 
                   std::vector<pheronome>& phens,
                   const position_t& pos_nest, const position_t& pos_food,
                   std::vector<Colonie>& colonies, std::size_t& cpteur,
                   const std::vector<int>& x_mins, const std::vector<int>& x_maxs, int num_threads, std::size_t iteration)
{
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        colonies[tid].reset_times(); 
        
        bool step_finished = false;
        double thread_active_time = 0.0;
        
        while (!step_finished) {
            
            auto t_start_work = std::chrono::high_resolution_clock::now();
            
            for (int i = 0; i < colonies[tid].get_qtt_ants(); ++i) {
                colonies[tid].advance(i, phens[tid], land, pos_food, pos_nest, cpteur, x_mins[tid], x_maxs[tid]);
            }

            auto migs = colonies[tid].extract_migrants(x_mins[tid], x_maxs[tid]);
            global_mig_left[tid].clear(); 
            global_mig_right[tid].clear();
            for(const auto& m : migs) {
                if(m.pos.x < x_mins[tid]) global_mig_left[tid].push_back(m);
                else global_mig_right[tid].push_back(m);
            }
            
            auto t_end_work = std::chrono::high_resolution_clock::now();
            thread_active_time += std::chrono::duration<double, std::milli>(t_end_work - t_start_work).count();

            #pragma omp barrier

            if(tid > 0) colonies[tid].inject_migrants(global_mig_right[tid-1]);
            if(tid < num_threads - 1) colonies[tid].inject_migrants(global_mig_left[tid+1]);

            #pragma omp single
            {
                global_migrants_in_step = 0;
            }
            
            int my_migrants = global_mig_left[tid].size() + global_mig_right[tid].size();
            #pragma omp atomic
            global_migrants_in_step += my_migrants;
            
            #pragma omp barrier

            if (global_migrants_in_step == 0) {
                step_finished = true;
            }
        } 

        // Save iteration statistics before swapping pheromones
        global_core_stats[tid].push_back({iteration, thread_active_time, colonies[tid].get_qtt_ants(), cpteur});

        phens[tid].do_evaporation();
        #pragma omp barrier
        
        phens[tid].update();
        #pragma omp barrier
        
        if(tid > 0) global_edge_left[tid] = phens[tid].get_column(x_mins[tid]); 
        if(tid < num_threads - 1) global_edge_right[tid] = phens[tid].get_column(x_maxs[tid] - 1); 
        #pragma omp barrier
        
        if(tid > 0) phens[tid].set_ghost_column(x_mins[tid] - 1, global_edge_right[tid-1]);
        if(tid < num_threads - 1) phens[tid].set_ghost_column(x_maxs[tid], global_edge_left[tid+1]);
    }
}

int main(int nargs, char* argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now();
    SDL_Init( SDL_INIT_VIDEO );
    
    int NUM_THREADS_SIMULATION = (nargs > 1) ? std::atoi(argv[1]) : 4; 

    global_core_stats.resize(NUM_THREADS_SIMULATION);

    std::size_t seed = 2026;
    const int nb_ants = 5000;
    const double eps = 0.8;
    const double alpha = 0.7;
    const double beta = 0.999;
    
    position_t pos_nest{256,256};
    position_t pos_food{500,500};
    
    fractal_land land(8,2,1.,1024);
    double max_val = 0.0, min_val = 0.0;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j ) {
            max_val = std::max(max_val, land(i,j));
            min_val = std::min(min_val, land(i,j));
        }
    double delta = max_val - min_val;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j )  {
            land(i,j) = (land(i,j)-min_val)/delta;
        }

    auto gen_ant_pos = [&land, &seed] () { return rand_int32(0, land.dimensions()-1, seed); };
    
    std::vector<position_t> pos_iniciais(nb_ants);
    std::vector<std::size_t> seeds_iniciais(nb_ants);
    
    for ( size_t i = 0; i < nb_ants; ++i ) {
        pos_iniciais[i] = {gen_ant_pos(), gen_ant_pos()};
        seeds_iniciais[i] = seed; 
    }

    std::vector<Colonie> colonies(NUM_THREADS_SIMULATION);
    std::vector<pheronome> phens;
    std::vector<int> x_mins(NUM_THREADS_SIMULATION);
    std::vector<int> x_maxs(NUM_THREADS_SIMULATION);
    
    global_mig_left.resize(NUM_THREADS_SIMULATION);
    global_mig_right.resize(NUM_THREADS_SIMULATION);
    global_edge_left.resize(NUM_THREADS_SIMULATION);
    global_edge_right.resize(NUM_THREADS_SIMULATION);

    int width = land.dimensions() / NUM_THREADS_SIMULATION;
    for (int t = 0; t < NUM_THREADS_SIMULATION; ++t) {
        colonies[t].set_exploration_coef(eps);
        x_mins[t] = t * width;
        x_maxs[t] = (t == NUM_THREADS_SIMULATION - 1) ? land.dimensions() : (t + 1) * width;
        phens.emplace_back(land.dimensions(), x_mins[t], x_maxs[t], pos_food, pos_nest, alpha, beta);
    }

    for ( size_t i = 0; i < nb_ants; ++i ) {
        position_t pos = pos_iniciais[i];
        for (int t = 0; t < NUM_THREADS_SIMULATION; ++t) {
            if (pos.x >= x_mins[t] && pos.x < x_maxs[t]) {
                colonies[t].add_ant(pos, seeds_iniciais[i]);
                break;
            }
        }
    }

    Window win("Ant Simulation - Parallélisation Adaptative", 2*land.dimensions()+10, land.dimensions()+266);
    Renderer renderer( land, phens, pos_nest, pos_food, colonies );
    
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    std::chrono::duration<double,std::milli> pure_computation_time(0);

    std::cout << "Starting simulation with " << NUM_THREADS_SIMULATION << " threads...\n";
    std::cout << "Close the window (X) to finish and generate CSV files.\n";

    size_t stop_after_finding_food = 0;
    while (cont_loop) {
        ++it;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                cont_loop = false;
        }
        if(stop_after_finding_food > 0 && it > stop_after_finding_food){
            cont_loop = false;
        }

        auto t_start_sim = std::chrono::high_resolution_clock::now();
        advance_time( land, phens, pos_nest, pos_food, colonies, food_quantity, x_mins, x_maxs, NUM_THREADS_SIMULATION, it );
        auto t_end_sim = std::chrono::high_resolution_clock::now();
        pure_computation_time += (t_end_sim - t_start_sim);

        renderer.display( win, food_quantity );

        if ( not_food_in_nest && food_quantity > 0 ) {
            auto time_of_first_food = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration_until_first_food = time_of_first_food - start_time;
            std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
            std::cout << "Temps pur de calcul (sans affichage): " << pure_computation_time.count() << "ms\n";
            std::cout << "Temps total (avec affichage et init): " << duration_until_first_food.count() << "ms\n";
            stop_after_finding_food = it + 600;
            not_food_in_nest = false;
        }
    }
    SDL_Quit();

    // =========================================================================
    // CSV DATA EXPORT ROUTINE
    // =========================================================================
    std::cout << "\nGenerating performance analysis files...\n";
    
    namespace fs = std::filesystem;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "test_" << NUM_THREADS_SIMULATION << "cores_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    std::string folder_name = ss.str();
    
    fs::create_directory(folder_name);

    for (int t = 0; t < NUM_THREADS_SIMULATION; ++t) {
        std::string filename = folder_name + "/core_" + std::to_string(t) + ".csv";
        std::ofstream file(filename);
        
        file << "Iteration,ActiveTime_ms,NumAnts,FoodCount\n";
        
        for (const auto& stat : global_core_stats[t]) {
            file << stat.iteration << "," 
                 << stat.active_time_ms << "," 
                 << stat.num_ants << ","
                 << stat.food_count << "\n";
        }
        file.close();
    }
    
    std::cout << "Success! " << NUM_THREADS_SIMULATION << " CSV files saved in folder: " << folder_name << "\n";

    return 0;
}