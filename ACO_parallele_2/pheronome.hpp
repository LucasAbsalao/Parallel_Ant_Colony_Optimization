#ifndef _PHERONOME_HPP_
#define _PHERONOME_HPP_
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#include <stdexcept>
#include "basic_types.hpp"

class pheronome {
public:
    using size_t      = unsigned long;
    using pheronome_t = std::array< double, 2 >;

    pheronome( size_t global_dim, int x_min, int x_max, const position_t& pos_food, const position_t& pos_nest,
               double alpha = 0.7, double beta = 0.999 )
        : m_dim( global_dim ), m_stride( global_dim + 2 ), m_size_x( x_max - x_min + 2 ),
          m_xmin(x_min), m_xmax(x_max),
          m_alpha(alpha), m_beta(beta),
          m_map_of_pheronome( m_size_x * m_stride, {{0., 0.}} ),
          m_buffer_pheronome( )
    {
        m_pos_nest = pos_nest;
        m_pos_food = pos_food;
        
        if (m_pos_food.x >= m_xmin && m_pos_food.x < m_xmax)
            m_map_of_pheronome.at(index(pos_food))[0] = 1.;
        if (m_pos_nest.x >= m_xmin && m_pos_nest.x < m_xmax)
            m_map_of_pheronome.at(index(pos_nest))[1] = 1.;
            
        cl_update( );
        m_buffer_pheronome = m_map_of_pheronome;
    }

    pheronome( const pheronome& ) = delete;
    pheronome( pheronome&& )      = default;
    ~pheronome( )                 = default;

    // Use int to safely allow -1 at the boundaries
    int index(int x, int y) const { return (x - m_xmin + 1) * m_stride + y + 1; }
    int index( const position_t& pos ) const { return (pos.x - m_xmin + 1) * m_stride + pos.y + 1; }

    // Using .at() for safe bounds checking
    pheronome_t& operator()( int i, int j ) { return m_map_of_pheronome.at(index(i, j)); }
    const pheronome_t& operator()( int i, int j ) const { return m_map_of_pheronome.at(index(i, j)); }
    pheronome_t& operator[] ( const position_t& pos ) { return m_map_of_pheronome.at(index(pos)); }
    const pheronome_t& operator[] ( const position_t& pos ) const { return m_map_of_pheronome.at(index(pos)); }

    std::vector<pheronome_t> get_column(int x_global) const {
        std::vector<pheronome_t> col(m_stride);
        int local_x = x_global - m_xmin + 1;
        for (size_t y = 0; y < m_stride; ++y) col.at(y) = m_map_of_pheronome.at(local_x * m_stride + y);
        return col;
    }

    void set_ghost_column(int x_global, const std::vector<pheronome_t>& col) {
        int local_x = x_global - m_xmin + 1;
        for (size_t y = 0; y < m_stride; ++y) m_map_of_pheronome.at(local_x * m_stride + y) = col.at(y);
    }

    void do_evaporation( ) {
        for ( std::size_t i = 1; i < m_size_x - 1; ++i )
            for ( std::size_t j = 1; j <= m_dim; ++j ) {
                m_buffer_pheronome.at(i * m_stride + j)[0] *= m_beta;
                m_buffer_pheronome.at(i * m_stride + j)[1] *= m_beta;
            }
    }

    void mark_pheronome( const position_t& pos ) {
        int i = pos.x; 
        int j = pos.y; 
        const pheronome_t& left_cell   = (*this)( i - 1, j );
        const pheronome_t& right_cell  = (*this)( i + 1, j );
        const pheronome_t& upper_cell  = (*this)( i, j - 1 );
        const pheronome_t& bottom_cell = (*this)( i, j + 1 );
        
        double v1_left = std::max( left_cell[0], 0. );
        double v2_left = std::max( left_cell[1], 0. );
        double v1_right = std::max( right_cell[0], 0. );
        double v2_right = std::max( right_cell[1], 0. );
        double v1_upper = std::max( upper_cell[0], 0. );
        double v2_upper = std::max( upper_cell[1], 0. );
        double v1_bottom = std::max( bottom_cell[0], 0. );
        double v2_bottom = std::max( bottom_cell[1], 0. );
        
        m_buffer_pheronome.at(index(pos))[0] =
            m_alpha * std::max( {v1_left, v1_right, v1_upper, v1_bottom} ) +
            ( 1 - m_alpha ) * 0.25 * ( v1_left + v1_right + v1_upper + v1_bottom );
        m_buffer_pheronome.at(index(pos))[1] =
            m_alpha * std::max( {v2_left, v2_right, v2_upper, v2_bottom} ) +
            ( 1 - m_alpha ) * 0.25 * ( v2_left + v2_right + v2_upper + v2_bottom );
    }

    void update( ) {
        m_map_of_pheronome.swap( m_buffer_pheronome );
        cl_update( );
        if (m_pos_food.x >= m_xmin && m_pos_food.x < m_xmax)
            m_map_of_pheronome.at(index(m_pos_food))[0] = 1;
        if (m_pos_nest.x >= m_xmin && m_pos_nest.x < m_xmax)
            m_map_of_pheronome.at(index(m_pos_nest))[1] = 1;
    }

private:
    void cl_update( ) {
        for ( unsigned long i = 0; i < m_size_x; ++i ) {
            m_map_of_pheronome.at(i * m_stride) = {{-1., -1.}};
            m_map_of_pheronome.at(i * m_stride + m_dim + 1) = {{-1., -1.}};
        }
        if (m_xmin == 0) {
            for ( unsigned long j = 0; j < m_stride; ++j )
                m_map_of_pheronome.at(0 * m_stride + j) = {{-1., -1.}};
        }
        if (m_xmax == (int)m_dim) {
            for ( unsigned long j = 0; j < m_stride; ++j )
                m_map_of_pheronome.at((m_size_x - 1) * m_stride + j) = {{-1., -1.}};
        }
    }
    unsigned long m_dim, m_stride, m_size_x;
    int m_xmin, m_xmax;
    double m_alpha, m_beta;
    std::vector< pheronome_t > m_map_of_pheronome, m_buffer_pheronome;
    position_t m_pos_nest, m_pos_food;
};
#endif