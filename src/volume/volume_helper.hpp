#ifndef HEADER_VOLUME_HELPER_HPP
#define HEADER_VOLUME_HELPER_HPP

#include <iostream>
#include <cassert>
#include <vector>
#include <random>
#include <fstream>
#include <glpk.h>
#include <chrono>



extern "C" { // must be included C stlye
#include "volume.h"
}

#include "../util/cli_functions.hpp"
#include "../util/performance_counter.hpp"
#include "../../polyvest/vol.h"
#include "volume_cost.hpp"

std::ostream& operator<<(std::ostream& os, const Polytope& p);
std::ostream& operator<<(std::ostream& os, const Polytope* p);

Polytope* Polytope_new_box(int n, FT r);
PolytopeT* PolytopeT_new_box(int n, FT r);
// allocates / generates cube polytope with radius r


class CLIFunctionsVolume : public CLIFunctions {
public:
   CLIFunctionsVolume(CLI &cli) : CLIFunctions(cli) {
      // initialize memory arrays of volume library
      volume_lib_init(200,20);// max_n=200, max_b=20
      
      // add performance counter cost functions
      pc_stack().add((void*)dotProduct_ref, new PC_Cost_Wrapper<dotProduct_cost_f>(dotProduct_cost_ref,"dotProduct_ref"));
      pc_stack().add((void*)dotProduct_2acc, new PC_Cost_Wrapper<dotProduct_cost_f>(dotProduct_cost_ref,"dotProduct_2acc"));
      pc_stack().add((void*)dotProduct_auto1, new PC_Cost_Wrapper<dotProduct_cost_f>(dotProduct_cost_ref,"dotProduct_auto1"));
      pc_stack().add((void*)dotProduct_auto2, new PC_Cost_Wrapper<dotProduct_cost_f>(dotProduct_cost_ref,"dotProduct_auto2"));
      pc_stack().add((void*)dotProduct_vec1, new PC_Cost_Wrapper<dotProduct_cost_f>(dotProduct_cost_ref,"dotProduct_vec1"));
      
      pc_stack().add((void*)vectorNorm_ref, new PC_Cost_Wrapper<vectorNorm_cost_f>(vectorNorm_cost_ref,"vectorNorm_ref"));
      
      pc_stack().add((void*)Ball_intersect_ref, new PC_Cost_Wrapper<Ball_intersect_cost_f>(Ball_intersect_cost_ref,"Ball_intersect_ref"));
      pc_stack().add((void*)Ball_intersectCoord_ref, new PC_Cost_Wrapper<Ball_intersectCoord_cost_f>(Ball_intersectCoord_cost_ref,"Ball_intersectCoord_ref"));
      
      // Polytope
      pc_stack().add((void*)Polytope_intersect_ref, new PC_Cost_Wrapper<intersect_cost_f>(Polytope_intersect_cost_ref,"Polytope_intersect_ref"));
      pc_stack().add((void*)Polytope_intersectCoord_ref, new PC_Cost_Wrapper<intersectCoord_cost_f>(Polytope_intersectCoord_cost_ref,"Polytope_intersectCoord_ref"));
      pc_stack().add((void*)Polytope_intersectCoord_cached_ref, new PC_Cost_Wrapper<intersectCoord_cost_f>(Polytope_intersectCoord_cached_cost_ref,"Polytope_intersectCoord_cached_ref"));
      pc_stack().add((void*)Polytope_cacheUpdateCoord_ref, new PC_Cost_Wrapper<cacheUpdateCoord_cost_f>(Polytope_cacheUpdateCoord_cost_ref,"Polytope_cacheUpdateCoord_ref"));
      
      // PolytopeT
      pc_stack().add((void*)PolytopeT_intersectCoord_ref, new PC_Cost_Wrapper<intersectCoord_cost_f>(PolytopeT_intersectCoord_cost_ref,"PolytopeT_intersectCoord_ref"));
      pc_stack().add((void*)PolytopeT_intersect_ref, new PC_Cost_Wrapper<intersect_cost_f>(PolytopeT_intersect_cost_ref,"PolytopeT_intersect_ref"));
      pc_stack().add((void*)PolytopeT_intersectCoord_cached_ref, new PC_Cost_Wrapper<intersectCoord_cost_f>(PolytopeT_intersectCoord_cached_cost_ref,"PolytopeT_intersectCoord_cached_ref"));
      pc_stack().add((void*)PolytopeT_cacheUpdateCoord_ref, new PC_Cost_Wrapper<cacheUpdateCoord_cost_f>(PolytopeT_cacheUpdateCoord_cost_ref,"PolytopeT_cacheUpdateCoord_ref"));
 
      // Ellipsoid
      pc_stack().add((void*)Ellipsoid_intersect_ref, new PC_Cost_Wrapper<intersect_cost_f>(Ellipsoid_intersect_cost_ref,"Ellipsoid_intersect_ref"));
      pc_stack().add((void*)Ellipsoid_intersectCoord_ref, new PC_Cost_Wrapper<intersectCoord_cost_f>(Ellipsoid_intersectCoord_cost_ref,"Ellipsoid_intersectCoord_ref"));
      pc_stack().add((void*)Ellipsoid_cacheUpdateCoord_ref, new PC_Cost_Wrapper<cacheUpdateCoord_cost_f>(Ellipsoid_cacheUpdateCoord_cost_ref,"Ellipsoid_cacheUpdateCoord_ref"));
      
      // volume
      pc_stack().add((void*)volume_ref, new PC_Cost_Wrapper<volume_cost_f>(volume_cost_ref,"volume_ref"));
      pc_stack().add((void*)walk_ref, new PC_Cost_Wrapper<walk_cost_f>(walk_cost_ref,"walk_ref"));
      pc_stack().add((void*)walkCoord_ref, new PC_Cost_Wrapper<walk_cost_f>(walkCoord_cost_ref,"walkCoord_ref"));
      

      // please add your functions below.
      // 
      // handle to global variable used to reference current choice of function
      // opt code for cli arg, under which the function is configured (char)
      // string for function name
      // string for default function name
      // map: function name -> function ptr
      
      claimOpt('f',"Algorithm Functions");
      add(new CLIF_Option<xyz_f_t>(&xyz_f,'f',"xyz_f","xyz_f1", std::map<std::string, xyz_f_t>{
                                                     {"xyz_f1",xyz_f1},
						     {"xyz_f2",xyz_f2} }));
      add(new CLIF_Option<dotProduct_f_t>(&dotProduct,'f',"dotProduct","2acc", std::map<std::string, dotProduct_f_t>{
                                                     {"ref",dotProduct_ref},
                                                     {"2acc",dotProduct_2acc},
                                                     {"auto1",dotProduct_auto1},
                                                     {"auto2",dotProduct_auto2},
						     {"vec1",dotProduct_vec1} }));
      
      add(new CLIF_Option<vectorNorm_f_t>(&vectorNorm,'f',"vectorNorm","ref", std::map<std::string, vectorNorm_f_t>{
						     {"ref",vectorNorm_ref} }));

      add(new CLIF_Option<walk_f_t>(&walk_f,'f',"walk_f","walk_ref", std::map<std::string, walk_f_t>{
                                                     {"walk_ref",walk_ref},
						     {"walkCoord_ref",walkCoord_ref} }));

      add(new CLIF_Option<intersectCoord_f_t>(&Polytope_T.intersectCoord,'f',"Polytope_intersectCoord","cached_ref", std::map<std::string, intersectCoord_f_t>{
                                                     {"ref",Polytope_intersectCoord_ref},
						     {"cached_ref",Polytope_intersectCoord_cached_ref} }));

      add(new CLIF_Option<intersectCoord_f_t>(&PolytopeT_T.intersectCoord,'f',"PolytopeT_intersectCoord","cached_ref", std::map<std::string, intersectCoord_f_t>{
                                                     {"ref",PolytopeT_intersectCoord_ref},
						     {"cached_nc1",PolytopeT_intersectCoord_cached_nc1},
						     {"cached_ref",PolytopeT_intersectCoord_cached_ref} }));


      // number parameters:
      claimOpt('c',"Algorithm Constants");
      add(new CLIF_OptionNumber<int>(&step_size,'c',"step_size","100000", 100, 1e7));
      add(new CLIF_OptionNumber<int>(&walk_size,'c',"walk_size","1", 1, 1e6));
   }
};




/**
 *\brief this function allow creating a random polytope around an ellipsoid given by ell
 * \param ell an n vector standing for a diagonal matrix (note dimension n is implicit in length of ell)
 * \param m the number of constraints
 * \param ret the return polytope
**/
void make_random_poly(const std::vector<double> &ell, int m, Polytope **ret);


/**
 * \brief read in one of the example polytopes defined in polyvest/examples
 * \param filename the relative path from root directory, something like polyvest/examples/simplex_10
 * \param P a *P will be filled with the example polytope from file
 **/
int read_polyvest_p(std::string filename, Polytope **P);


/**
 * \brief convert from Polytope to Polyvest_p
 * \param P the given polytope
 * \param Q is filled with content equivalend to P
 **/
void polyvest_convert(Polytope *P, vol::Polyvest_p *Q);


#endif // HEADER_VOLUME_HELPER_HPP



