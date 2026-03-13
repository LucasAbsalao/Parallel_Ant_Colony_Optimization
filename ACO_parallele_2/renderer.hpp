#pragma once
#include <vector>
#include "fractal_land.hpp"
#include "colonie.hpp"
#include "pheronome.hpp"
#include "window.hpp"

class Renderer
{
public:
    Renderer(  const fractal_land& land, 
               const std::vector<pheronome>& phens,
               const position_t& pos_nest, const position_t& pos_food,
               const std::vector<Colonie>& colonies );

    Renderer(const Renderer& ) = delete;
    ~Renderer();

    void display( Window& win, std::size_t const& compteur );
private:
    fractal_land const& m_ref_land;
    SDL_Texture* m_land{ nullptr }; 
    const std::vector<pheronome>& m_phens;
    const position_t& m_pos_nest;
    const position_t& m_pos_food;
    const std::vector<Colonie>& m_colonies;
    std::vector<std::size_t> m_curve;    
};