#include "simulationcraft.hpp"
#include "sc_shaman.hpp"

#ifndef SC_SHAMAN_CLASS
#define SC_SHAMAN_CLASS

namespace shaman
{

// ==========================================================================
// Shaman Custom Buff Declaration
// ==========================================================================
//


//struct ascendance_buff_t : public buff_t
//{
//  action_t* lava_burst;  // move to ele
//
//  ascendance_buff_t( shaman_t* p )
//    : buff_t( p, "ascendance",
//              p->specialization() == SHAMAN_ENHANCEMENT ? p->find_spell( 114051 )
//                                                        : p->find_spell( 114050 ) ),  // No resto for now
//      lava_burst( nullptr )
//  {
//    set_trigger_spell( p->talent.ascendance );
//    set_tick_callback( [ p ]( buff_t* b, int, timespan_t ) { double g = b->data().effectN( 4 ).base_value(); } );
//    set_cooldown( timespan_t::zero() );  // Cooldown is handled by the action
//  }
//};

shaman_td_t::shaman_td_t( player_t* target, shaman_t* p ) : actor_target_data_t( target, p )
{
  // Shared
  dot.flame_shock = target->get_dot( "flame_shock", p );

  // Elemental
};




}  // namespace shaman

#endif /* SC_SHAMAN_CLASS */