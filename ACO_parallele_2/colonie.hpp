#pragma once
# include <utility>
# include <vector>
# include "pheronome.hpp"
# include "fractal_land.hpp"
# include "basic_types.hpp"

struct AntData {
    position_t pos;
    int state; 
    std::size_t seed;
    double consumed_time;
    bool needs_marking; // Ensures it marks the pheromone after crossing the boundary
};

class Colonie{
    public:
        enum state { unloaded = 0, loaded = 1 };
        Colonie() : m_eps(0.0), qtt_ants(0) {}
        Colonie(const size_t& qtt_ants): qtt_ants(qtt_ants) {}

        Colonie(const Colonie& other_col) = default;
        Colonie(Colonie&& other_col) = default;
        ~Colonie() = default;

        void set_loaded(int idx);
        void unset_loaded(int idx);
        bool is_loaded(int idx) const;

        const position_t& get_position(int idx) const;
        void set_exploration_coef(double eps);

        void add_ant(position_t pos, std::size_t seed);
        void reset_times(); // Resets the time of all ants at the beginning of the frame
        
        std::vector<AntData> extract_migrants(int x_min, int x_max);
        void inject_migrants(const std::vector<AntData>& migrants);
        int get_qtt_ants() const { return qtt_ants; }

        void advance(int idx, pheronome& phen, const fractal_land& land,
                    const position_t& pos_food, const position_t& pos_nest, 
                    std::size_t& cpteur_food, int x_min, int x_max );

    private:
        double m_eps;
        int qtt_ants;
        std::vector<position_t> pos_ants;
        std::vector<state> state_ants;
        std::vector<std::size_t> m_seed;
        std::vector<double> consumed_time_ants;
        std::vector<bool> needs_marking_ants;
};