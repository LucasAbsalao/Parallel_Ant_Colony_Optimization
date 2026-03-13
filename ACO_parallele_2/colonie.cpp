#include "colonie.hpp"
#include "rand_generator.hpp"

void Colonie::set_loaded(int idx){ state_ants.at(idx) = loaded; }
void Colonie::unset_loaded(int idx){ state_ants.at(idx) = unloaded; }
bool Colonie::is_loaded(int idx) const{ return state_ants.at(idx) == loaded; }
const position_t& Colonie::get_position(int idx) const{ return pos_ants.at(idx); }
void Colonie::set_exploration_coef(double eps){ m_eps = eps; }

void Colonie::add_ant(position_t pos, std::size_t seed) {
    pos_ants.push_back(pos);
    state_ants.push_back(unloaded);
    m_seed.push_back(seed);
    consumed_time_ants.push_back(0.0);
    needs_marking_ants.push_back(false);
    qtt_ants++;
}

void Colonie::reset_times() {
    for (int i = 0; i < qtt_ants; ++i) {
        consumed_time_ants[i] = 0.0;
    }
}

std::vector<AntData> Colonie::extract_migrants(int x_min, int x_max) {
    std::vector<AntData> migrants;
    for (int i = qtt_ants - 1; i >= 0; --i) {
        if (pos_ants[i].x < x_min || pos_ants[i].x >= x_max) {
            migrants.push_back({pos_ants[i], static_cast<int>(state_ants[i]), m_seed[i], 
                                consumed_time_ants[i], needs_marking_ants[i]});
            
            pos_ants[i] = pos_ants.back();
            state_ants[i] = state_ants.back();
            m_seed[i] = m_seed.back();
            consumed_time_ants[i] = consumed_time_ants.back();
            needs_marking_ants[i] = needs_marking_ants.back();
            
            pos_ants.pop_back(); state_ants.pop_back(); m_seed.pop_back(); 
            consumed_time_ants.pop_back(); needs_marking_ants.pop_back();
            qtt_ants--;
        }
    }
    return migrants;
}

void Colonie::inject_migrants(const std::vector<AntData>& migrants) {
    for (const auto& m : migrants) {
        pos_ants.push_back(m.pos);
        state_ants.push_back(static_cast<state>(m.state));
        m_seed.push_back(m.seed);
        consumed_time_ants.push_back(m.consumed_time);
        needs_marking_ants.push_back(m.needs_marking);
        qtt_ants++;
    }
}

void Colonie::advance(int idx, pheronome& phen, const fractal_land& land,
            const position_t& pos_food, const position_t& pos_nest, 
            std::size_t& cpteur_food, int x_min, int x_max ){
            
    // Apply pending pheromone from previous iteration (boundary crossing)
    if (needs_marking_ants[idx]) {
        phen.mark_pheronome(pos_ants[idx]);
        needs_marking_ants[idx] = false;
    }
    
    auto ant_choice = [this, idx]() mutable { return rand_double( 0., 1., this->m_seed.at(idx) ); };
    auto dir_choice = [this, idx]() mutable { return rand_int32( 1, 4, this->m_seed.at(idx) ); };
    
    while ( consumed_time_ants[idx] < 1. ) {
        int ind_pher = ( is_loaded(idx) ? 1 : 0 );
        double choix = ant_choice( );
        position_t old_pos_ant = get_position(idx);
        position_t new_pos_ant = old_pos_ant;
        
        double max_phen = std::max( {phen( new_pos_ant.x - 1, new_pos_ant.y )[ind_pher],
                                     phen( new_pos_ant.x + 1, new_pos_ant.y )[ind_pher],
                                     phen( new_pos_ant.x, new_pos_ant.y - 1 )[ind_pher],
                                     phen( new_pos_ant.x, new_pos_ant.y + 1 )[ind_pher]} );
        
        if ( ( choix > m_eps ) || ( max_phen <= 0. ) ) {
            do {
                new_pos_ant = old_pos_ant;
                int d = dir_choice();
                if ( d==1 ) new_pos_ant.x -= 1;
                if ( d==2 ) new_pos_ant.y -= 1;
                if ( d==3 ) new_pos_ant.x += 1;
                if ( d==4 ) new_pos_ant.y += 1;
            } while ( phen[new_pos_ant][ind_pher] == -1 );
        } else {
            if ( phen( new_pos_ant.x - 1, new_pos_ant.y )[ind_pher] == max_phen ) new_pos_ant.x -= 1;
            else if ( phen( new_pos_ant.x + 1, new_pos_ant.y )[ind_pher] == max_phen ) new_pos_ant.x += 1;
            else if ( phen( new_pos_ant.x, new_pos_ant.y - 1 )[ind_pher] == max_phen ) new_pos_ant.y -= 1;
            else new_pos_ant.y += 1;
        }
        
        pos_ants.at(idx) = new_pos_ant;
        
        if ( new_pos_ant == pos_nest ) {
            if ( is_loaded(idx) ) {
                #pragma omp atomic
                cpteur_food += 1;
            }
            unset_loaded(idx);
        }
        if ( new_pos_ant == pos_food ) {
            set_loaded(idx);
        }

        // Add the time cost of the terrain BEFORE checking boundaries
        consumed_time_ants[idx] += land( new_pos_ant.x, new_pos_ant.y);

        if (new_pos_ant.x < x_min || new_pos_ant.x >= x_max) {
            needs_marking_ants[idx] = true; // Notify the neighboring thread to mark pheromone
            break; 
        }

        phen.mark_pheronome( new_pos_ant );
    }
}