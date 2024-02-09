#include "patches_battle.h"

#include "custom_enemy.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_mario_move.h"
#include "tot_move_manager.h"

#include <ttyd/battle.h>
#include <ttyd/battle_ac.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_disp.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_status_effect.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/eff_stamp_n64.h>
#include <ttyd/effdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // action_command_patches.s
    void StartButtonDownChooseButtons();
    void BranchBackButtonDownChooseButtons();
    void StartButtonDownCheckComplete();
    void BranchBackButtonDownCheckComplete();
    void ConditionalBranchButtonDownCheckComplete();
    // audience_level_patches.s
    void StartSetTargetAudienceCount();
    void BranchBackSetTargetAudienceCount();
    // permanent_status_patches.s
    void StartCheckRecoveryStatus();
    void BranchBackCheckRecoveryStatus();
    void ConditionalBranchCheckRecoveryStatus();
    void StartStatusIconDisplay();
    void BranchBackStatusIconDisplay();
    void ConditionalBranchStatusIconDisplay();
    // star_power_patches.s
    void StartApplySpRegenMultiplierNoBingo();
    void BranchBackApplySpRegenMultiplierNoBingo();
    void StartApplySpRegenMultiplierBingo();
    void BranchBackApplySpRegenMultiplierBingo();
    // status_effect_patches.s
    void StartToggleScopedAndCheckFreezeBreak();
    void BranchBackToggleScopedAndCheckFreezeBreak();
    
    void setTargetAudienceCount() {
        mod::infinite_pit::battle::SetTargetAudienceAmount();
    }
    double applySpRegenMultiplier(double base_regen) {
        return mod::infinite_pit::battle::ApplySpRegenMultiplier(base_regen);
    }
    void toggleOffScopedStatus(
        ttyd::battle_unit::BattleWorkUnit* attacker,
        ttyd::battle_unit::BattleWorkUnit* target,
        ttyd::battle_database_common::BattleWeapon* weapon) {
        mod::infinite_pit::battle::ToggleScopedStatus(attacker, target, weapon);
    }
}

namespace mod::infinite_pit {

namespace {

// For convenience.
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_unit;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace ItemType = ::ttyd::item_data::ItemType;

using WeaponDamageFn = uint32_t (*) (
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*);

}

// Function hooks.
extern int32_t (*g_BattleActionCommandCheckDefence_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern int32_t (*g_BattlePreCheckDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t);
extern uint32_t (*g_BattleSetStatusDamageFromWeapon_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t);
extern uint32_t (*g_BtlUnit_CheckRecoveryStatus_trampoline)(
    BattleWorkUnit*, int8_t);
// Patch addresses.
extern const int32_t g_BattleCheckDamage_AlwaysFreezeBreak_BH;
extern const int32_t g_BattleCheckDamage_Patch_PaybackDivisor;
extern const int32_t g_BattleCheckDamage_Patch_HoldFastDivisor;
extern const int32_t g_BattleCheckDamage_Patch_ReturnPostageDivisor;
extern const int32_t g_BattleCheckDamage_Patch_SkipHugeTinyArrows;
extern const int32_t g_BattleAudience_ApRecoveryBuild_NoBingoRegen_BH;
extern const int32_t g_BattleAudience_ApRecoveryBuild_BingoRegen_BH;
extern const int32_t g_BattleAudience_SetTargetAmount_BH;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_BH;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_EH;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_BH;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_EH;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_CH1;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_BH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_EH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_CH1;

namespace battle {

// Fetches the base parameters for a given status from a weapon.
void GetStatusParams(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWeapon* weapon,
    int32_t status_type, int32_t& chance, int32_t& turns, int32_t& strength) {
    chance = 0;
    turns = 0;
    strength = 0;
    
    switch (status_type) {
        case StatusEffectType::ALLERGIC:
            chance = weapon->allergic_chance;
            turns = weapon->allergic_time;
            break;
        case StatusEffectType::SLEEP:
            chance = weapon->sleep_chance;
            turns = weapon->sleep_time;
            break;
        case StatusEffectType::STOP:
            chance = weapon->stop_chance;
            turns = weapon->stop_time;
            break;
        case StatusEffectType::DIZZY:
            chance = weapon->dizzy_chance;
            turns = weapon->dizzy_time;
            break;
        case StatusEffectType::POISON:
            chance = weapon->poison_chance;
            turns = weapon->poison_time;
            strength = weapon->poison_strength;
            break;
        case StatusEffectType::CONFUSE:
            chance = weapon->confuse_chance;
            turns = weapon->confuse_time;
            break;
        case StatusEffectType::ELECTRIC:
            chance = weapon->electric_chance;
            turns = weapon->electric_time;
            break;
        case StatusEffectType::DODGY:
            chance = weapon->dodgy_chance;
            turns = weapon->dodgy_time;
            break;
        case StatusEffectType::BURN:
            chance = weapon->burn_chance;
            turns = weapon->burn_time;
            break;
        case StatusEffectType::FREEZE:
            chance = weapon->freeze_chance;
            turns = weapon->freeze_time;
            break;
        case StatusEffectType::HUGE:
            if (weapon->size_change_strength > 0) {
                chance = weapon->size_change_chance;
                turns = weapon->size_change_time;
                strength = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::TINY:
            if (weapon->size_change_strength < 0) {
                chance = weapon->size_change_chance;
                turns = weapon->size_change_time;
                strength = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_UP:
            if (weapon->atk_change_strength > 0) {
                chance = weapon->atk_change_chance;
                turns = weapon->atk_change_time;
                strength = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_DOWN:
            if (weapon->atk_change_strength < 0) {
                chance = weapon->atk_change_chance;
                turns = weapon->atk_change_time;
                strength = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_UP:
            if (weapon->def_change_strength > 0) {
                chance = weapon->def_change_chance;
                turns = weapon->def_change_time;
                strength = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_DOWN:
            if (weapon->def_change_strength < 0) {
                chance = weapon->def_change_chance;
                turns = weapon->def_change_time;
                strength = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::CHARGE:
            chance = weapon->charge_strength ? 100 : 0;
            strength = weapon->charge_strength;
            break;
        case StatusEffectType::INVISIBLE:
            chance = weapon->invisible_chance;
            turns = weapon->invisible_time;
            break;
        case StatusEffectType::FAST:
            chance = weapon->fast_chance;
            turns = weapon->fast_time;
            break;
        case StatusEffectType::SLOW:
            chance = weapon->slow_chance;
            turns = weapon->slow_time;
            break;
        case StatusEffectType::PAYBACK:
            chance = weapon->payback_time ? 100 : 0;
            turns = weapon->payback_time;
            break;
        case StatusEffectType::HOLD_FAST:
            chance = weapon->hold_fast_time ? 100 : 0;
            turns = weapon->hold_fast_time;
            break;
        case StatusEffectType::HP_REGEN:
            chance = weapon->hp_regen_time ? 100 : 0;
            turns = weapon->hp_regen_time;
            strength = weapon->hp_regen_strength;
            break;
        case StatusEffectType::FP_REGEN:
            chance = weapon->fp_regen_time ? 100 : 0;
            turns = weapon->fp_regen_time;
            strength = weapon->fp_regen_strength;
            break;
        case StatusEffectType::FRIGHT:
            chance = weapon->fright_chance;
            break;
        case StatusEffectType::GALE_FORCE:
            chance = weapon->gale_force_chance;
            break;
        case StatusEffectType::OHKO:
            chance = weapon->ohko_chance;
            break;
    }
    
    switch (weapon->item_id) {
        case ItemType::SLEEPY_STOMP:
            if (status_type == StatusEffectType::SLEEP) {
                turns += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::JUMP_SLEEPY_STOMP) * 2 - 2;
            }
            break;
        case ItemType::HEAD_RATTLE:
            if (status_type == StatusEffectType::TINY) {
                turns += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::HAMMER_SHRINK_SMASH) * 2 - 2;
            }
            break;
        case ItemType::ICE_SMASH:
            if (status_type == StatusEffectType::FREEZE) {
                turns += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::HAMMER_ICE_SMASH) * 1 - 1;
            }
            break;
        case ItemType::TORNADO_JUMP:
            if (status_type == StatusEffectType::DIZZY) {
                // If not a floating enemy, don't inflict Dizzy status.
                if (!(target->attribute_flags & 
                      BattleUnitAttribute_Flags::UNQUAKEABLE)) {
                    turns = 0;
                    chance = 0;
                }
            }
            break;
        case ItemType::CHARGE:
        case ItemType::CHARGE_P:
            if (status_type == StatusEffectType::CHARGE) {
                strength += mario_move::GetStrategyBadgeLevel(
                    /* is_charge = */ true,
                    attacker->current_kind == BattleUnitType::MARIO) - 1;
            }
            break;
        case ItemType::SUPER_CHARGE:
        case ItemType::SUPER_CHARGE_P:
            if (status_type == StatusEffectType::DEFENSE_UP) {
                strength += mario_move::GetStrategyBadgeLevel(
                    /* is_charge = */ false,
                    attacker->current_kind == BattleUnitType::MARIO) - 1;
            }
            break;
    }

    if (status_type == StatusEffectType::CHARGE) {
        // Scale Charge attacks in the same way as ATK and FP damage.
        if (!weapon->item_id &&
            attacker->current_kind <= BattleUnitType::BONETAIL) {
            int32_t altered_charge;
            GetEnemyStats(
                attacker->current_kind, nullptr, &altered_charge,
                nullptr, nullptr, nullptr, strength);
            strength = altered_charge;
        }
        // Cap max Charge at 99.
        if (strength > 99) strength = 99;
    }
}

// Processes all statuses given a weapon.
uint32_t GetStatusDamageFromWeapon(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t extra_params) {
    uint32_t result = 0;
    if ((extra_params & 0x2000'0000) == 0) {
        for (int32_t type = 0; type < StatusEffectType::STATUS_MAX; ++type) {
            int32_t chance, turns, strength;
            GetStatusParams(
                attacker, target, weapon, type, chance, turns, strength);

            bool always_update = true;
            switch (type) {
                case StatusEffectType::GALE_FORCE: {
                    // Modify rate based on enemy HP and Huge/Tiny status.
                    if (target->size_change_turns > 0) {
                        if (target->size_change_strength > 0) {
                            chance = 0;
                        } else {
                            chance *= 1.5;
                        }
                    } else {
                        // Rate ranges from 70% at max health to 100% at half.
                        float factor =
                            1.3f - 0.6f * target->current_hp / target->max_hp;
                        if (factor > 1.0f) factor = 1.0f;
                        chance *= factor;
                    }
                    break;
                }
                case StatusEffectType::POISON:
                    // Each new infliction increases power by 1.
                    if (target->poison_turns > 0) {
                        strength = target->poison_strength + 1;
                    }
                    if (strength > 5) strength = 5;
                    break;
                case StatusEffectType::HUGE:
                case StatusEffectType::ATTACK_UP:
                case StatusEffectType::DEFENSE_UP:
                    // Should not update status if turns and strength = 0
                    // (e.g. a Tasty Tonic was used).
                    always_update = false;
                    break;
            }
            
            // Statuses are more likely to land.
            if (target->status_flags & BattleUnitStatus_Flags::SCOPED) {
                chance *= 1.5;
            }

            if (always_update || turns != 0 || strength != 0) {
                int32_t damage_result = 
                    ttyd::battle_damage::BattleSetStatusDamage(
                        &result, target, part, weapon->special_property_flags,
                        type, chance, /* gale_factor */ 0, turns, strength);

                if (damage_result && type == StatusEffectType::INVISIBLE) {
                    ttyd::battle_disp::btlDispPoseAnime(part);
                }
            }
        }
    }
    return result;
}

// Ticks down the turn count for a given status, and returns whether it expired.
uint32_t StatusEffectTick(BattleWorkUnit* unit, int8_t status_type) {
    uint32_t result = false;
    
    if (unit == nullptr)
        return result;
    if (ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO))
        return result;
    if (unit->current_hp < 1)
        return result;
    
    int8_t turns, strength;
    ttyd::battle_unit::BtlUnit_GetStatus(unit, status_type, &turns, &strength);
    
    if (turns <= 0) return result;
    
    // 100+ turn (permanent) statuses should not tick down.
    if (turns <= 99) --turns;
    
    if (turns == 0) {
        // If expired, reset the effect strength to 0.
        strength = 0;
        result = 1;
    } else if (status_type == StatusEffectType::POISON && strength < 5) {
        // Poison strengthens every turn it remains active.
        ++strength;
    }
    
    BtlUnit_SetStatus(unit, status_type, turns, strength);
    
    return result;
}

// Handles Lucky chance from evasion badges.
bool CheckEvasionBadges(BattleWorkUnit* unit) {
    if (g_Mod->state_.GetOptionNumericValue(OPT_EVASION_BADGES_CAP)) {
        float hit_chance = 100.f;
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            hit_chance *= 0.90f;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            hit_chance *= 0.75f;
        }
        if (unit->current_hp <= unit->unit_kind_params->danger_hp) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                hit_chance *= 0.67f;
            }
        }
        if (hit_chance < 20.f) hit_chance = 20.f;
        return ttyd::system::irand(100) >= hit_chance;
    } else {
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            if (ttyd::system::irand(100) < 10) return true;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            if (ttyd::system::irand(100) < 25) return true;
        }
        if (unit->current_hp <= unit->unit_kind_params->danger_hp) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                if (ttyd::system::irand(100) < 33) return true;
            }
        }
    }
    return false;
}

// Checks whether an attack should connect, or what caused it to miss.
int32_t PreCheckDamage(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t extra_params) {
        
    if (!target || !part) return 1;
    
    // Expend charge for chargeable attacks, whether attack lands or not.
    if (weapon->special_property_flags & 
        AttackSpecialProperty_Flags::CHARGE_BUFFABLE) {
        attacker->token_flags |= BattleUnitToken_Flags::CHARGE_EXPENDED;
    }
    
    if ((part->part_attribute_flags & PartsAttribute_Flags::UNK_MISS_4000) &&
        !(weapon->special_property_flags & 
          AttackSpecialProperty_Flags::UNKNOWN_0x40000)) {
        return 2;
    }
    
    // Scoped status guarantees a hit.
    if (target->status_flags & BattleUnitStatus_Flags::SCOPED) {
        return 1;
    }
    if (weapon->special_property_flags & 
        AttackSpecialProperty_Flags::CANNOT_MISS) {
        return 1;
    }
    
    if (BtlUnit_CheckStatus(target, StatusEffectType::INVISIBLE) ||
        (target->attribute_flags & BattleUnitAttribute_Flags::UNK_8) ||
        (target->attribute_flags & BattleUnitAttribute_Flags::VEILED)) {
        return 4;
    }
    if ((part->counter_attribute_flags & 
         PartsCounterAttribute_Flags::PREEMPTIVE_SPIKY) &&
        !((weapon->counter_resistance_flags & 
           AttackCounterResistance_Flags::PREEMPTIVE_SPIKY) ||
          attacker->badges_equipped.spike_shield)) {
        return 5;
    }
    if (part->part_attribute_flags & PartsAttribute_Flags::UNK_40) {
        return 4;
    }
    if (extra_params & 0x10'0000) {
        return 1;
    }
    if (CheckEvasionBadges(target)) {
        return 3;
    }
    
    if (BtlUnit_CheckStatus(target, StatusEffectType::DODGY) &&
        !BtlUnit_CheckStatus(target, StatusEffectType::SLEEP) &&
        !BtlUnit_CheckStatus(target, StatusEffectType::STOP) &&
        ttyd::system::irand(100) < 50) {
        return 6;
    }
    
    float accuracy = weapon->base_accuracy;
    if (BtlUnit_CheckStatus(attacker, StatusEffectType::DIZZY)) accuracy /= 2;
    if (ttyd::battle::g_BattleWork->stage_hazard_work.fog_active) accuracy /= 2;
    return ttyd::system::irand(100) < accuracy ? 1 : 2;
}

// Replaces the original damage function used by TTYD, for ease of editing.
// Called by Alter(Fp)DamageCalculation in patches_enemy.
int32_t CalculateBaseDamage(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    // For shorter / more readable code lines, lol.
    const auto& sp = weapon->special_property_flags;
    
    if (target->token_flags & BattleUnitToken_Flags::UNK_2) {
        unk1 |= 0x1000;
    }
    
    int32_t element = weapon->element;
    if (attacker &&
        (sp & AttackSpecialProperty_Flags::IGNITES_IF_BURNED) &&
        BtlUnit_CheckStatus(attacker, StatusEffectType::BURN)) {
        element = AttackElement::FIRE;
    }
    *unk0 = unk1 | 0x1e;
    
    if (!weapon->damage_function) {
        return 0;
    }
    if (attacker &&
        (sp & AttackSpecialProperty_Flags::BADGE_BUFFABLE) &&
        attacker->badges_equipped.all_or_nothing > 0 &&
        !(battleWork->ac_manager_work.ac_result & 2) &&
        !(unk1 & 0x20000)) {
        return 0;
    }
    // Immune to damage/status.
    if (part->part_attribute_flags & PartsAttribute_Flags::UNK_2000_0000) {
        return 0;
    }
    
    int32_t atk = weapon->damage_function_params[0];
    
    auto weapon_fn = (WeaponDamageFn)weapon->damage_function;
    if (weapon_fn) atk = weapon_fn(attacker, weapon, target, part);
    
    // Positive attack modifiers (badges and statuses).
    if (sp & AttackSpecialProperty_Flags::BADGE_BUFFABLE) {
        atk += attacker->badges_equipped.all_or_nothing;
        atk += attacker->badges_equipped.power_plus;
        atk += attacker->badges_equipped.p_up_d_down;
        
        const bool weaker_rush_badges =
            g_Mod->state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES);
        
        if (attacker->current_hp <= attacker->unit_kind_params->danger_hp) {
            const int32_t power = weaker_rush_badges ? 1 : 2;
            atk += power * attacker->badges_equipped.power_rush;
        }
        if (attacker->current_hp <= attacker->unit_kind_params->peril_hp) {
            const int32_t power = weaker_rush_badges ? 2 : 5;
            atk += power * attacker->badges_equipped.mega_rush;
        }
        if (part->part_attribute_flags & PartsAttribute_Flags::WEAK_TO_ICE_POWER) {
            atk += attacker->badges_equipped.ice_power;
        }
    }
    if (sp & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        int8_t strength = 0;
        BtlUnit_GetStatus(
            attacker, StatusEffectType::ATTACK_UP, nullptr, &strength);
        atk += strength;
        // ATK spell activated (doesn't check if Mario is attacker!)
        if (battleWork->impending_merlee_spell_type == 1) {
            atk += 3;
        }
    }
    if (sp & AttackSpecialProperty_Flags::CHARGE_BUFFABLE) {
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::CHARGE)) {
            int8_t strength = 0;
            BtlUnit_GetStatus(
                attacker, StatusEffectType::CHARGE, nullptr, &strength);
            atk += strength;
            attacker->token_flags |= BattleUnitToken_Flags::CHARGE_EXPENDED;
        }
    }
    
    // Negative attack modifiers (badges and statuses).
    if (sp & AttackSpecialProperty_Flags::BADGE_BUFFABLE) {
        atk -= attacker->badges_equipped.p_down_d_up;
        atk -= attacker->badges_equipped.hp_drain;
        atk -= attacker->badges_equipped.fp_drain;
    }
    if (sp & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        int8_t strength = 0;
        BtlUnit_GetStatus(
            attacker, StatusEffectType::ATTACK_DOWN, nullptr, &strength);
        atk += strength;
        
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::BURN)) {
            atk -= 1;
        }
    }
    if (atk < 0) atk = 0;
    
    // Attack multiplier statuses.
    if (sp & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::HUGE)) {
            // Round up.
            atk = ((atk * 3) + 1) / 2;
        } else if (atk > 1 &&
            BtlUnit_CheckStatus(attacker, StatusEffectType::TINY)) {
            // Round down (unless base is 1).
            atk /= 2;
        }
    }
    
    int32_t def = part->defense[element];
    int32_t def_attr = part->defense_attr[element];
    switch (element) {
        case AttackElement::NORMAL:
            *unk0 = unk1 | 0x17;
            break;
        case AttackElement::FIRE:
            *unk0 = unk1 | 0x18;
            break;
        case AttackElement::ICE:
            *unk0 = unk1 | 0x1a;
            break;
        case AttackElement::EXPLOSION:
            *unk0 = unk1 | 0x19;
            break;
        case AttackElement::ELECTRIC:
            *unk0 = unk1 | 0x1b;
            break;
    }
    
    // Defense modifiers.
    // If defense-piercing or elemental healing, set DEF to 0.
    if (sp & AttackSpecialProperty_Flags::DEFENSE_PIERCING or def_attr == 3) {
        def = 0;
    } else {
        def += target->badges_equipped.defend_plus;
        if (unk1 & 0x40000) {  // successful guard
            def += target->badges_equipped.damage_dodge;
        }
        if (target->status_flags & BattleUnitStatus_Flags::DEFENDING) {
            def += 1;
        }
        
        int8_t strength = 0;
        BtlUnit_GetStatus(
            target, StatusEffectType::DEFENSE_UP, nullptr, &strength);
        def += strength;
        BtlUnit_GetStatus(
            target, StatusEffectType::DEFENSE_DOWN, nullptr, &strength);
        def += strength;
        if (def < 0) def = 0;
        
        if (battleWork->impending_merlee_spell_type == 2 &&
            target->current_kind == BattleUnitType::MARIO) {
            def += 3;
        }
        
        if (part->part_attribute_flags & 
            PartsAttribute_Flags::WEAK_TO_ATTACK_FX_R) {
            def += target->unit_work[1];
            if (def < 0) def = 0;
        }
    }
    
    int32_t damage = atk - def;
    
    damage += target->badges_equipped.p_up_d_down;
    if (damage > 0) {
        if (sp & AttackSpecialProperty_Flags::DIMINISHING_BY_HIT) {
            damage -= target->hp_damaging_hits_dealt;
        }
        if (sp & AttackSpecialProperty_Flags::DIMINISHING_BY_TARGET) {
            damage -= attacker->hits_dealt_this_attack;
        }
        if (damage < 1) damage = 1;
    }
    if (unk1 & 0x40000) {  // guarding
        damage -= 1;
    }
    if (element == AttackElement::FIRE) {
        damage -= target->badges_equipped.ice_power;
    }
    damage -= target->badges_equipped.p_down_d_up;
    damage *= (target->badges_equipped.double_pain + 1);
    
    int32_t last_stands = target->badges_equipped.last_stand;
    if (target->current_hp <= target->unit_kind_params->danger_hp && 
        last_stands > 0) {
        damage = (damage + last_stands) / (last_stands + 1);
    }
    
    if (BtlUnit_CheckStatus(target, StatusEffectType::FREEZE) &&
        element != AttackElement::ICE) {
        damage *= 2;
    }
    
    switch (def_attr) {
        case 1:
            damage += 1;
            break;
        case 2:
            damage = 0;
            break;
        case 4:
            if (!(unk1 & 0x8000'0000)) damage = 0;
            break;
    }
    
    if (def_attr == 3) {
        damage *= -1;
        *unk0 |= 0x2000;
    } else if (damage < 1) {
        damage = 0;
        *unk0 = unk1 | 0x1e;
    } else if (
        (unk1 & 0x100) &&
        (sp & AttackSpecialProperty_Flags::UNKNOWN_0x8000 ||
         !(target->attribute_flags & BattleUnitAttribute_Flags::UNK_100))) {
        *unk0 |= 0x10000;
    }
    
    return damage;
}

void ApplyFixedPatches() {
    g_BattleActionCommandCheckDefence_trampoline = patch::hookFunction(
        ttyd::battle_ac::BattleActionCommandCheckDefence,
        [](BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Run normal logic if option turned off.
            const int32_t sp_cost =
                g_Mod->state_.GetOptionValue(OPTNUM_SUPERGUARD_SP_COST);
            if (sp_cost <= 0) {
                const int32_t defense_result =
                    g_BattleActionCommandCheckDefence_trampoline(unit, weapon);
                if (defense_result == 5) {
                    // Successful Superguard, track in play stats.
                    g_Mod->state_.ChangeOption(STAT_SUPERGUARDS);
                }
                return defense_result;
            }
            
            int8_t superguard_frames[7];
            bool restore_superguard_frames = false;
            // Temporarily disable Superguarding if SP is too low.
            if (ttyd::mario_pouch::pouchGetAP() < sp_cost) {
                restore_superguard_frames = true;
                memcpy(superguard_frames, ttyd::battle_ac::superguard_frames, 7);
                for (int32_t i = 0; i < 7; ++i) {
                    ttyd::battle_ac::superguard_frames[i] = 0;
                }
            }
            const int32_t defense_result =
                g_BattleActionCommandCheckDefence_trampoline(unit, weapon);
            if (defense_result == 5) {
                // Successful Superguard, subtract SP and track in play stats.
                ttyd::mario_pouch::pouchAddAP(-sp_cost);
                g_Mod->state_.ChangeOption(STAT_SUPERGUARDS);
            }
            if (restore_superguard_frames) {
                memcpy(ttyd::battle_ac::superguard_frames, superguard_frames, 7);
            }
            return defense_result;
        });
    
    // Replacing several core damage / status infliction functions.
    g_BattlePreCheckDamage_trampoline = patch::hookFunction(
        ttyd::battle_damage::BattlePreCheckDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target, 
            BattleWorkUnitPart* part, BattleWeapon* weapon, 
            uint32_t extra_params) {
                // Replaces vanilla logic completely.
                return PreCheckDamage(
                    attacker, target, part, weapon, extra_params);
            });
    g_BattleSetStatusDamageFromWeapon_trampoline = patch::hookFunction(
        ttyd::battle_damage::BattleSetStatusDamageFromWeapon, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target, 
            BattleWorkUnitPart* part, BattleWeapon* weapon, 
            uint32_t extra_params) {
                // Replaces vanilla logic completely.
                return GetStatusDamageFromWeapon(
                    attacker, target, part, weapon, extra_params);
            });
    g_BtlUnit_CheckRecoveryStatus_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_CheckRecoveryStatus, [](
            BattleWorkUnit* unit, int8_t status_type) {
                // Replaces vanilla logic completely.
                return StatusEffectTick(unit, status_type);
            });
            
    // Add support for Snow Whirled-like variant of button pressing AC.
    // Override choosing buttons:
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_ChooseButtons_BH),
        reinterpret_cast<void*>(StartButtonDownChooseButtons),
        reinterpret_cast<void*>(BranchBackButtonDownChooseButtons));
    // Reset buttons instead of ending attack when completed:
    mod::patch::writeBranch(
        reinterpret_cast<void*>(
            g_battleAcMain_ButtonDown_CheckComplete_BH),
        reinterpret_cast<void*>(StartButtonDownCheckComplete));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackButtonDownCheckComplete),
        reinterpret_cast<void*>(
            g_battleAcMain_ButtonDown_CheckComplete_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchButtonDownCheckComplete),
        reinterpret_cast<void*>(
            g_battleAcMain_ButtonDown_CheckComplete_CH1));
            
    // Add support for "permanent statuses" (turn count >= 100):
    // Skip drawing status icon if 100 or over:
    mod::patch::writeBranch(
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_BH),
        reinterpret_cast<void*>(StartStatusIconDisplay));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_CH1));
    
    // Toggle off Scoped status, and force freeze-break anytime a Frozen enemy
    // is hit by a non-Ice attack.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCheckDamage_AlwaysFreezeBreak_BH),
        reinterpret_cast<void*>(StartToggleScopedAndCheckFreezeBreak),
        reinterpret_cast<void*>(BranchBackToggleScopedAndCheckFreezeBreak));
            
    // Skip drawing Huge / Tiny status arrows when inflicted.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_SkipHugeTinyArrows),
        0x2c19000cU /* cmpwi r25, 12 */);
        
    // Increase all forms of Payback-esque status returned damage to 1x.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_PaybackDivisor),
        0x38000032U /* li r0, 50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_HoldFastDivisor),
        0x38000032U /* li r0, 50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_ReturnPostageDivisor),
        0x38000032U /* li r0, 50 */);
        
    // Change frame windows for guarding / Superguarding at different levels
    // of Simplifiers / Unsimplifiers to be more symmetric.
    const int8_t kGuardFrames[] =     { 12, 10, 9, 8, 7, 6, 5, 0 };
    const int8_t kSuperguardFrames[]  = { 5, 4, 4, 3, 2, 2, 1, 0 };
    mod::patch::writePatch(
        ttyd::battle_ac::guard_frames, kGuardFrames, sizeof(kGuardFrames));
    mod::patch::writePatch(
        ttyd::battle_ac::superguard_frames, kSuperguardFrames, 
        sizeof(kSuperguardFrames));
        
    // Override the default target audience size.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_SetTargetAmount_BH),
        reinterpret_cast<void*>(StartSetTargetAudienceCount),
        reinterpret_cast<void*>(BranchBackSetTargetAudienceCount));
        
    // Apply the option to change the amount of SP regained from attacks.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_ApRecoveryBuild_NoBingoRegen_BH),
        reinterpret_cast<void*>(StartApplySpRegenMultiplierNoBingo),
        reinterpret_cast<void*>(BranchBackApplySpRegenMultiplierNoBingo));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_ApRecoveryBuild_BingoRegen_BH),
        reinterpret_cast<void*>(StartApplySpRegenMultiplierBingo),
        reinterpret_cast<void*>(BranchBackApplySpRegenMultiplierBingo));
}

void SetTargetAudienceAmount() {
    uintptr_t audience_work_base =
        reinterpret_cast<uintptr_t>(
            ttyd::battle::g_BattleWork->audience_work);
    float target_amount = 200.0f;
    // If set to rank up by progression, make the target audience follow suit;
    // otherwise, keep the target fixed at max capacity.
    if (g_Mod->state_.GetOptionValue(OPTVAL_STAGE_RANK_30_FLOORS)) {
        const int32_t floor = g_Mod->state_.floor_;
        target_amount = floor >= 195 ? 200.0f : floor + 5.0f;
    }
    *reinterpret_cast<float*>(audience_work_base + 0x13778) = target_amount;
}

double ApplySpRegenMultiplier(double base_regen) {
    return base_regen * 
        g_Mod->state_.GetOptionNumericValue(OPTNUM_SP_REGEN_MODIFIER) / 20.0;
}

void ToggleScopedStatus(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWeapon* weapon) {
    if (attacker->current_kind <= BattleUnitType::BONETAIL) return;
    
    // If the player attempted to land damage or a negative/neutral status.
    bool player_attempted_attack = false;
    if (weapon->damage_function) {
        player_attempted_attack = true;
    } else {
        if (weapon->sleep_time || 
            weapon->stop_time ||
            weapon->dizzy_time ||
            weapon->poison_time ||
            weapon->confuse_time ||
            weapon->burn_time ||
            weapon->freeze_time ||
            weapon->size_change_time ||
            weapon->atk_change_time ||
            weapon->def_change_time ||
            weapon->slow_time ||
            weapon->allergic_time ||
            weapon->fright_chance ||
            weapon->gale_force_chance ||
            weapon->ohko_chance) {
            player_attempted_attack = true;
        }
    }
    if (player_attempted_attack) {
        target->status_flags &= ~BattleUnitStatus_Flags::SCOPED;
    }
}

void QueueCustomStatusMessage(BattleWorkUnit* unit, const char* announce_msg) {
    // Queue custom message, using status 1 (Stop)'s unused no-effect entry.
    ttyd::battle_status_effect::_st_chg_msg_data[2].msg_no_effect = announce_msg;
    ttyd::battle_status_effect::BattleStatusChangeInfoSetAnnouce(
        unit, /* placeholder status + turns */ 1, 1, /* no effect */ 0);
    ttyd::battle_status_effect::BattleStatusChangeMsgSetAnnouce(
        unit, /* placeholder status */ 1, /* no effect */ 0);
}

// Applies a custom status effect to the target.
// Params: unit, part, status_flag, color1, color2, sfx_name, announce_msg
EVT_DEFINE_USER_FUNC(evtTot_ApplyCustomStatus) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t part_idx = evtGetValue(evt, evt->evtArguments[1]);
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    auto* part = ttyd::battle::BattleGetUnitPartsPtr(unit_idx, part_idx);
    
    uint32_t status_flag = evtGetValue(evt, evt->evtArguments[2]);
    uint32_t color1 = evtGetValue(evt, evt->evtArguments[3]);
    uint32_t color2 = evtGetValue(evt, evt->evtArguments[4]);
    const char* sfx_name = (const char*)evtGetValue(evt, evt->evtArguments[5]);
    const char* announce_msg = (const char*)evtGetValue(evt, evt->evtArguments[6]);
    
    // Apply status flag.
    unit->status_flags |= status_flag;
    
    // Spawn splash effect.
    gc::vec3 pos;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
    // Adjust position for center point based on whether on ceiling.
    if ((unit->attribute_flags & 2) == 0) {
        pos.y += unit->height / 2;
    } else {
        pos.y -= unit->height / 2;
    }
    switch (unit->current_kind) {
        case BattleUnitType::HOOKTAIL:
        case BattleUnitType::GLOOMTAIL:
        case BattleUnitType::BONETAIL:
            pos.x -= 300;
            pos.z += 30;
            break;
        case BattleUnitType::SHADOW_QUEEN_PHASE_2:
            pos.z += 40;
            break;
    }
    auto* eff = ttyd::eff_stamp_n64::effStampN64Entry(
        pos.x, pos.y, pos.z + 10.0f, 2);
    // Apply user-determined colors to effect.
    ((char*)eff->eff_work)[0x38] = color1 >> 16;
    ((char*)eff->eff_work)[0x39] = (color1 >> 8) & 0xff;
    ((char*)eff->eff_work)[0x3a] = color1        & 0xff;
    ((char*)eff->eff_work)[0x3b] = color2 >> 16;
    ((char*)eff->eff_work)[0x3c] = (color2 >> 8) & 0xff;
    ((char*)eff->eff_work)[0x3d] = color2        & 0xff;
    
    // Play sound effect.
    ttyd::battle_unit::BtlUnit_GetHitPos(unit, part, &pos.x, &pos.y, &pos.z);
    ttyd::pmario_sound::psndSFXOn_3D(sfx_name, &pos);
    
    // Queue custom status message.
    QueueCustomStatusMessage(unit, announce_msg);
    
    return 2;
}

}  // namespace battle
}  // namespace mod::infinite_pit