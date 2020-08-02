#pragma once
#include "simulationcraft.hpp"
#include "player/pet_spawner.hpp"

#ifndef SC_SHAMAN
#define SC_SHAMAN

namespace shaman
{
typedef std::pair<std::string, simple_sample_data_with_min_max_t> data_t;
typedef std::pair<std::string, simple_sample_data_t> simple_data_t;
struct shaman_t;

enum totem_e
{
  TOTEM_NONE = 0,
  TOTEM_AIR,
  TOTEM_EARTH,
  TOTEM_FIRE,
  TOTEM_WATER,
  TOTEM_MAX
};

enum wolf_type_e
{
  SPIRIT_WOLF = 0,
  FIRE_WOLF,
  FROST_WOLF,
  LIGHTNING_WOLF
};

enum class elemental
{
  FIRE,
  EARTH,
  STORM,
};

enum imbue_e
{
  IMBUE_NONE = 0,
  FLAMETONGUE_IMBUE,
  WINDFURY_IMBUE,
  FROSTBRAND_IMBUE,
  EARTHLIVING_IMBUE
};

struct shaman_attack_t;
struct shaman_spell_t;
struct shaman_heal_t;
struct shaman_totem_pet_t;
struct totem_pulse_event_t;
struct totem_pulse_action_t;

struct shaman_td_t : public actor_target_data_t
{
  struct dots
  {
    dot_t* flame_shock;
  } dot;

  struct debuffs
  {
    // Elemental
  } debuff;

  struct heals
  {
    dot_t* riptide;
    dot_t* earthliving;
  } heal;
  
  shaman_td_t( player_t* target, shaman_t* p );

  shaman_t* actor() const
  {
    return debug_cast<shaman_t*>( source );
  }
};

struct counter_t
{
  const sim_t* sim;

  double value, interval;
  timespan_t last;

  counter_t( shaman_t* p );

  void add( double val )
  {
    // Skip iteration 0 for non-debug, non-log sims
    if ( sim->current_iteration == 0 && sim->iterations > sim->threads && !sim->debug && !sim->log )
      return;

    value += val;
    if ( last > timespan_t::min() )
      interval += ( sim->current_time() - last ).total_seconds();
    last = sim->current_time();
  }

  void reset()
  {
    last = timespan_t::min();
  }

  double divisor() const
  {
    if ( !sim->debug && !sim->log && sim->iterations > sim->threads )
      return sim->iterations - sim->threads;
    else
      return std::min( sim->iterations, sim->threads );
  }

  double mean() const
  {
    return value / divisor();
  }

  double interval_mean() const
  {
    return interval / divisor();
  }

  void merge( const counter_t& other )
  {
    value += other.value;
    interval += other.interval;
  }
};


struct shaman_t : public player_t
{
public:
  // Misc
  bool lava_surge_during_lvb; //to ele
  std::vector<counter_t*> counters;
  /// Shaman ability cooldowns
  std::vector<cooldown_t*> ability_cooldowns;
  player_t* recent_target =
      nullptr;  // required for Earthen Rage, whichs' ticks damage the most recently attacked target


  // Data collection for cooldown waste
  auto_dispose<std::vector<data_t*> > cd_waste_exec, cd_waste_cumulative;
  auto_dispose<std::vector<simple_data_t*> > cd_waste_iter;

  // Cached actions
  struct actions_t
  {
    spell_t* lightning_shield; 
    spell_t* earthen_rage; 
  } action;

  // Pets
  struct pets_t
  {
    pet_t* pet_fire_elemental;
    pet_t* pet_storm_elemental;
    pet_t* pet_earth_elemental;

    pet_t* guardian_fire_elemental;
    pet_t* guardian_storm_elemental;
    pet_t* guardian_earth_elemental;

    // spawner::pet_spawner_t<pet_t, shaman_t> ember_elemental;
    // spawner::pet_spawner_t<pet_t, shaman_t> spark_elemental;

    pets_t( shaman_t* );
  } pet;

  // Constants
  struct
  {
    double matching_gear_multiplier;
  } constant;

  // Buffs
  struct buffs_t
  {
    // shared between all three specs
    buff_t* ascendance;
    buff_t* ghost_wolf;

    // Elemental, Restoration
    buff_t* lava_surge;

    // Elemental
    buff_t* earthen_rage;
    buff_t* master_of_the_elements;
    buff_t* surge_of_power;
    buff_t* icefury;
    buff_t* unlimited_power;
    buff_t* stormkeeper;
    stat_buff_t* elemental_blast_crit;
    stat_buff_t* elemental_blast_haste;
    stat_buff_t* elemental_blast_mastery;
    buff_t* wind_gust;  // Storm Elemental passive 263806

    // Restoration
    buff_t* spirit_walk;
    buff_t* spiritwalkers_grace;
    buff_t* tidal_waves;

    // PvP
    buff_t* thundercharge;

  } buff;

  // Cooldowns
  struct cooldowns_t
  {
    cooldown_t* ascendance;
    cooldown_t* fire_elemental;
    cooldown_t* feral_spirits;
    cooldown_t* lava_burst;
    cooldown_t* storm_elemental;
    cooldown_t* strike;  // shared CD of Storm Strike and Windstrike
  } cooldown;

  // Gains
  struct gains_t
  {
    gain_t* aftershock;
    gain_t* resurgence;
    gain_t* fire_elemental;
    gain_t* spirit_of_the_maelstrom;
  } gain;

  // Tracked Procs
  struct procs_t
  {
    // Elemental, Restoration
    proc_t* lava_surge;
    proc_t* wasted_lava_surge;
    proc_t* surge_during_lvb;
  } proc;

  // Class Specializations
  struct specializations_t
  {
    //Specs
    const spell_data_t* enhancement_shaman;
    const spell_data_t* elemental_shaman;
    const spell_data_t* restoration_shaman;  

    // Generic
    const spell_data_t* mail_specialization;
    const spell_data_t* shaman;

    // Elemental
    const spell_data_t* chain_lightning_2;  // 7.1 Chain Lightning additional 2 targets passive
    const spell_data_t* elemental_fury;     // general crit multiplier
    const spell_data_t* lava_burst_2;       // 7.1 Lava Burst autocrit with FS passive
    const spell_data_t* lava_surge;

    // Restoration
    const spell_data_t* purification;
    const spell_data_t* resurgence;
    const spell_data_t* riptide;
    const spell_data_t* tidal_waves;
    const spell_data_t* spiritwalkers_grace;
  } spec;

  // Masteries
  struct
  {
    const spell_data_t* elemental_overload;
    const spell_data_t* deep_healing;
  } mastery;

  // Talents
  struct talents_t
  {
    // Generic / Shared
    const spell_data_t* elemental_blast;
    const spell_data_t* totem_mastery;
    const spell_data_t* spirit_wolf;
    const spell_data_t* earth_shield;
    const spell_data_t* static_charge;
    const spell_data_t* natures_guardian;
    const spell_data_t* wind_rush_totem;
    const spell_data_t* stormkeeper;
    const spell_data_t* ascendance;

    // Elemental
    const spell_data_t* earthen_rage;
    const spell_data_t* echo_of_the_elements;
    // elemental blast - shared

    const spell_data_t* aftershock;
    // echoing shock
    // totem mastery - shared

    // spirit wolf - shared
    // earth shield - shared
    // static charge - shared

    const spell_data_t* master_of_the_elements;
    const spell_data_t* storm_elemental;
    const spell_data_t* liquid_magma_totem;

    // natures guardian - shared
    const spell_data_t* ancestral_guidance;
    // wind rush totem - shared

    const spell_data_t* surge_of_power;
    const spell_data_t* primal_elementalist;
    const spell_data_t* icefury;

    const spell_data_t* unlimited_power;
    // stormkeeper - shared
    // ascendance - shared

    // Restoration
    const spell_data_t* graceful_spirit;
  } talent;

  // Misc Spells
  struct misc_t
  {
    const spell_data_t* resurgence;
    const spell_data_t* fire_elemental;
    const spell_data_t* storm_elemental;
    const spell_data_t* earth_elemental;
  } spell;

  // Cached pointer for ascendance / normal white melee
  shaman_attack_t* melee_mh;
  shaman_attack_t* melee_oh;


  shaman_t( sim_t* sim, util::string_view name, race_e r = RACE_TAUREN )
    : player_t( sim, SHAMAN, name, r ),
      lava_surge_during_lvb( false ),
      action(),
      pet( this ),
      constant(),
      buff(),
      cooldown(),
      gain(),
      proc(),
      spec(),
      mastery(),
      talent(),
      spell()
  {
    /*
    range::fill( pet.spirit_wolves, nullptr );
    range::fill( pet.elemental_wolves, nullptr );
    */

    // Cooldowns
    cooldown.ascendance      = get_cooldown( "ascendance" );
    cooldown.fire_elemental  = get_cooldown( "fire_elemental" );
    cooldown.storm_elemental = get_cooldown( "storm_elemental" );
    cooldown.lava_burst      = get_cooldown( "lava_burst" );
    

    melee_mh      = nullptr;
    melee_oh      = nullptr;

    if ( specialization() == SHAMAN_ELEMENTAL)
      resource_regeneration = regen_type::DISABLED;
    else
      resource_regeneration = regen_type::DYNAMIC;
  }

  ~shaman_t() override;

  // Misc
  bool active_elemental_pet() const;
  void summon_fire_elemental( timespan_t duration, bool essence_proc );
  void summon_storm_elemental( timespan_t duration, bool essence_proc );

  // triggers
  void trigger_maelstrom_gain( double base, gain_t* gain = nullptr );
  void trigger_windfury_weapon( const action_state_t* );
  void trigger_flametongue_weapon( const action_state_t* );
  void trigger_icy_edge( const action_state_t* );
  void trigger_stormbringer( const action_state_t* state, double proc_chance = -1.0, proc_t* proc_obj = nullptr );
  void trigger_lightning_shield( const action_state_t* state );
  void trigger_hot_hand( const action_state_t* state );

  // Legendary
  // empty - for now

  // Character Definition
  void init_spells() override;
  void init_base_stats() override;
  void init_scaling() override;
  void create_buffs() override;
  void create_actions() override;
  void create_options() override;
  void init_gains() override;
  void init_procs() override;

  // APL releated methods
  void init_action_list() override;
  void init_action_list_enhancement();
  void init_action_list_elemental();
  void init_action_list_restoration_dps();
  std::string generate_bloodlust_options();
  std::string default_potion() const override;
  std::string default_flask() const override;
  std::string default_food() const override;
  std::string default_rune() const override;

  void init_rng() override;
  void init_special_effects() override;

  double resource_loss( resource_e resource_type, double amount, gain_t* g = nullptr, action_t* a = nullptr ) override;
  void moving() override;
  void invalidate_cache( cache_e c ) override;
  double temporary_movement_modifier() const override;
  double passive_movement_modifier() const override;
  double composite_melee_crit_chance() const override;
  double composite_melee_haste() const override;
  double composite_melee_speed() const override;
  double composite_attack_power_multiplier() const override;
  double composite_attribute_multiplier( attribute_e ) const override;
  double composite_spell_crit_chance() const override;
  double composite_spell_haste() const override;
  double composite_spell_power( school_e school ) const override;
  double composite_spell_power_multiplier() const override;
  double composite_player_multiplier( school_e school ) const override;
  double composite_player_target_multiplier( player_t* target, school_e school ) const override;
  double composite_player_pet_damage_multiplier( const action_state_t* state ) const override;
  double composite_maelstrom_gain_coefficient( const action_state_t* state = nullptr ) const;
  double matching_gear_multiplier( attribute_e attr ) const override;
  action_t* create_action( const std::string& name, const std::string& options ) override;
  pet_t* create_pet( const std::string& name, const std::string& type = std::string() ) override;
  void create_pets() override;
  std::unique_ptr<expr_t> create_expression( const std::string& name ) override;
  resource_e primary_resource() const override
  {
    return RESOURCE_MANA;
  }
  role_e primary_role() const override;
  stat_e convert_hybrid_stat( stat_e s ) const override;
  void arise() override;
  void combat_begin() override;
  void reset() override;
  void merge( player_t& other ) override;
  void copy_from( player_t* ) override;

  void datacollection_begin() override;
  void datacollection_end() override;

  target_specific_t<shaman_td_t> target_data;

  shaman_td_t* get_target_data( player_t* target ) const override
  {
    shaman_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new shaman_td_t( target, const_cast<shaman_t*>( this ) );
    }
    return td;
  }

  // Helper to trigger a secondary ability through action scheduling (i.e., schedule_execute()),
  // without breaking targeting information. Note, causes overhead as an extra action_state_t object
  // is needed to carry the information.
  void trigger_secondary_ability( const action_state_t* source_state, action_t* secondary_action,
                                  bool inherit_state = false );

  template <typename T_CONTAINER, typename T_DATA>
  T_CONTAINER* get_data_entry( const std::string& name, std::vector<T_DATA*>& entries )
  {
    for ( size_t i = 0; i < entries.size(); i++ )
    {
      if ( entries[ i ]->first == name )
      {
        return &( entries[ i ]->second );
      }
    }

    entries.push_back( new T_DATA( name, T_CONTAINER() ) );
    return &( entries.back()->second );
  }
};

shaman_t::~shaman_t()
{
  range::dispose( counters );
}

counter_t::counter_t( shaman_t* p ) : sim( p->sim ), value( 0 ), interval( 0 ), last( timespan_t::min() )
{
  p->counters.push_back( this );
}


// ==========================================================================
// Shaman Custom Buff Declaration
// ==========================================================================
//

//struct ascendance_buff_t : public buff_t
//{
//  ascendance_buff_t( shaman_t* p );
//  void ascendance( attack_t* mh, attack_t* oh );
//  bool trigger( int stacks, double value, double chance, timespan_t duration ) override;
//  void expire_override( int expiration_stacks, timespan_t remaining_duration ) override;
//};

//shaman_td_t::shaman_td_t( player_t* target, shaman_t* p ) : actor_target_data_t( target, p )
//{
//  
//};

// ==========================================================================
// Shaman Action Base Template
// ==========================================================================

//Some base implementations, begrudginly, have to go in here because template reasons.
template <class Base>
struct shaman_action_t : public Base
{
private:
  using ab = Base;  // action base, eg. spell_t
public:
  using base_t = shaman_action_t<Base>;

  // Cooldown tracking
  bool track_cd_waste;
  simple_sample_data_with_min_max_t *cd_wasted_exec, *cd_wasted_cumulative;
  simple_sample_data_t* cd_wasted_iter;

  // Ghost wolf unshift
  bool unshift_ghost_wolf;

  // Maelstrom stuff
  gain_t* gain;
  double maelstrom_gain;
  double maelstrom_gain_coefficient;
  bool enable_enh_mastery_scaling;

  bool affected_by_molten_weapon;

  shaman_action_t( const std::string& n, shaman_t* player, const spell_data_t* s = spell_data_t::nil() )
    : ab( n, player, s )
  {
    ab::may_crit = true;

    // Auto-parse maelstrom gain from energize
    for ( size_t i = 1; i <= ab::data().effect_count(); i++ )
    {
      const spelleffect_data_t& effect = ab::data().effectN( i );
      if ( effect.type() != E_ENERGIZE || static_cast<power_e>( effect.misc_value1() ) != POWER_MAELSTROM )
      {
        continue;
      }

      maelstrom_gain    = effect.resource( RESOURCE_MAELSTROM );
      ab::energize_type = ENERGIZE_NONE;  // disable resource generation from spell data.
    }

    //Elemental
    if ( ab::data().affected_by( player->spec.elemental_shaman->effectN( 1 ) ) )
    {
      ab::base_dd_multiplier *= 1.0 + player->spec.elemental_shaman->effectN( 1 ).percent();
    }
    if ( ab::data().affected_by( player->spec.elemental_shaman->effectN( 2 ) ) )
    {
      ab::base_td_multiplier *= 1.0 + player->spec.elemental_shaman->effectN( 2 ).percent();
    }

    //Restoration
    if ( ab::data().affected_by( player->spec.restoration_shaman->effectN( 4 ) ) )
    {
      ab::base_td_multiplier *= 1.0 + player->spec.restoration_shaman->effectN( 4 ).percent();
    }
    if ( ab::data().affected_by( player->spec.restoration_shaman->effectN( 7 ) ) )
    {
      ab::base_multiplier *= 1.0 + player->spec.restoration_shaman->effectN( 7 ).percent();
    }
  }

  std::string full_name() const
  {
    std::string n = ab::data().name_cstr();
    return n.empty() ? ab::name_str : n;
  }

  void init() override
  {
    ab::init();

    if ( track_cd_waste )
    {
      cd_wasted_exec =
          p()->template get_data_entry<simple_sample_data_with_min_max_t, data_t>( ab::name_str, p()->cd_waste_exec );
      cd_wasted_cumulative = p()->template get_data_entry<simple_sample_data_with_min_max_t, data_t>(
          ab::name_str, p()->cd_waste_cumulative );
      cd_wasted_iter =
          p()->template get_data_entry<simple_sample_data_t, simple_data_t>( ab::name_str, p()->cd_waste_iter );
    }
  }

  void init_finished() override
  {
    ab::init_finished();

    if ( this->cooldown->duration > timespan_t::zero() )
    {
      p()->ability_cooldowns.push_back( this->cooldown );
    }
  }

  double composite_attack_power() const override
  {
    double m = ab::composite_attack_power();

    return m;
  }

  double recharge_multiplier( const cooldown_t& cd ) const override
  {
    double m = ab::recharge_multiplier( cd );

    m *= 1.0 / ( 1.0 + p()->buff.thundercharge->stack_value() );

    // TODO: Current presumption is self-cast, giving multiplicative effect
    m *= 1.0 / ( 1.0 + p()->buff.thundercharge->stack_value() );

    return m;
  }

  double action_multiplier() const override
  {
    double m = ab::action_multiplier();

    return m;
  }

  virtual shaman_t* p()
  {
    return debug_cast<shaman_t*>( ab::player );
  }

  virtual const shaman_t* p() const
  {
    return debug_cast<shaman_t*>( ab::player );
  }

  shaman_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Move to enhance, convert to MW chance on actions
  virtual double composite_maelstrom_gain_coefficient( const action_state_t* state = nullptr ) const
  {
    double m = maelstrom_gain_coefficient;

    m *= p()->composite_maelstrom_gain_coefficient( state );

    return m;
  }

  void execute() override
  {
    ab::execute();

    trigger_maelstrom_gain( ab::execute_state );
  }

  void tick( dot_t* d ) override
  {
    ab::tick( d );
  }

  void impact( action_state_t* state ) override
  {
    ab::impact( state );

    p()->trigger_stormbringer( state );
  }

  void schedule_execute( action_state_t* execute_state = nullptr ) override
  {
    if ( !ab::background && unshift_ghost_wolf )
    {
      p()->buff.ghost_wolf->expire();
    }

    ab::schedule_execute( execute_state );
  }

  void update_ready( timespan_t cd ) override
  {
    if ( cd_wasted_exec &&
         ( cd > timespan_t::zero() || ( cd <= timespan_t::zero() && ab::cooldown->duration > timespan_t::zero() ) ) &&
         ab::cooldown->current_charge == ab::cooldown->charges && ab::cooldown->last_charged > timespan_t::zero() &&
         ab::cooldown->last_charged < ab::sim->current_time() )
    {
      double time_ = ( ab::sim->current_time() - ab::cooldown->last_charged ).total_seconds();
      if ( p()->sim->debug )
      {
        p()->sim->out_debug.printf( "%s %s cooldown waste tracking waste=%.3f exec_time=%.3f", p()->name(), ab::name(),
                                    time_, ab::time_to_execute.total_seconds() );
      }
      time_ -= ab::time_to_execute.total_seconds();

      if ( time_ > 0 )
      {
        cd_wasted_exec->add( time_ );
        cd_wasted_iter->add( time_ );
      }
    }

    ab::update_ready( cd );
  }

  std::unique_ptr<expr_t> create_expression( const std::string& name ) override
  {
    if ( !util::str_compare_ci( name, "cooldown.higher_priority.min_remains" ) )
      return ab::create_expression( name );

    struct hprio_cd_min_remains_expr_t : public expr_t
    {
      action_t* action_;
      std::vector<cooldown_t*> cd_;

      // TODO: Line_cd support
      hprio_cd_min_remains_expr_t( action_t* a ) : expr_t( "min_remains" ), action_( a )
      {
        action_priority_list_t* list = a->player->get_action_priority_list( a->action_list->name_str );
        for ( auto list_action : list->foreground_action_list )
        {
          // Jump out when we reach this action
          if ( list_action == action_ )
            break;

          // Skip if this action's cooldown is the same as the list action's cooldown
          if ( list_action->cooldown == action_->cooldown )
            continue;

          // Skip actions with no cooldown
          if ( list_action->cooldown && list_action->cooldown->duration == timespan_t::zero() )
            continue;

          // Skip cooldowns that are already accounted for
          if ( std::find( cd_.begin(), cd_.end(), list_action->cooldown ) != cd_.end() )
            continue;

          // std::cout << "Appending " << list_action -> name() << " to check list" << std::endl;
          cd_.push_back( list_action->cooldown );
        }
      }

      double evaluate() override
      {
        if ( cd_.size() == 0 )
          return 0;

        timespan_t min_cd = cd_[ 0 ]->remains();
        for ( size_t i = 1, end = cd_.size(); i < end; i++ )
        {
          timespan_t remains = cd_[ i ]->remains();
          // std::cout << "cooldown.higher_priority.min_remains " << cd_[ i ] -> name_str << " remains=" <<
          // remains.total_seconds() << std::endl;
          if ( remains < min_cd )
            min_cd = remains;
        }

        // std::cout << "cooldown.higher_priority.min_remains=" << min_cd.total_seconds() << std::endl;
        return min_cd.total_seconds();
      }
    };

    return std::make_unique<hprio_cd_min_remains_expr_t>( this );
  }

  virtual void trigger_maelstrom_gain( const action_state_t* state )
  {
    if ( maelstrom_gain == 0 )
    {
      return;
    }

    double g = maelstrom_gain;
    g *= composite_maelstrom_gain_coefficient( state );
    // TODO: Some sort of selector whether it's per cast or per target. Per target is the "default".
    g *= state->n_targets;
    ab::player->resource_gain( RESOURCE_MAELSTROM, g, gain, this );
  }
};

// ==========================================================================
// Shaman Base Spell
// ==========================================================================

// Base implementations, begrudginly, have to go in here because template reasons.
template <class Base>
struct shaman_spell_base_t : public virtual shaman_action_t<Base>
{
private:
  using ab = shaman_action_t<Base>;

public:
  using base_t = shaman_spell_base_t<Base>;

  shaman_spell_base_t( const std::string& n, shaman_t* player, const spell_data_t* s = spell_data_t::nil() )
    : ab( n, player, s ) { }

  void execute() override
  {
    ab::execute();

    // for benefit tracking purpose
    ab::p()->buff.spiritwalkers_grace->up();

    if ( ab::p()->talent.aftershock->ok() && ab::current_resource() == RESOURCE_MAELSTROM &&
         ab::last_resource_cost > 0 && ab::rng().roll( ab::p()->talent.aftershock->effectN( 1 ).percent() ) )
    {
      ab::p()->trigger_maelstrom_gain( ab::last_resource_cost, ab::p()->gain.aftershock );
      expansion::bfa::trigger_leyshocks_grand_compilation( STAT_CRIT_RATING, ab::player );
    }
  }
};


//OKAY ALL OF THIS NEEDS TO GO INTO A PET HEADER AND NAMESPACE
namespace pet
{
// Simple helper to summon n (default 1) sleeping pet(s) from a container
template <typename T>
void summon( const T& container, timespan_t duration, size_t n = 1 )
{
  size_t summoned = 0;

  for ( size_t i = 0, end = container.size(); i < end; ++i )
  {
    auto ptr = container[ i ];
    if ( !ptr->is_sleeping() )
    {
      continue;
    }

    ptr->summon( duration );
    if ( ++summoned == n )
    {
      break;
    }
  }
}
// ==========================================================================
// Base Shaman Pet
// ==========================================================================

struct shaman_pet_t : public pet_t
{
  bool use_auto_attack;

  shaman_pet_t( shaman_t* owner, const std::string& name, bool guardian = true, bool auto_attack = true )
    : pet_t( owner->sim, owner, name, guardian ), use_auto_attack( auto_attack )
  {
    resource_regeneration = regen_type::DISABLED;

    main_hand_weapon.type       = WEAPON_BEAST;
    main_hand_weapon.swing_time = timespan_t::from_seconds( 2.0 );
  }

  shaman_t* o() const
  {
    return debug_cast<shaman_t*>( owner );
  }

  virtual void create_default_apl()
  {
    action_priority_list_t* def = get_action_priority_list( "default" );
    if ( use_auto_attack )
    {
      def->add_action( "auto_attack" );
    }
  }

  void init_action_list() override
  {
    pet_t::init_action_list();

    if ( action_list_str.empty() )
    {
      create_default_apl();
    }
  }

  action_t* create_action( const std::string& name, const std::string& options_str ) override;

  virtual attack_t* create_auto_attack()
  {
    return nullptr;
  }

  // Apparently shaman pets by default do not inherit attack speed buffs from owner
  double composite_melee_speed() const override
  {
    return o()->cache.attack_haste();
  }
};

// ==========================================================================
// Base Shaman Pet Action
// ==========================================================================

template <typename T_PET, typename T_ACTION>
struct pet_action_t : public T_ACTION
{
  typedef pet_action_t<T_PET, T_ACTION> super;

  pet_action_t( T_PET* pet, const std::string& name, const spell_data_t* spell = spell_data_t::nil(),
                const std::string& options = std::string() )
    : T_ACTION( name, pet, spell )
  {
    this->parse_options( options );

    this->special  = true;
    this->may_crit = true;
    // this -> crit_bonus_multiplier *= 1.0 + p() -> o() -> spec.elemental_fury -> effectN( 1 ).percent();
  }

  T_PET* p() const
  {
    return debug_cast<T_PET*>( this->player );
  }

  void init() override
  {
    T_ACTION::init();

    if ( !this->player->sim->report_pets_separately )
    {
      auto it = range::find_if( p()->o()->pet_list,
                                [ this ]( pet_t* pet ) { return this->player->name_str == pet->name_str; } );

      if ( it != p()->o()->pet_list.end() && this->player != *it )
      {
        this->stats = ( *it )->get_stats( this->name(), this );
      }
    }
  }
};

// ==========================================================================
// Base Shaman Pet Melee Attack
// ==========================================================================

template <typename T_PET>
struct pet_melee_attack_t : public pet_action_t<T_PET, melee_attack_t>
{
  using super = pet_melee_attack_t<T_PET>;

  pet_melee_attack_t( T_PET* pet, const std::string& name, const spell_data_t* spell = spell_data_t::nil(),
                      const std::string& options = std::string() )
    : pet_action_t<T_PET, melee_attack_t>( pet, name, spell, options )
  {
    if ( this->school == SCHOOL_NONE )
      this->school = SCHOOL_PHYSICAL;

    if ( this->p()->owner_coeff.sp_from_sp > 0 || this->p()->owner_coeff.sp_from_ap > 0 )
    {
      this->spell_power_mod.direct = 1.0;
    }
  }

  void init() override
  {
    pet_action_t<T_PET, melee_attack_t>::init();

    if ( !this->special )
    {
      this->weapon            = &( this->p()->main_hand_weapon );
      this->base_execute_time = this->weapon->swing_time;
    }
  }

  void execute() override
  {
    // If we're casting, we should clip a swing
    if ( this->time_to_execute > timespan_t::zero() && this->player->executing )
      this->schedule_execute();
    else
      pet_action_t<T_PET, melee_attack_t>::execute();
  }
};

// ==========================================================================
// Generalized Auto Attack Action
// ==========================================================================

struct auto_attack_t : public melee_attack_t
{
  auto_attack_t( shaman_pet_t* player ) : melee_attack_t( "auto_attack", player )
  {
    assert( player->main_hand_weapon.type != WEAPON_NONE );
    player->main_hand_attack = player->create_auto_attack();
  }

  void execute() override
  {
    player->main_hand_attack->schedule_execute();
  }

  bool ready() override
  {
    if ( player->is_moving() )
      return false;
    return ( player->main_hand_attack->execute_event == nullptr );
  }
};

// ==========================================================================
// Base Shaman Pet Spell
// ==========================================================================

template <typename T_PET>
struct pet_spell_t : public pet_action_t<T_PET, spell_t>
{
  using super = pet_spell_t<T_PET>;

  pet_spell_t( T_PET* pet, const std::string& name, const spell_data_t* spell = spell_data_t::nil(),
               const std::string& options = std::string() )
    : pet_action_t<T_PET, spell_t>( pet, name, spell, options )
  {
    this->parse_options( options );
  }
};

// ==========================================================================
// Base Shaman Pet Method Definitions
// ==========================================================================

action_t* shaman_pet_t::create_action( const std::string& name, const std::string& options_str )
{
  if ( name == "auto_attack" )
    return new auto_attack_t( this );

  return pet_t::create_action( name, options_str );
}

// ==========================================================================
// Feral Spirit
// ==========================================================================

struct base_wolf_t : public shaman_pet_t
{
  action_t* alpha_wolf;
  buff_t* alpha_wolf_buff;
  wolf_type_e wolf_type;

  base_wolf_t( shaman_t* owner, const std::string& name )
    : shaman_pet_t( owner, name ), alpha_wolf( nullptr ), alpha_wolf_buff( nullptr ), wolf_type( SPIRIT_WOLF )
  {
    owner_coeff.ap_from_ap = 0.6;

    main_hand_weapon.swing_time = timespan_t::from_seconds( 1.5 );
  }
};

template <typename T>
struct wolf_base_attack_t : public pet_melee_attack_t<T>
{
  using super = wolf_base_attack_t<T>;

  wolf_base_attack_t( T* wolf, const std::string& n, const spell_data_t* spell = spell_data_t::nil(),
                      const std::string& options_str = std::string() )
    : pet_melee_attack_t<T>( wolf, n, spell )
  {
    this->parse_options( options_str );
  }

  void execute() override
  {
    pet_melee_attack_t<T>::execute();
  }

  void tick( dot_t* d ) override
  {
    pet_melee_attack_t<T>::tick( d );
  }
};

template <typename T>
struct wolf_base_auto_attack_t : public pet_melee_attack_t<T>
{
  using super = wolf_base_auto_attack_t<T>;

  wolf_base_auto_attack_t( T* wolf, const std::string& n, const spell_data_t* spell = spell_data_t::nil(),
                           const std::string& options_str = std::string() )
    : pet_melee_attack_t<T>( wolf, n, spell )
  {
    this->parse_options( options_str );

    this->background = this->repeating = true;
    this->special                      = false;

    this->weapon            = &( this->p()->main_hand_weapon );
    this->weapon_multiplier = 1.0;

    this->base_execute_time = this->weapon->swing_time;
    this->school            = SCHOOL_PHYSICAL;
  }
};

struct spirit_wolf_t : public base_wolf_t
{
  struct fs_melee_t : public wolf_base_auto_attack_t<spirit_wolf_t>
  {
    const spell_data_t* maelstrom;

    fs_melee_t( spirit_wolf_t* player ) : super( player, "melee" ), maelstrom( player->find_spell( 190185 ) )
    {
    }

    void impact( action_state_t* state ) override
    {
      melee_attack_t::impact( state );

      shaman_t* o = p()->o();
      /*if ( o->spec.feral_spirit_2->ok() )
      {
        o->trigger_maelstrom_gain( maelstrom->effectN( 1 ).resource( RESOURCE_MAELSTROM ), o->gain.feral_spirit );
      }*/
    }
  };

  spirit_wolf_t( shaman_t* owner ) : base_wolf_t( owner, "spirit_wolf" )
  {
  }

  attack_t* create_auto_attack() override
  {
    return new fs_melee_t( this );
  }
};

// ==========================================================================
// DOOM WOLVES OF NOT REALLY DOOM ANYMORE
// ==========================================================================

struct elemental_wolf_base_t : public base_wolf_t
{
  struct dw_melee_t : public wolf_base_auto_attack_t<elemental_wolf_base_t>
  {
    const spell_data_t* maelstrom;

    dw_melee_t( elemental_wolf_base_t* player ) : super( player, "melee" ), maelstrom( player->find_spell( 190185 ) )
    {
    }

    void impact( action_state_t* state ) override
    {
      super::impact( state );

      //Needs to be maelstrom weapon
      //p()->o()->trigger_maelstrom_gain( maelstrom->effectN( 1 ).resource( RESOURCE_MAELSTROM ),
      //                                  p()->o()->gain.feral_spirit );
    }
  };

  cooldown_t* special_ability_cd;

  elemental_wolf_base_t( shaman_t* owner, const std::string& name )
    : base_wolf_t( owner, name ), special_ability_cd( nullptr )
  {
    // Make Wolves dynamic so we get accurate reporting for special abilities
    dynamic = true;
  }

  attack_t* create_auto_attack() override
  {
    return new dw_melee_t( this );
  }
};

struct frost_wolf_t : public elemental_wolf_base_t
{
  frost_wolf_t( shaman_t* owner ) : elemental_wolf_base_t( owner, /*owner->raptor_glyph*/ false ? "frost_raptor" : "frost_wolf" )
  {
    wolf_type = FROST_WOLF;
  }
};

struct fire_wolf_t : public elemental_wolf_base_t
{
  fire_wolf_t( shaman_t* owner ) : elemental_wolf_base_t( owner, /*owner->raptor_glyph*/ false ? "fiery_raptor" : "fiery_wolf" )
  {
    wolf_type = FIRE_WOLF;
  }
};

struct lightning_wolf_t : public elemental_wolf_base_t
{
  lightning_wolf_t( shaman_t* owner )
    : elemental_wolf_base_t( owner, /*owner->raptor_glyph*/ false ? "lightning_raptor" : "lightning_wolf" )
  {
    wolf_type = LIGHTNING_WOLF;
  }
};

// ==========================================================================
// Primal Elemental Base
// ==========================================================================

struct primal_elemental_t : public shaman_pet_t
{
  struct travel_t : public action_t
  {
    travel_t( player_t* player ) : action_t( ACTION_OTHER, "travel", player )
    {
      background = true;
    }
    void execute() override
    {
      player->current.distance = 1;
    }
    timespan_t execute_time() const override
    {
      return timespan_t::from_seconds( player->current.distance / 10.0 );
    }
    bool ready() override
    {
      return ( player->current.distance > 1 );
    }
    bool usable_moving() const override
    {
      return true;
    }
  };

  primal_elemental_t( shaman_t* owner, const std::string& name, bool guardian = false, bool auto_attack = true )
    : shaman_pet_t( owner, name, guardian, auto_attack )
  {
  }

  void create_default_apl() override
  {
    if ( use_auto_attack )
    {
      // Travel must come before auto attacks
      action_priority_list_t* def = get_action_priority_list( "default" );
      def->add_action( "travel" );
    }

    shaman_pet_t::create_default_apl();
  }

  action_t* create_action( const std::string& name, const std::string& options_str ) override
  {
    if ( name == "travel" )
      return new travel_t( this );

    return shaman_pet_t::create_action( name, options_str );
  }

  double composite_attack_power_multiplier() const override
  {
    double m = pet_t::composite_attack_power_multiplier();

    m *= 1.0 + o()->talent.primal_elementalist->effectN( 1 ).percent();

    return m;
  }

  double composite_spell_power_multiplier() const override
  {
    double m = pet_t::composite_spell_power_multiplier();

    m *= 1.0 + o()->talent.primal_elementalist->effectN( 1 ).percent();

    return m;
  }

  attack_t* create_auto_attack() override
  {
    auto attack               = new pet_melee_attack_t<primal_elemental_t>( this, "melee" );
    attack->background        = true;
    attack->repeating         = true;
    attack->special           = false;
    attack->school            = SCHOOL_PHYSICAL;
    attack->weapon_multiplier = 1.0;
    return attack;
  }
};

// ==========================================================================
// Earth Elemental
// ==========================================================================

struct earth_elemental_t : public primal_elemental_t
{
  earth_elemental_t( shaman_t* owner, bool guardian )
    : primal_elemental_t( owner, ( !guardian ) ? "primal_earth_elemental" : "greater_earth_elemental", guardian )
  {
    main_hand_weapon.swing_time = timespan_t::from_seconds( 2.0 );
    owner_coeff.ap_from_sp      = 0.25;
  }
};

// ==========================================================================
// Fire Elemental
// ==========================================================================

struct fire_elemental_t : public primal_elemental_t
{
  const spell_data_t* ember_elemental_summon;

  fire_elemental_t( shaman_t* owner, bool guardian )
    : primal_elemental_t( owner, ( guardian ) ? "greater_fire_elemental" : "primal_fire_elemental", guardian, false ),
      ember_elemental_summon( owner->find_spell( 275385 ) )
  {
    owner_coeff.sp_from_sp = 1.0;
  }

  struct meteor_t : public pet_spell_t<fire_elemental_t>
  {
    meteor_t( fire_elemental_t* player, const std::string& options )
      : super( player, "meteor", player->find_spell( 117588 ), options )
    {
      aoe = -1;
    }
  };

  struct fire_blast_t : public pet_spell_t<fire_elemental_t>
  {
    fire_blast_t( fire_elemental_t* player, const std::string& options )
      : super( player, "fire_blast", player->find_spell( 57984 ), options )
    {
    }

    bool usable_moving() const override
    {
      return true;
    }
  };

  struct immolate_t : public pet_spell_t<fire_elemental_t>
  {
    immolate_t( fire_elemental_t* player, const std::string& options )
      : super( player, "immolate", player->find_spell( 118297 ), options )
    {
      hasted_ticks = tick_may_crit = true;
    }
  };

  void create_default_apl() override
  {
    primal_elemental_t::create_default_apl();

    action_priority_list_t* def = get_action_priority_list( "default" );

    if ( o()->talent.primal_elementalist->ok() )
    {
      def->add_action( "meteor" );
      def->add_action( "immolate,target_if=!ticking" );
    }

    def->add_action( "fire_blast" );
  }

  action_t* create_action( const std::string& name, const std::string& options_str ) override
  {
    if ( name == "fire_blast" )
      return new fire_blast_t( this, options_str );
    if ( name == "meteor" )
      return new meteor_t( this, options_str );
    if ( name == "immolate" )
      return new immolate_t( this, options_str );

    return primal_elemental_t::create_action( name, options_str );
  }

  void dismiss( bool expired ) override
  {
    primal_elemental_t::dismiss( expired );
  }
};

// create baby azerite trait version

// struct ember_elemental_t : public primal_elemental_t
//{
//  ember_elemental_t( shaman_t* owner ) : primal_elemental_t( owner, "ember_elemental", true, false )
//  {
//  }
//
//  struct ember_blast_t : public pet_spell_t<ember_elemental_t>
//  {
//    ember_blast_t( ember_elemental_t* player, const std::string& options )
//      : super( player, "ember_blast", player->find_spell( 275382 ), options )
//    {
//      may_crit    = true;
//      base_dd_min = base_dd_max = player->o()->azerite.echo_of_the_elementals.value();
//    }
//
//    bool usable_moving() const override
//    {
//      return true;
//    }
//  };
//
//  void create_default_apl() override
//  {
//    primal_elemental_t::create_default_apl();
//
//    action_priority_list_t* def = get_action_priority_list( "default" );
//
//    def->add_action( "ember_blast" );
//  }
//
//  action_t* create_action( const std::string& name, const std::string& options_str ) override
//  {
//    if ( name == "ember_blast" )
//      return new ember_blast_t( this, options_str );
//
//    return primal_elemental_t::create_action( name, options_str );
//  }
//};

// ==========================================================================
// Storm Elemental
// ==========================================================================

struct storm_elemental_t : public primal_elemental_t
{
  struct eye_of_the_storm_aoe_t : public pet_spell_t<storm_elemental_t>
  {
    int tick_number   = 0;
    double damage_amp = 0.0;

    eye_of_the_storm_aoe_t( storm_elemental_t* player, const std::string& options )
      : super( player, "eye_of_the_storm_aoe", player->find_spell( 269005 ), options )
    {
      aoe        = -1;
      background = true;

      // parent spell (eye_of_the_storm_t) has the damage increase percentage
      damage_amp = player->o()->find_spell( 157375 )->effectN( 2 ).percent();
    }

    double action_multiplier() const override
    {
      double m = pet_spell_t::action_multiplier();
      m *= std::pow( 1.0 + damage_amp, tick_number );
      return m;
    }
  };

  struct eye_of_the_storm_t : public pet_spell_t<storm_elemental_t>
  {
    eye_of_the_storm_aoe_t* breeze = nullptr;

    eye_of_the_storm_t( storm_elemental_t* player, const std::string& options )
      : super( player, "eye_of_the_storm", player->find_spell( 157375 ), options )
    {
      channeled   = true;
      tick_action = breeze = new eye_of_the_storm_aoe_t( player, options );
    }

    void tick( dot_t* d ) override
    {
      breeze->tick_number = d->current_tick;
      pet_spell_t::tick( d );
    }

    bool ready() override
    {
      if ( p()->o()->talent.primal_elementalist->ok() )
      {
        return pet_spell_t<storm_elemental_t>::ready();
      }
      return false;
    }
  };

  struct wind_gust_t : public pet_spell_t<storm_elemental_t>
  {
    wind_gust_t( storm_elemental_t* player, const std::string& options )
      : super( player, "wind_gust", player->find_spell( 157331 ), options )
    {
    }
  };

  struct call_lightning_t : public pet_spell_t<storm_elemental_t>
  {
    call_lightning_t( storm_elemental_t* player, const std::string& options )
      : super( player, "call_lightning", player->find_spell( 157348 ), options )
    {
    }

    void execute() override
    {
      super::execute();

      p()->call_lightning->trigger();
    }
  };

  buff_t* call_lightning;
  const spell_data_t* spark_elemental_summon;

  storm_elemental_t( shaman_t* owner, bool guardian )
    : primal_elemental_t( owner, ( !guardian ) ? "primal_storm_elemental" : "greater_storm_elemental", guardian,
                          false ),
      call_lightning( nullptr ),
      spark_elemental_summon( owner->find_spell( 275386 ) )
  {
    owner_coeff.sp_from_sp = 1.0000;
  }

  void create_default_apl() override
  {
    primal_elemental_t::create_default_apl();

    action_priority_list_t* def = get_action_priority_list( "default" );
    if ( o()->talent.primal_elementalist->ok() )
    {
      def->add_action( "eye_of_the_storm,if=buff.call_lightning.remains>=10" );
    }
    def->add_action( "call_lightning" );
    def->add_action( "wind_gust" );
  }

  void create_buffs() override
  {
    primal_elemental_t::create_buffs();

    call_lightning = make_buff( this, "call_lightning", find_spell( 157348 ) )->set_cooldown( timespan_t::zero() );
  }

  double composite_player_multiplier( school_e school ) const override
  {
    double m = primal_elemental_t::composite_player_multiplier( school );

    if ( call_lightning->up() )
      m *= 1.0 + call_lightning->data().effectN( 2 ).percent();

    return m;
  }

  action_t* create_action( const std::string& name, const std::string& options_str ) override
  {
    if ( name == "eye_of_the_storm" )
      return new eye_of_the_storm_t( this, options_str );
    if ( name == "call_lightning" )
      return new call_lightning_t( this, options_str );
    if ( name == "wind_gust" )
      return new wind_gust_t( this, options_str );

    return primal_elemental_t::create_action( name, options_str );
  }

  void dismiss( bool expired ) override
  {
    primal_elemental_t::dismiss( expired );
    o()->buff.wind_gust->expire();
  }
};

// create baby azerite trait version

// struct spark_elemental_t : public primal_elemental_t
//{
//  spark_elemental_t( shaman_t* owner ) : primal_elemental_t( owner, "spark_elemental", true, false )
//  {
//  }
//
//  struct shocking_blast_t : public pet_spell_t<spark_elemental_t>
//  {
//    shocking_blast_t( spark_elemental_t* player, const std::string& options )
//      : super( player, "shocking_blast", player->find_spell( 275384 ), options )
//    {
//      may_crit    = true;
//      base_dd_min = base_dd_max = player->o()->azerite.echo_of_the_elementals.value();
//    }
//
//    bool usable_moving() const override
//    {
//      return true;
//    }
//  };
//
//  void create_default_apl() override
//  {
//    primal_elemental_t::create_default_apl();
//
//    action_priority_list_t* def = get_action_priority_list( "default" );
//
//    def->add_action( "shocking_blast" );
//  }
//
//  action_t* create_action( const std::string& name, const std::string& options_str ) override
//  {
//    if ( name == "shocking_blast" )
//      return new shocking_blast_t( this, options_str );
//
//    return primal_elemental_t::create_action( name, options_str );
//  }
//};

}  // end namespace pet


}  // namespace shaman

#endif  /* SC_SHAMAN */
