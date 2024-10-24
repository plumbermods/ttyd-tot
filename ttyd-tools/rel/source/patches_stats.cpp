#include "patches_stats.h"

#include "common_functions.h"
#include "mod.h"
#include "patch.h"
#include "tot_manager_achievements.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>

#include <cstdint>

namespace mod::tot::patch {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_BtlUnit_PayWeaponCost_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern int32_t (*g_pouchEquipBadgeIndex_trampoline)(int32_t);
extern int16_t (*g_pouchAddStarPiece_trampoline)(int16_t);
extern int16_t (*g_pouchSetCoin_trampoline)(int16_t);
extern int16_t (*g_pouchAddCoin_trampoline)(int16_t);
extern void (*g_BtlActRec_AddCount_trampoline)(uint8_t*);

namespace stats {
    
void ApplyFixedPatches() {        
    g_BtlUnit_PayWeaponCost_trampoline = mod::hookFunction(
        ttyd::battle_unit::BtlUnit_PayWeaponCost, [](
            BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Track FP / SP spent.
            const int32_t fp_cost = BtlUnit_GetWeaponCost(unit, weapon);
            g_Mod->state_.ChangeOption(STAT_RUN_FP_SPENT, fp_cost);
            g_Mod->state_.ChangeOption(STAT_PERM_FP_SPENT, fp_cost);
            g_Mod->state_.ChangeOption(STAT_RUN_SP_SPENT, weapon->base_sp_cost);
            g_Mod->state_.ChangeOption(STAT_PERM_SP_SPENT, weapon->base_sp_cost);
            // Run normal pay-weapon-cost logic.
            g_BtlUnit_PayWeaponCost_trampoline(unit, weapon);
        });

    g_pouchEquipBadgeIndex_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchEquipBadgeIndex, [](int32_t index) {
            // Track number of badges equipped.
            g_Mod->state_.ChangeOption(STAT_RUN_BADGES_EQUIPPED, 1);
            // Run normal badge equipping logic.
            return g_pouchEquipBadgeIndex_trampoline(index);
        });

    g_pouchAddStarPiece_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchAddStarPiece, [](int16_t star_pieces) {
            // Cap at 999 or 9,999 based on whether in run.
            const int32_t cap = 
                g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 999 : 9999;
            auto* pouch = ttyd::mario_pouch::pouchGetPtr();
            pouch->star_pieces = Clamp(
                static_cast<int32_t>(pouch->star_pieces + star_pieces), 0, cap);
            return pouch->star_pieces;
        });

    g_pouchSetCoin_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchSetCoin, [](int16_t coins) {
            // Cap at 999 or 9,999 based on whether in run.
            const int32_t cap = 
                g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 999 : 9999;
            auto* pouch = ttyd::mario_pouch::pouchGetPtr();
            pouch->coins = Clamp(static_cast<int32_t>(coins), 0, cap);
            return pouch->coins;
        });

    g_pouchAddCoin_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchAddCoin, [](int16_t coins) {
            // Actually add coins; cap at 999 or 9,999 based on whether in run.
            const int32_t cap =
                g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 999 : 9999;
            auto* pouch = ttyd::mario_pouch::pouchGetPtr();
            pouch->coins =
                Clamp(static_cast<int32_t>(pouch->coins + coins), 0, cap);

            // Track coins gained / spent during runs.
            // (Bandits stealing coins count as negative coins gained.)
            if (g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                if (coins < 0 && !ttyd::mariost::g_MarioSt->bInBattle) {
                    g_Mod->state_.ChangeOption(STAT_RUN_COINS_SPENT, -coins);
                    g_Mod->state_.ChangeOption(STAT_PERM_COINS_SPENT, -coins);
                } else {
                    g_Mod->state_.ChangeOption(STAT_RUN_COINS_EARNED, coins);
                    g_Mod->state_.ChangeOption(STAT_PERM_COINS_EARNED, coins);
                    AchievementsManager::CheckCompleted(
                        AchievementId::MISC_RUN_COINS_999);
                }
            }

            return pouch->coins;
        });

    g_BtlActRec_AddCount_trampoline = mod::hookFunction(
        ttyd::battle_actrecord::BtlActRec_AddCount, [](uint8_t* counter) {
            auto& actRecordWork = ttyd::battle::g_BattleWork->act_record_work;
            // Track every time an item is used by the player in-battle.
            if (counter == &actRecordWork.mario_num_times_attack_items_used ||
                counter == &actRecordWork.mario_num_times_non_attack_items_used ||
                counter == &actRecordWork.partner_num_times_attack_items_used ||
                counter == &actRecordWork.partner_num_times_non_attack_items_used) {
                g_Mod->state_.ChangeOption(STAT_RUN_ITEMS_USED);
                g_Mod->state_.ChangeOption(STAT_PERM_ITEMS_USED);
            }
            // Run act record counting logic.
            g_BtlActRec_AddCount_trampoline(counter); 
        });
}

}  // namespace stats
}  // namespace mod::tot::patch