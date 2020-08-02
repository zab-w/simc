#include "simulationcraft.hpp"
#include "sc_shaman_class.hpp"

namespace shaman
{

struct enhancement_shaman_t;

struct enhancement_td_t : public shaman_td_t
{
public:
  shaman_td_t* get()
  {
    return this;
  };

  struct dots : shaman_td_t::dots
  {
    dot_t* molten_weapon;
  } dot;

  struct debuffs : shaman_td_t::debuffs
  {
    buff_t* earthen_spike;
  } debuff;

  struct heals : shaman_td_t::heals
  {
  
  } heal;

  enhancement_td_t( player_t* target, enhancement_shaman_t* p );

  /*enhancement_shaman_t* actor() const
  {
    return debug_cast < enhancement_shaman_t * ( source );
  }*/
  
};

struct enhancement_shaman_t : public shaman_t
{
  public:
  bool raptor_glyph;

  struct actions_t : shaman_t::actions_t
  {
    spell_t* crashing_storm;
    attack_t* crash_lightning_aoe;
    spell_t* molten_weapon;
    action_t* molten_weapon_dot;
  } action;

  struct pets_t : shaman_t::pets_t
  {
    spawner::pet_spawner_t<pet_t, shaman_t> spirit_wolves;
    spawner::pet_spawner_t<pet_t, shaman_t> fire_wolves;
    spawner::pet_spawner_t<pet_t, shaman_t> frost_wolves;
    spawner::pet_spawner_t<pet_t, shaman_t> lightning_wolves;

    pets_t( enhancement_shaman_t* );
  } pet;

  struct buffs_t : shaman_t::buffs_t
  {
    buff_t* crash_lightning;
    buff_t* feral_spirit;
    buff_t* hot_hand;
    buff_t* lightning_shield;
    buff_t* stormbringer;

    buff_t* forceful_winds;
    buff_t* icy_edge;
    buff_t* molten_weapon;
    buff_t* crackling_surge;
    buff_t* gathering_storms;
  } buff;

  struct cooldowns_t : shaman_t::cooldowns_t
  {
    cooldown_t* feral_spirits;
    cooldown_t* strike; // shared CD of Storm Strike and Windstrike
    cooldown_t* shock; // shared CD of Flame Shock and Frost Shock
  } cooldown;

  struct gains_t : shaman_t::gains_t
  {
    gain_t* ascendance;
    gain_t* feral_spirit;
    gain_t* forceful_winds;
  } gain;

  struct procs_t : shaman_t::procs_t
  {
    proc_t* windfury;
    proc_t* hot_hand;
    proc_t* stormbringer;
  };

  struct specializations_t : shaman_t::specializations_t
  {
    // Enhancement
    const spell_data_t* crash_lightning;
    const spell_data_t* critical_strikes;
    const spell_data_t* dual_wield;
    //const spell_data_t* feral_spirit_2;  // Is now maelstrom weapon
    const spell_data_t* maelstrom_weapon;
    const spell_data_t* stormbringer;
    const spell_data_t* flametongue;
    const spell_data_t* windfury;
  } spec;

  // Masteries
  struct
  {
    const spell_data_t* enhanced_elements; 
  } mastery;

  struct talents_t : shaman_t::talents_t
  {
    // Enhancement

    // T15
    // lashing flames
    const spell_data_t* forceful_winds;
    // elemental blast - shared

    // T25
    // stormfury
    const spell_data_t* hot_hand;
    // totem mastery - shared

    // T30
    // spirit wolf - shared
    // earth shield - shared
    // static charge - shared

    // T35
    // cycle of the elements
    const spell_data_t* hailstorm;
    // fire nova

    // T40
    // natures guardian - shared
    const spell_data_t* feral_lunge;
    // wind rush totem - shared

    // T45
    const spell_data_t* crashing_storm;
    // stormkeeper - shared
    const spell_data_t* sundering;

    // T50
    const spell_data_t* elemental_spirits;
    const spell_data_t* earthen_spike;
    // ascendance - shared
  } talent;

  struct misc_t : shaman_t::misc_t
  {
    const spell_data_t* maelstrom_melee_gain;
    const spell_data_t* feral_spirit;
  } spell;

  // Cached pointer for ascendance / normal white melee
  shaman_attack_t* ascendance_mh;
  shaman_attack_t* ascendance_oh;

  shaman_attack_t *windfury_mh, *windfury_oh;
  shaman_spell_t* flametongue;

  // Elemental Spirits attacks
  shaman_attack_t* molten_weapon;
  shaman_attack_t* icy_edge;

  enhancement_shaman_t( sim_t* sim, util::string_view name, race_e r = RACE_TAUREN )
      : shaman_t(sim, name, r),
      pet( this ),
      raptor_glyph( false )
  {
    cooldown.strike = get_cooldown( "strike" );
    cooldown.feral_spirits = get_cooldown( "feral_spirit" );

    ascendance_mh = nullptr;
    ascendance_oh = nullptr;

    // Weapon Enchants
    windfury_mh = nullptr;
    windfury_oh = nullptr;
    flametongue = nullptr;

    // Elemental Spirits attacks
    molten_weapon = nullptr;
    icy_edge      = nullptr;

    resource_regeneration = regen_type::DISABLED;
  }

  ~enhancement_shaman_t() override;

  // Misc
  void summon_feral_spirits( timespan_t duration );

  // triggers
  void trigger_maelstrom_gain( double base, gain_t* gain = nullptr );
  void trigger_windfury_weapon( const action_state_t* );
  void trigger_flametongue_weapon( const action_state_t* );
  void trigger_icy_edge( const action_state_t* );
  void trigger_stormbringer( const action_state_t* state, double proc_chance = -1.0, proc_t* proc_obj = nullptr );
  void trigger_lightning_shield( const action_state_t* state );
  void trigger_hot_hand( const action_state_t* state );

};

struct lightning_shield_buff_t : public buff_t
{
  lightning_shield_buff_t( shaman_t* p ) : buff_t( p, "lightning_shield", p->find_spell( 192106 ) )
  {
    set_duration( s_data->duration() );
  }
};

struct forceful_winds_buff_t : public buff_t
{
  forceful_winds_buff_t( shaman_t* p ) : buff_t( p, "forceful_winds", p->find_spell( 262652 ) )
  {
  }
};

struct icy_edge_buff_t : public buff_t
{
  icy_edge_buff_t( shaman_t* p ) : buff_t( p, "icy_edge", p->find_spell( 224126 ) )
  {
    set_duration( s_data->duration() );
    set_max_stack( 10 );
    set_stack_behavior( buff_stack_behavior::ASYNCHRONOUS );
  }
};

struct molten_weapon_buff_t : public buff_t
{
  molten_weapon_buff_t( shaman_t* p ) : buff_t( p, "molten_weapon", p->find_spell( 224125 ) )
  {
    set_duration( s_data->duration() );
    set_default_value( 1.0 + s_data->effectN( 1 ).percent() );
    set_max_stack( 10 );
    add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );
    set_stack_behavior( buff_stack_behavior::ASYNCHRONOUS );
  }
};

struct crackling_surge_buff_t : public buff_t
{
  crackling_surge_buff_t( shaman_t* p ) : buff_t( p, "crackling_surge", p->find_spell( 224127 ) )
  {
    set_duration( s_data->duration() );
    set_default_value( s_data->effectN( 1 ).percent() );
    set_max_stack( 10 );
    set_stack_behavior( buff_stack_behavior::ASYNCHRONOUS );
  }
};

struct gathering_storms_buff_t : public buff_t
{
  gathering_storms_buff_t( shaman_t* p ) : buff_t( p, "gathering_storms", p->find_spell( 198300 ) )
  {
    set_duration( s_data->duration() );
    // Buff applied to player is id# 198300, but appears to pull the value data from the crash lightning ability id
    // instead.  Probably setting an override value on gathering storms from crash lightning data.
    // set_default_value( s_data->effectN( 1 ).percent() ); --replace with this if ever changed
    set_default_value( p->find_spell( 187874 )->effectN( 2 ).percent() );
    // set_max_stack( 1 );
  }
};

//struct enh_ascendance_buff_t : public ascendance_buff_t
//{
//  enh_ascendance_buff_t( enhancement_shaman_t* p ) : ascendance_buff_t(p)
//  {
//  /*  set_tick_callback( [ p ]( buff_t* b, int, timespan_t ) 
//    {
//        double g = b->data().effectN( 4 ).base_value();
//    } );
//    p->trigger_maelstrom_gain( g, p->gain.ascendance );*/
//  }
//
//};

enhancement_td_t::enhancement_td_t( player_t* target, enhancement_shaman_t* p ) : shaman_td_t( target, p )
{
  dot.molten_weapon    = target->get_dot( "molten_weapon", p );
  debuff.earthen_spike = make_buff( *this, "earthen_spike", p->talent.earthen_spike )
                             ->set_cooldown( timespan_t::zero() )  // Handled by the action
                             // -10% resistance in spell data, treat it as a multiplier instead
                             ->set_default_value( 1.0 + p->talent.earthen_spike->effectN( 2 ).percent() );
};

// ==========================================================================
// Enhancement Action, Attack and Spell declarations
// ==========================================================================
//
template <class Base>
struct enhancement_action_t : public virtual shaman_action_t<Base>
{
private:
  using ab = Base;

public:
  using base_t = enhancement_action_t<Base>;

  bool may_proc_maelstrom_weapon;
  bool may_proc_stormbringer;
  bool may_proc_ability_procs;
  proc_t *proc_mw, *proc_sb;

  enhancement_action_t( const std::string& n, shaman_t* player, const spell_data_t* s = spell_data_t::nil() )
    : ab( n, player, s ),
      may_proc_maelstrom_weapon( false ),  // Change to whitelisting
      may_proc_stormbringer( p->spec.stormbringer->ok() ),

      proc_mw( nullptr ),
      proc_sb( nullptr ),
  {
    if ( ab::data().affected_by( player->spec.enhancement_shaman->effectN( 1 ) ) )
    {
      ab::base_multiplier *= 1.0 + player->spec.enhancement_shaman->effectN( 1 ).percent();
    }

    if ( ab::data().affected_by( player->spec.restoration_shaman->effectN( 3 ) ) )
    {
      ab::base_dd_multiplier *= 1.0 + player->spec.restoration_shaman->effectN( 3 ).percent();
    }
  }

  enhancement_shaman_t* p() override
  {
    return debug_cast<enhancement_shaman_t*>( ab::player );
  }

  const enhancement_shaman_t* p() const override
  {
    return debug_cast<enhancement_shaman_t*>( ab::player );
  }

  void init() override
  {
    ab::init();

    // Setup Hasted CD for Enhancement
    if ( ab::data().affected_by( p()->spec.shaman->effectN( 2 ) ) )
    {
      ab::cooldown->hasted = true;
    }

    // Setup Hasted GCD for Enhancement
    if ( ab::data().affected_by( p()->spec.shaman->effectN( 3 ) ) )
    {
      ab::gcd_type = gcd_haste_type::ATTACK_HASTE;
    }

    if ( may_proc_stormbringer )
    {
      may_proc_stormbringer = ab::weapon;
    }
  }

  double action_multiplier() const override
  {
    double m = ab::action_multiplier();

    // Move to enhancement

     if ( p()->specialization() == SHAMAN_ENHANCEMENT )
    {
      if ( ( dbc::is_school( this->school, SCHOOL_FIRE ) || dbc::is_school( this->school, SCHOOL_FROST ) ||
             dbc::is_school( this->school, SCHOOL_NATURE ) ) &&
           p()->mastery.enhanced_elements->ok() )
      {
        if ( ab::data().affected_by( p()->mastery.enhanced_elements->effectN( 1 ) ) ||
             ab::data().affected_by( p()->mastery.enhanced_elements->effectN( 5 ) ) || enable_enh_mastery_scaling )
        {
          //...hopefully blizzard never makes direct and periodic scaling different from eachother in our mastery..
          m *= 1.0 + p()->cache.mastery_value();
        }
      }
    }

     if ( affected_by_molten_weapon && p()->buff.molten_weapon->check() )
    {
      m *= std::pow( p()->buff.molten_weapon->check_value(), p()->buff.molten_weapon->check() );
    }

    return m;
  }

  void ab::init_finished() override
  {
    double procchance = p()->spec.stormbringer->proc_chance();
    if ( may_proc_stormbringer )
    {
      proc_sb = player->get_proc( std::string( "Stormbringer: " ) + full_name() );
    }

    if ( may_proc_stormbringer )
    {
      proc_sb = player->get_proc( std::string( "Stormbringer: " ) + full_name() );
    }

    if ( may_proc_maelstrom_weapon )
    {
      proc_mw = player->get_proc( std::string( "Maelstrom Weapon: " ) + full_name() );
    }

    ab::init_finished();
  }

  double stormbringer_proc_chance() const
  {
    double base_chance = 0;

    base_chance += p()->spec.stormbringer->proc_chance() +
                   p()->cache.mastery() * p()->mastery.enhanced_elements->effectN( 3 ).mastery_value();

    return base_chance;
  }

  void trigger_maelstrom_weapon( const action_state_t* source_state, double amount = 0 )
  {
    if ( !may_proc_maelstrom_weapon )
    {
      return;
    }

    /*if ( p()->buff.ghost_wolf->check() )
    {
      return;
    }*/

    // needs to roll stacks of MW weapon
    // proc_mw->occur();

    return;
  }
};

struct enhancement_attack_t : public shaman_attack_t, enhancement_action_t<melee_attack_t>
{
private:  
  using shaman_action = shaman_action_t<melee_attack_t>;
  using enh_action = enhancement_action_t<melee_attack_t>;
  using ab = shaman_attack_t;

public:
  bool may_proc_windfury;
  bool may_proc_flametongue;
  bool may_proc_hot_hand;
  bool may_proc_icy_edge;

  proc_t *proc_wf, *proc_ft, *proc_fb, *proc_ls, *proc_hh, *proc_pp;
  
  enhancement_attack_t( const std::string& token, enhancement_shaman_t* p, const spell_data_t* s )
      : ab( token, p, s ),
      enh_action( token, p, s ),
      shaman_action(token, p, s),
      may_proc_windfury( p->spec.windfury->ok() ),
      may_proc_flametongue( p->spec.flametongue->ok() ),
      may_proc_hot_hand( p->talent.hot_hand->ok() ),
      may_proc_icy_edge( false ),
      proc_wf( nullptr ),
      proc_ft( nullptr ),
      proc_hh( nullptr )
  {

  }

  void init() override
  {
    ab::init();

    if ( may_proc_flametongue )
    {
      may_proc_flametongue = ab::weapon != nullptr;
    }

    if ( may_proc_windfury )
    {
      may_proc_windfury = ab::weapon != nullptr;
    }

    if ( may_proc_hot_hand )
    {
      may_proc_hot_hand = ab::weapon != nullptr;
    }

    may_proc_lightning_shield = ab::weapon != nullptr;
  }

  void init_finished() override
  {
    if ( may_proc_flametongue )
    {
      proc_ft = player->get_proc( std::string( "Flametongue: " ) + full_name() );
    }

    if ( may_proc_hot_hand )
    {
      proc_hh = player->get_proc( std::string( "Hot Hand: " ) + full_name() );
    }

    if ( may_proc_lightning_shield )  // Needs to refactor to defensive version
    {
      proc_ls = player->get_proc( std::string( "Lightning Shield Overcharge: " ) + full_name() );
    }

    if ( may_proc_windfury )
    {
      proc_wf = player->get_proc( std::string( "Windfury: " ) + full_name() );
    }

    ab::init_finished();
  }
};

struct enhancement_spell_t : public shaman_spell_t, enhancement_action_t<spell_t>
{
private:
    using enh_action = enhancement_action_t<spell_t>;
    using shaman_action = shaman_action_t<spell_t>;
    using spell_base = shaman_spell_base_t;
    using ab = shaman_spell_t;

public:

    enhancement_spell_t( const std::string& token, enhancement_shaman_t* p, const spell_data_t* s ) : 
        shaman_action(token, p, s),
        enh_action(token, p, s),
        spell_base(token, p, s),
        ab(token, p, s)
    {
      if ( data().affected_by( p->find_spell( 320137 )->effectN( 1 ) ) )
      {
        affected_by_stormkeeper = true;

        affected_by_molten_weapon =
            ab::data().affected_by_label( player->find_spell( 224125 )->effectN( 1 ).misc_value2() );
      }
    }

    void init() override
    {
      ab::init();
    }

};







//enhancement_shaman_t::pets_t::pets_t(enhancement_shaman_t* s)
//  : shaman_t::pets_t::pets_t(s),
//    spirit_wolves( "spirit_wolf", s, []( enhancement_shaman_t* s ) { return new pet pet::spirit_wolf_t( s ); } ),
//    fire_wolves( "fiery_wolf", s, []( enhancement_shaman_t* s ) { return new pet::fire_wolf_t( s ); } ),
//    frost_wolves( "frost_wolf", s, []( enhancement_shaman_t* s ) { return new pet::frost_wolf_t( s ); } ),
//    lightning_wolves( "lightning_wolf", s, []( enhancement_shaman_t* s ) { return new pet::lightning_wolf_t( s ); } )
//{
//}






}  // namespace shaman