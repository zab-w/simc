#include "simulationcraft.hpp"
#include "sc_shaman.hpp"

namespace shaman
{
static std::vector<player_t*>& __check_distance_targeting( const action_t* action, std::vector<player_t*>& tl )
{
  sim_t* sim = action->sim;
  if ( !sim->distance_targeting_enabled )
  {
    return tl;
  }

  player_t* target = action->target;
  player_t* player = action->player;
  double radius    = action->radius;
  int aoe          = action->aoe;

  player_t* last_chain;  // We have to track the last target that it hit.
  last_chain = target;
  std::vector<player_t*>
      best_so_far;  // Keeps track of the best chain path found so far, so we can use it if we give up.
  std::vector<player_t*> current_attempt;
  best_so_far.push_back( last_chain );
  current_attempt.push_back( last_chain );

  size_t num_targets  = sim->target_non_sleeping_list.size();
  size_t max_attempts = static_cast<size_t>(
      std::min( ( num_targets - 1.0 ) * 2.0, 30.0 ) );  // With a lot of targets this can get pretty high. Cap it at 30.
  size_t local_attempts = 0, attempts = 0, chain_number = 1;
  std::vector<player_t*> targets_left_to_try(
      sim->target_non_sleeping_list.data() );  // This list contains members of a vector that haven't been tried yet.
  auto position = std::find( targets_left_to_try.begin(), targets_left_to_try.end(), target );
  if ( position != targets_left_to_try.end() )
    targets_left_to_try.erase( position );

  std::vector<player_t*> original_targets(
      targets_left_to_try );  // This is just so we don't have to constantly remove the original target.

  bool stop_trying = false;

  while ( !stop_trying )
  {
    local_attempts = 0;
    attempts++;
    if ( attempts >= max_attempts )
      stop_trying = true;
    while ( targets_left_to_try.size() > 0 && local_attempts < num_targets * 2 )
    {
      player_t* possibletarget;
      size_t rng_target = static_cast<size_t>(
          sim->rng().range( 0.0, ( static_cast<double>( targets_left_to_try.size() ) - 0.000001 ) ) );
      possibletarget = targets_left_to_try[ rng_target ];

      double distance_from_last_chain = last_chain->get_player_distance( *possibletarget );
      if ( distance_from_last_chain <= radius + possibletarget->combat_reach )
      {
        last_chain = possibletarget;
        current_attempt.push_back( last_chain );
        targets_left_to_try.erase( targets_left_to_try.begin() + rng_target );
        chain_number++;
      }
      else
      {
        // If there is no hope of this target being chained to, there's no need to test it again
        // for other possibilities.
        if ( distance_from_last_chain > ( ( radius + possibletarget->combat_reach ) * ( aoe - chain_number ) ) )
          targets_left_to_try.erase( targets_left_to_try.begin() + rng_target );
        local_attempts++;  // Only count failures towards the limit-cap.
      }
      // If we run out of targets to hit, or have hit 5 already. Break.
      if ( static_cast<int>( current_attempt.size() ) == aoe || current_attempt.size() == num_targets )
      {
        stop_trying = true;
        break;
      }
    }
    if ( current_attempt.size() > best_so_far.size() )
      best_so_far = current_attempt;

    current_attempt.clear();
    current_attempt.push_back( target );
    last_chain          = target;
    targets_left_to_try = original_targets;
    chain_number        = 1;
  }

  if ( sim->log )
    sim->out_debug.printf( "%s Total attempts at finding path: %.3f - %.3f targets found - %s target is first chain",
                           player->name(), static_cast<double>( attempts ), static_cast<double>( best_so_far.size() ),
                           target->name() );
  tl.swap( best_so_far );
  return tl;
}

// ==========================================================================
// Shaman Custom Buff Declaration
// ==========================================================================
//


struct ascendance_buff_t : public buff_t
{
  action_t* lava_burst;  // move to ele

  ascendance_buff_t( shaman_t* p )
    : buff_t( p, "ascendance",
              p->specialization() == SHAMAN_ENHANCEMENT ? p->find_spell( 114051 )
                                                        : p->find_spell( 114050 ) ),  // No resto for now
      lava_burst( nullptr )
  {
    set_trigger_spell( p->talent.ascendance );
    set_tick_callback( [ p ]( buff_t* b, int, timespan_t ) { double g = b->data().effectN( 4 ).base_value(); } );
    set_cooldown( timespan_t::zero() );  // Cooldown is handled by the action
  }
};





}  // namespace shaman