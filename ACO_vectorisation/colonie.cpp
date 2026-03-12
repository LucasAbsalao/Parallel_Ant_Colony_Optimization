#include"colonie.hpp"
#include "rand_generator.hpp"

void Colonie::set_loaded(int idx){
    state_ants.at(idx) = loaded;
}

void Colonie::unset_loaded(int idx){
    state_ants.at(idx) = unloaded;
}

bool Colonie::is_loaded(int idx) const{
    return state_ants.at(idx) == loaded;
}

const position_t& Colonie::get_position(int idx) const{
    return pos_ants.at(idx);
}

void Colonie::set_exploration_coef(double eps){
    m_eps = eps;
}

void Colonie::advance(int idx, pheronome& phen, const fractal_land& land,
            const position_t& pos_food, const position_t& pos_nest, 
            std::size_t& cpteur_food ){
    auto ant_choice = [this, idx]() mutable { return rand_double( 0., 1., this->m_seed[idx] ); };
    auto dir_choice = [this, idx]() mutable { return rand_int32( 1, 4, this->m_seed[idx] ); };
    double                                   consumed_time = 0.;
    // Tant que la fourmi peut encore bouger dans le pas de temps imparti
    while ( consumed_time < 1. ) {
        // Si la fourmi est chargée, elle suit les phéromones de deuxième type, sinon ceux du premier.
        int        ind_pher    = ( is_loaded(idx) ? 1 : 0 );
        double     choix       = ant_choice( );
        position_t old_pos_ant = get_position(idx);
        position_t new_pos_ant = old_pos_ant;
        double max_phen    = std::max( {phen( new_pos_ant.x - 1, new_pos_ant.y )[ind_pher],
                                     phen( new_pos_ant.x + 1, new_pos_ant.y )[ind_pher],
                                     phen( new_pos_ant.x, new_pos_ant.y - 1 )[ind_pher],
                                     phen( new_pos_ant.x, new_pos_ant.y + 1 )[ind_pher]} );
        if ( ( choix > m_eps ) || ( max_phen <= 0. ) ) {
            do {
                new_pos_ant = old_pos_ant;
                int d = dir_choice();
                if ( d==1 ) new_pos_ant.x  -= 1;
                if ( d==2 ) new_pos_ant.y -= 1;
                if ( d==3 ) new_pos_ant.x  += 1;
                if ( d==4 ) new_pos_ant.y += 1;

            } while ( phen[new_pos_ant][ind_pher] == -1 );
        } else {
            // On choisit la case où le phéromone est le plus fort.
            if ( phen( new_pos_ant.x - 1, new_pos_ant.y )[ind_pher] == max_phen )
                new_pos_ant.x -= 1;
            else if ( phen( new_pos_ant.x + 1, new_pos_ant.y )[ind_pher] == max_phen )
                new_pos_ant.x += 1;
            else if ( phen( new_pos_ant.x, new_pos_ant.y - 1 )[ind_pher] == max_phen )
                new_pos_ant.y -= 1;
            else  // if (phen(new_pos_ant.first,new_pos_ant.second+1)[ind_pher] == max_phen)
                new_pos_ant.y += 1;
        }
        consumed_time += land( new_pos_ant.x, new_pos_ant.y);
        phen.mark_pheronome( new_pos_ant );
        pos_ants[idx] = new_pos_ant;
        if ( get_position(idx) == pos_nest ) {
            if ( is_loaded(idx) ) {
                cpteur_food += 1;
            }
            unset_loaded(idx);
        }
        if ( get_position(idx) == pos_food ) {
            set_loaded(idx);
        }
    }
}