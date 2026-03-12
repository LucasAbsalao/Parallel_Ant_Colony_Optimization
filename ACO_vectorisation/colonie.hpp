#pragma once
# include <utility>
# include <vector>
# include "pheronome.hpp"
# include "fractal_land.hpp"
# include "basic_types.hpp"
# include "rand_generator.hpp"

class Colonie{
    public:
        enum state { unloaded = 0, loaded = 1 };
        Colonie(const size_t qtt_ants, double eps);

        Colonie(const Colonie& other_col) = default;
        Colonie(Colonie&& other_col) = default;
        ~Colonie() = default;

        void create_ants(unsigned long int dimension, std::size_t seed);

        void set_loaded(int idx);
        void unset_loaded(int idx);
        bool is_loaded(int idx) const;

        const position_t& get_position(int idx) const;
        const int get_qtt_ants() const;
        void set_exploration_coef(double eps);

        void advance(int idx, pheronome& phen, const fractal_land& land,
                    const position_t& pos_food, const position_t& pos_nest, 
                    std::size_t& cpteur_food );

    private:
        double m_eps; // Coefficient d'exploration commun à toutes les fourmis.
        int qtt_ants;
        std::vector<position_t> pos_ants;
        std::vector<state> state_ants;
        std::vector<std::size_t> m_seed;
};