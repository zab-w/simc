#include "simulationcraft.hpp"
#include "sc_shaman.hpp"

namespace shaman
{


struct enhancement_td_t : public shaman_td_t
{
public:
  shaman_td_t* get()
  {
    return this;
  }

  struct dots : shaman_td_t::dots
  {
    dot_t* molten_weapon;
  } dot;

  struct debuffs : shaman_td_t::debuffs
  {
    buff_t* earthen_spike;
  } debuff;

  enhancement_td_t( player_t* target, enhancement_shaman_t* p );
  
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
    const spell_data_t* enhancement_shaman;
    const spell_data_t* feral_spirit_2;  // 7.1 Feral Spirit Maelstrom gain passive
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

struct enh_ascendance_buff_t : public ascendance_buff_t
{
  enh_ascendance_buff_t( enhancement_shaman_t* p ) : ascendance_buff_t(p)
  {
    set_tick_callback( [ p ]( buff_t* b, int, timespan_t ) 
    {
        double g = b->data().effectN( 4 ).base_value();
    } );
    p->trigger_maelstrom_gain( g, p->gain.ascendance );
  }

};

enhancement_td_t::enhancement_td_t( player_t* target, enhancement_shaman_t* p ) : shaman_td_t(target, p)
{
   dot.molten_weapon    = target->get_dot( "molten_weapon", p );
   debuff.earthen_spike = make_buff( *this, "earthen_spike", p->talent.earthen_spike )
                             ->set_cooldown( timespan_t::zero() )  // Handled by the action
                             // -10% resistance in spell data, treat it as a multiplier instead
                             ->set_default_value( 1.0 + p->talent.earthen_spike->effectN( 2 ).percent() );
}










//enhancement_shaman_t::pets_t::pets_t(enhancement_shaman_t* s)
//  : shaman_t::pets_t::pets_t(s),
//    spirit_wolves( "spirit_wolf", s, []( enhancement_shaman_t* s ) { return new pet pet::spirit_wolf_t( s ); } ),
//    fire_wolves( "fiery_wolf", s, []( enhancement_shaman_t* s ) { return new pet::fire_wolf_t( s ); } ),
//    frost_wolves( "frost_wolf", s, []( enhancement_shaman_t* s ) { return new pet::frost_wolf_t( s ); } ),
//    lightning_wolves( "lightning_wolf", s, []( enhancement_shaman_t* s ) { return new pet::lightning_wolf_t( s ); } )
//{
//}






}  // namespace shaman