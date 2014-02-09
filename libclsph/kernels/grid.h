#ifndef _GRID_H_
#define _GRID_H_

#include "common.cl.h"

int3 get_cell_coords(int index, simulation_parameters params);
int get_grid_index(int x, int y, int z, simulation_parameters params);
uint2 get_start_end_indices_for_cell(int cell_index, global const unsigned int* cell_table, simulation_parameters params);

int3 get_cell_coords(int index, simulation_parameters params) {
    int3 coords = {
        index % params.grid_size_x,
        (index / params.grid_size_x) % params.grid_size_y,
        index / (params.grid_size_x * params.grid_size_y),
    };
    return coords;
}

int get_grid_index(int x, int y, int z, simulation_parameters params) {
    return x + params.grid_size_x * (y + params.grid_size_y * z);
}

uint2 get_start_end_indices_for_cell(int cell_index, global const unsigned int* cell_table, simulation_parameters params) {
    uint2 indices = {
        cell_table[cell_index],
        (params.grid_cell_count > cell_index + 1) ? cell_table[cell_index + 1] : params.particles_count,
    };

    return indices;
}

/**
* locate_in_grid
* 1)Updates each particle with its position in the grid
* 2)Fills an array with the number of particles contained in each grid cell
*
* Parameters:------------------------------------------------------------
* (in/out)  particles        : The buffer contaning all the particle data
* (in)      params           : The simulation parameters
* (in)      volumes          : Contains all the volumes with which the fluid can interact. Also contains the simulation boundaries
*
* Kernel information:----------------------------------------------------
* Global work group size : particles_count
*/
void kernel locate_in_grid(
    global const particle* particles, 
    global particle* out_particles,
    simulation_parameters params,
    collision_volumes volumes){

    size_t current_particle_index = get_global_id(0);
    out_particles[current_particle_index] = particles[current_particle_index];

    float3 position_in_grid = {0.f,0.f,0.f};
    
    float x_min = volumes.scene_bounding_box.center.x - volumes.scene_bounding_box.axis_extends.x;
    float y_min = volumes.scene_bounding_box.center.y - volumes.scene_bounding_box.axis_extends.y;
    float z_min = volumes.scene_bounding_box.center.z - volumes.scene_bounding_box.axis_extends.z;
    
    //Grid cells will always have a radius length h
    position_in_grid.x = (particles[current_particle_index].position.x - x_min) / (params.h * 2);
    position_in_grid.y = (particles[current_particle_index].position.y - y_min) / (params.h * 2);
    position_in_grid.z = (particles[current_particle_index].position.z - z_min) / (params.h * 2);

    int grid_index = get_grid_index((int)position_in_grid.x, (int)position_in_grid.y, (int)position_in_grid.z, params);
    
    out_particles[current_particle_index].grid_index = grid_index;
}
#endif