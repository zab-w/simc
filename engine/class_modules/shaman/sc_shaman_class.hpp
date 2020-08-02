#include "simulationcraft.hpp"
#include "sc_shaman.hpp"

#ifndef SC_SHAMAN_CLASS
#define SC_SHAMAN_CLASS

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

// ==========================================================================
// Shaman Attack
// ==========================================================================

struct shaman_attack_t : public virtual shaman_action_t<melee_attack_t>
{
public:
    using ab = shaman_action_t<melee_attack_t>;

public:
  
  bool may_proc_lightning_shield;
  proc_t* proc_ls;

  shaman_attack_t( const std::string& token, shaman_t* p, const spell_data_t* s )
    : ab( token, p, s ), 
      may_proc_lightning_shield( false )
  {
    special    = true;
    may_glance = false;
  }

  

  // need to roll MW gain proc and add stack
  // virtual double maelstrom_weapon_energize_amount( const action_state_t* /* source */ ) const
  //{
  //  return p()->spell.maelstrom_melee_gain->effectN( 1 ).resource( RESOURCE_MAELSTROM );
  //}

  void impact( action_state_t* state ) override
  {
    base_t::impact( state );

    // Bail out early if the result is a miss/dodge/parry/ms
    if ( !result_is_hit( state->result ) )
      return;

    p()->trigger_windfury_weapon( state );
    p()->trigger_flametongue_weapon( state );
    p()->trigger_lightning_shield( state );
    p()->trigger_hot_hand( state );
    p()->trigger_icy_edge( state );
  }

  
};

// ==========================================================================
// Shaman Offensive Spell
// ==========================================================================

struct shaman_spell_t : public virtual shaman_spell_base_t<spell_t>
{
  action_t* overload;
  private:
    using action = shaman_action_t<spell_t>;
    using ab = shaman_spell_base_t;

  public:
      bool affected_by_master_of_the_elements = false;
      bool affected_by_stormkeeper            = false;

    shaman_spell_t( const std::string& token, shaman_t* p, const spell_data_t* s = spell_data_t::nil(),
                    const std::string& options = std::string() )
      : action(token, p, s),
        ab( token, p, s ),
        overload( nullptr )
    {
      parse_options( options );

      if ( data().affected_by( p->spec.elemental_fury->effectN( 1 ) ) )
      {
        crit_bonus_multiplier *= 1.0 + p->spec.elemental_fury->effectN( 1 ).percent();
      }

      if ( data().affected_by( p->find_spell( 260734 )->effectN( 1 ) ) )
      {
        affected_by_master_of_the_elements = true;
      }

      //Move to specs spell
      if ( data().affected_by( p->find_spell( 191634 )->effectN( 1 ) ) )
      {
        affected_by_stormkeeper = true;
      }
    }
  
    void init_finished() override
    {
      base_t::init_finished();
    }
  
    //double action_multiplier() const override
    //{
    //  double m = ab::action_multiplier();
    //  // BfA Elemental talent - Master of the Elements
    //  if ( affected_by_master_of_the_elements )
    //  {
    //    m *= 1.0 + p()->buff.master_of_the_elements->value();
    //  }
    //  return m;
    //}

    double composite_spell_power() const override
    {
      double sp = base_t::composite_spell_power();

      return sp;
    }

    void execute() override
    {
      ab::execute();

      if ( p()->talent.earthen_rage->ok() && !background /*&& execute_state->action->harmful*/ )
      {
        p()->recent_target = execute_state->target;
        p()->buff.earthen_rage->trigger();
      }

      // BfA Elemental talent - Master of the Elements
      if ( affected_by_master_of_the_elements && !background )
      {
        p()->buff.master_of_the_elements->decrement();
      }
    }

    void schedule_travel( action_state_t* s ) override
    {
      trigger_elemental_overload( s );

      base_t::schedule_travel( s );
    }

    bool usable_moving() const override
    {
      if ( p()->buff.spiritwalkers_grace->check() || execute_time() == timespan_t::zero() )
        return true;

      return base_t::usable_moving();
    }

    virtual double overload_chance( const action_state_t* ) const
    {
      return p()->cache.mastery_value();
    }

    // Additional guaranteed overloads
    virtual size_t n_overloads( const action_state_t* ) const
    {
      return 0;
    }

    // Additional overload chances
    virtual size_t n_overload_chances( const action_state_t* ) const
    {
      return 0;
    }

    void trigger_elemental_overload( const action_state_t* source_state ) const
    {
      struct elemental_overload_event_t : public event_t
      {
        action_state_t* state;

        elemental_overload_event_t( action_state_t* s )
          : event_t( *s->action->player, timespan_t::from_millis( 400 ) ), state( s )
        {
        }

        ~elemental_overload_event_t() override
        {
          if ( state )
            action_state_t::release( state );
        }

        const char* name() const override
        {
          return "elemental_overload_event_t";
        }

        void execute() override
        {
          state->action->schedule_execute( state );
          state = nullptr;
        }
      };

      if ( !p()->mastery.elemental_overload->ok() )
      {
        return;
      }

      if ( !overload )
      {
        return;
      }

      /* Hacky to recreate ingame behavior. Stormkeeper forces only the first overload to happen. */
      unsigned overloads = rng().roll( overload_chance( source_state ) );

      if ( p()->buff.stormkeeper->up() && affected_by_stormkeeper )
      {
        overloads = 1;
      }

      overloads += (unsigned)n_overloads( source_state );

      for ( size_t i = 0, end = overloads; i < end; ++i )
      {
        action_state_t* s = overload->get_state();
        overload->snapshot_state( s, result_amount_type::DMG_DIRECT );
        s->target = source_state->target;

        make_event<elemental_overload_event_t>( *sim, s );
      }
    }

    void impact( action_state_t* state ) override
    {
      base_t::impact( state );

      // p()->trigger_stormbringer( state );
    }

    //Move to enhance
    /*virtual double stormbringer_proc_chance() const
    {
      double base_chance = 0;

      base_chance += p()->spec.stormbringer->proc_chance() +
                     p()->cache.mastery() * p()->mastery.enhanced_elements->effectN( 3 ).mastery_value();

      return base_chance;
    }*/


};


// ==========================================================================
// Shaman Heal
// ==========================================================================

struct shaman_heal_t : public shaman_spell_base_t<heal_t>
{
private:
    using action = shaman_action_t<heal_t>;
    using ab = shaman_spell_base_t;

  double elw_proc_high, elw_proc_low, resurgence_gain;

  bool proc_tidal_waves, consume_tidal_waves;

    shaman_heal_t( const std::string& token, shaman_t* p, const spell_data_t* s = spell_data_t::nil(),
                      const std::string& options = std::string() )
    : action( token, p, s ),
      ab( token, p, s ),      
      elw_proc_high( .2 ),
      elw_proc_low( 1.0 ),
      resurgence_gain( 0 ),
      proc_tidal_waves( false ),
      consume_tidal_waves( false )
  {
    parse_options( options );
  }

  shaman_heal_t( shaman_t* p, const spell_data_t* s = spell_data_t::nil(),
                      const std::string& options = std::string() )
    : action( "", p, s ),
      ab( "", p, s ),
      elw_proc_high( .2 ),
      elw_proc_low( 1.0 ),
      resurgence_gain( 0 ),
      proc_tidal_waves( false ),
      consume_tidal_waves( false )
  {
    parse_options( options );
  }

  double composite_spell_power() const override
  {
    double sp = base_t::composite_spell_power();

    if ( p()->main_hand_weapon.buff_type == EARTHLIVING_IMBUE )
      sp += p()->main_hand_weapon.buff_value;

    return sp;
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = base_t::composite_da_multiplier( state );
    m *= 1.0 + p()->spec.purification->effectN( 1 ).percent();
    return m;
  }

  double composite_ta_multiplier( const action_state_t* state ) const override
  {
    double m = base_t::composite_ta_multiplier( state );
    m *= 1.0 + p()->spec.purification->effectN( 1 ).percent();
    return m;
  }

  double composite_target_multiplier( player_t* target ) const override
  {
    double m = base_t::composite_target_multiplier( target );
    return m;
  }

  void impact( action_state_t* s ) override;

  void execute() override
  {
    base_t::execute();

    if ( consume_tidal_waves )
      p()->buff.tidal_waves->decrement();
  }

  virtual double deep_healing( const action_state_t* s )
  {
    if ( !p()->mastery.deep_healing->ok() )
      return 0.0;

    double hpp = ( 1.0 - s->target->health_percentage() / 100.0 );

    return 1.0 + hpp * p()->cache.mastery_value();
  }
};


}  // namespace shaman

#endif /* SC_SHAMAN_CLASS */