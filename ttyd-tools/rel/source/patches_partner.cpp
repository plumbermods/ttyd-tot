#include "patches_partner.h"

#include "common_functions.h"
#include "custom_enemy.h"
#include "custom_item.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_battle.h"
#include "tot_party_bobbery.h"
#include "tot_party_flurrie.h"
#include "tot_party_goombella.h"
#include "tot_party_koops.h"
#include "tot_party_mowz.h"
#include "tot_party_vivian.h"
#include "tot_party_yoshi.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>
#include <ttyd/unit_bomzou.h>
#include <ttyd/unit_koura.h>
#include <ttyd/unit_party_christine.h>
#include <ttyd/unit_party_chuchurina.h>
#include <ttyd/unit_party_clauda.h>
#include <ttyd/unit_party_nokotarou.h>
#include <ttyd/unit_party_sanders.h>
#include <ttyd/unit_party_vivian.h>
#include <ttyd/unit_party_yoshi.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // tattle_disp_patches.s
    void StartDispTattleStats();
    void BranchBackDispTattleStats();
    
    void dispTattleStats(
        gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color,
        ttyd::battle_unit::BattleWorkUnit* unit) {
        mod::infinite_pit::partner::DisplayTattleStats(
            matrix, number, is_small, color, unit);
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_weapon_power::weaponGetPowerDefault;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

// Global, toggles whether to show/hide ATK/DEF stats below enemies.
// Can be toggled on or off freely with 'Z' button, but is reset each battle.
bool g_ShowAtkDefThisFloor = true;
bool g_JustPressedZ = false;

}

// Function hooks.
extern int32_t (*g__make_madowase_weapon_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_get_monosiri_msg_no_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_BattleDrawEnemyHP_DrawEnemyHPText_BH;
extern const int32_t g_koura_pose_tbl_reset_Patch_HeavyDmg;
extern const int32_t g_koura_pose_tbl_reset_Patch_LightDmg;
extern const int32_t g_koura_damage_core_Patch_HeavyDmg;
extern const int32_t g_koura_damage_core_Patch_LightDmg;
extern const int32_t g_subsetevt_blow_dead_Patch_GetRewards;
extern const int32_t g_BattleSetStatusDamage_Patch_GaleLevelFactor;
extern const int32_t g_partyClauda_makeTechMenuFuncPtr;
extern const int32_t g_partyYoshi_makeTechMenuFuncPtr;
extern const int32_t g_partyChuchurina_makeTechMenuFuncPtr;
extern const int32_t g_partySanders_makeTechMenuFuncPtr;
extern const int32_t g_partyVivian_makeTechMenuFuncPtr;
extern const int32_t g_partyNokotarou_makeTechMenuFuncPtr;
extern const int32_t g_partyChristine_makeTechMenuFuncPtr;

namespace partner {
    
namespace {

// Patch to disable the coins / EXP from Gale Force for most enemies.
// (Replaces the code with no-ops).  Needs to be patched into enemies' specific
// death events in some cases as well.
EVT_BEGIN(GaleForceKillPatch)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(GaleForceKillPatch) == 0x38);

// Patch to disable getting Star Power early from certain attacks;
// battle::AwardStarPowerAndResetFaceDirection will be used to award it
// at the end of the attack instead, to make sure Stylishes are counted.
EVT_BEGIN(DeclareStarPowerPatch)
DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(DeclareStarPowerPatch) == 0x10);

}
    
void ApplyFixedPatches() {
    // Reset whether to show ATK/DEF stats below enemies.
    g_ShowAtkDefThisFloor = true;
    
    // Tattle returns a custom message based on the enemy's stats.
    g_btlevtcmd_get_monosiri_msg_no_trampoline = mod::patch::hookFunction(
        ttyd::unit_party_christine::btlevtcmd_get_monosiri_msg_no,
        [](EvtEntry* evt, bool isFirstCall) {
            auto* battleWork = ttyd::battle::g_BattleWork;
            int32_t unit_idx = evtGetValue(evt, evt->evtArguments[0]);
            unit_idx = ttyd::battle_sub::BattleTransID(evt, unit_idx);
            auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
            
            // Get original pointer to Tattle string.
            g_btlevtcmd_get_monosiri_msg_no_trampoline(evt, isFirstCall);
            const char* tattle_msg = 
                reinterpret_cast<const char*>(
                    evtGetValue(evt, evt->evtArguments[2]));
            // Build a custom tattle, if the enemy has stats to pull from.
            tattle_msg = SetCustomTattle(unit, tattle_msg);
            evtSetValue(evt, evt->evtArguments[2], PTR(tattle_msg));
            return 2;
        });

    // Calls a custom function to display ATK / DEF under HP if a unit has
    // previously been Tattled (and if it's currently the player's turn to act).
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleDrawEnemyHP_DrawEnemyHPText_BH),
        reinterpret_cast<void*>(StartDispTattleStats),
        reinterpret_cast<void*>(BranchBackDispTattleStats));
        
    // Set HP thresholds for different Shell Shield disrepair animation states:
    // - On initialization (pose_tbl_reset)
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_koura_pose_tbl_reset_Patch_HeavyDmg), 1);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_koura_pose_tbl_reset_Patch_LightDmg), 2);
    // - On damage (damage_core)
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_koura_damage_core_Patch_HeavyDmg), 1);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_koura_damage_core_Patch_LightDmg), 2);
    
    // Disable getting coins and experience from a successful Gale Force.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_subsetevt_blow_dead_Patch_GetRewards),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    // Remove the (Mario - enemy level) adjustment to Gale Force's chance.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleSetStatusDamage_Patch_GaleLevelFactor),
        0x60000000U /* nop */);
        
    // Increase Tease's base status rate to 1.27x and make it inflict Confuse.
    g__make_madowase_weapon_trampoline = mod::patch::hookFunction(
        ttyd::unit_party_chuchurina::_make_madowase_weapon,
        [](EvtEntry* evt, bool isFirstCall) {
            g__make_madowase_weapon_trampoline(evt, isFirstCall);
            BattleWeapon& weapon = 
                *reinterpret_cast<BattleWeapon*>(evt->lwData[12]);
            weapon.confuse_chance = weapon.dizzy_chance * 1.27;
            weapon.confuse_time = 3;
            weapon.dizzy_chance = 0;
            weapon.dizzy_time = 0;
            return 2;
        });
    
    // Bomb Squad explosions pierce Defense.
    ttyd::unit_bomzou::weapon_bomzou_explosion.special_property_flags |= 0x40;
        
    // TOT: Replace party member weapon selection functions.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyChristine_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_goombella::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyNokotarou_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_koops::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyClauda_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_flurrie::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyYoshi_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_yoshi::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyVivian_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_vivian::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partySanders_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_bobbery::MakeSelectWeaponTable));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyChuchurina_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_mowz::MakeSelectWeaponTable));
}

void DisplayTattleStats(
    gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color,
    BattleWorkUnit* unit) {
    // Pressing Z during the player action phase toggles on/off ATK+DEF display.
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::Z) {
        if (!g_JustPressedZ) {
            g_ShowAtkDefThisFloor = !g_ShowAtkDefThisFloor;
            g_JustPressedZ = true;
        }
    } else {
        g_JustPressedZ = false;
    }
        
    // If enemy has been Tattled (Peekaboo does not count), 
    // display the enemy's ATK and DEF underneath their HP.
    bool show_atk_def =
        (ttyd::swdrv::swGet(0x117a + unit->true_kind) ||
        ttyd::swdrv::swGet(0x117a + unit->current_kind));
    // If cheat is enabled, force it to be on. (TODO: toggle on/off with Z?)
    if (g_Mod->state_.GetOptionNumericValue(OPT_SHOW_ATK_DEF) ||
        g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
        show_atk_def = true;
    }
    // Hide ATK / DEF outside player action phase (or if hidden for this floor).
    if (!(ttyd::battle::g_BattleWork->battle_flags & 0x80) ||
        !g_ShowAtkDefThisFloor) {
        show_atk_def = false;
    }
       
    if (show_atk_def) {
        int32_t atk, def;
        // If the enemy's atk and def aren't fetched, just draw HP normally.
        if (!GetTattleDisplayStats(unit->current_kind, &atk, &def)) {
            ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
            return;
        }
        atk = Clamp(atk, 0, 99);
        def = Clamp(def, 0, 99);

        // Undo alignment adjustment the game normally does for small numbers.
        if (number < 100) matrix->m[0][3] += 4.0f;
        if (number < 10) matrix->m[0][3] += 4.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
        
        // Draw ATK and DEF numbers.
        uint32_t color_atk = 0xffa0a0ffU;
        uint32_t color_def = 0xc0c0ffffU;
        matrix->m[1][3] -= 20.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, def, is_small, &color_def);
        matrix->m[0][3] -= 16.0f * (def > 9 ? 3 : 2) - 4.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, atk, is_small, &color_atk);
        // Draw slash in-between ATK and DEF.
        matrix->m[0][3] += 14.0f;
        ttyd::icondrv::iconDispGxCol(matrix, 0x10, 0x1e0, color);
    } else {
        // Otherwise, just draw HP.
        ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
    }
}

void RefreshExtraTattleStats() {
    g_ShowAtkDefThisFloor = true;
}

EVT_DEFINE_USER_FUNC(InitializePartyMember) {
    const int32_t starting_rank =
        g_Mod->state_.GetOptionNumericValue(OPT_PARTNER_RANK);
    const int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    const int16_t starting_hp =
        ttyd::mario_pouch::_party_max_hp_table[idx * 4 + starting_rank];
    const int32_t hp_plus_p_cnt =
        ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::HP_PLUS_P);
    auto& party_data = ttyd::mario_pouch::pouchGetPtr()->party_data[idx];
    party_data.base_max_hp = starting_hp;
    party_data.max_hp = starting_hp + hp_plus_p_cnt * 5;
    party_data.current_hp = starting_hp + hp_plus_p_cnt * 5;
    party_data.hp_level = starting_rank;
    party_data.attack_level = starting_rank;
    party_data.tech_level = starting_rank;
    return 2;
}

}  // namespace partner
}  // namespace mod::infinite_pit