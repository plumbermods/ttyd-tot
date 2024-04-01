#include "patches_field.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_core.h"
#include "patches_partner.h"

#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/evt_badgeshop.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_shop.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_win.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

namespace {
    
using ::ttyd::battle_database_common::BattleUnitSetup;
using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::mario_pouch::PouchData;
using ::ttyd::npcdrv::NpcBattleInfo;
using ::ttyd::npcdrv::NpcEntry;
using ::ttyd::npcdrv::NpcTribeDescription;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
// Patch addresses.
extern const int32_t g_select_disp_Patch_PitListPriceHook;
extern const int32_t g_select_disp_Patch_PitItemPriceHook;
extern const int32_t g_jon_setup_npc_ex_para_FuncOffset;
extern const int32_t g_jon_yattukeFlag_FuncOffset;
extern const int32_t g_jon_evt_raster_FuncOffset;
extern const int32_t g_jon_init_evt_MoverSetupHookOffset;
extern const int32_t g_jon_enemy_100_Offset;
extern const int32_t g_jon_evt_open_box_EvtOffset;
extern const int32_t g_jon_move_evt_EvtOffset;
extern const int32_t g_jon_talk_idouya_EvtOffset;
extern const int32_t g_jon_npcEnt_idouya_Offset;
extern const int32_t g_jon_talk_gyousyou_MinItemForBadgeDialogOffset;
extern const int32_t g_jon_talk_gyousyou_NoInvSpaceBranchOffset;
extern const int32_t g_jon_dokan_open_PipeOpenEvtOffset;
extern const int32_t g_jon_evt_kanban2_ReturnSignEvtOffset;
extern const int32_t g_jon_floor_inc_EvtOffset;
extern const int32_t g_jon_bero_boss_EntryBeroEntryOffset;
extern const int32_t g_jon_bero_boss_ReturnBeroEntryOffset;
extern const int32_t g_jon_setup_boss_EvtOffset;
extern const int32_t g_jon_bero_return_ReturnBeroEntryOffset;
extern const int32_t g_jon_zonbaba_first_event_EvtOffset;
extern const int32_t g_jon_btlsetup_jon_tbl_Offset;
extern const int32_t g_jon_unit_boss_zonbaba_battle_entry_event;

namespace field {

namespace {

// Global variables and constants.

const char kPitNpcName[] = "\x93\x47";  // "enemy"
const char kPiderName[] = "\x83\x70\x83\x43\x83\x5f\x81\x5b\x83\x58";
const char kArantulaName[] = 
    "\x83\x60\x83\x85\x83\x89\x83\x93\x83\x5e\x83\x89\x81\x5b";
const char kChainChompName[] = "\x83\x8f\x83\x93\x83\x8f\x83\x93";
const char kRedChompName[] = 
    "\x83\x6f\x81\x5b\x83\x58\x83\x67\x83\x8f\x83\x93\x83\x8f\x83\x93";
const char kBonetailName[] = "\x83\x5d\x83\x93\x83\x6f\x83\x6f";
const char kChetRippoName[] =
    "\x83\x70\x83\x8f\x81\x5b\x83\x5f\x83\x45\x83\x93\x89\xae";
const char kMoverName[] = "\x88\xda\x93\xae\x89\xae";
const char kMoverTribeName[] = "\x83\x76\x83\x6a\x8f\xee\x95\xf1\x89\xae";

ttyd::npcdrv::NpcSetupInfo g_ChetRippoNpcSetupInfo[2];
    
// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(GetNumChestRewards, 1)
EVT_DECLARE_USER_FUNC(GetChestReward, 1)
EVT_DECLARE_USER_FUNC(FullyHealParty, 0)
EVT_DECLARE_USER_FUNC(CheckRewardClaimed, 1)
EVT_DECLARE_USER_FUNC(CheckPromptSave, 1)
EVT_DECLARE_USER_FUNC(IncrementInfinitePitFloor, 1)
EVT_DECLARE_USER_FUNC(GetUniqueItemName, 1)
EVT_DECLARE_USER_FUNC(AddItemStarPower, 2)
EVT_DECLARE_USER_FUNC(CheckChetRippoSpawn, 1)
EVT_DECLARE_USER_FUNC(CheckMoverSpawn, 1)
EVT_DECLARE_USER_FUNC(CheckAnyStatsDowngradeable, 1)
EVT_DECLARE_USER_FUNC(CheckStatDowngradeable, 2)
EVT_DECLARE_USER_FUNC(DowngradeStat, 1)
EVT_DECLARE_USER_FUNC(TrackChetRippoSellActionType, 1)
EVT_DECLARE_USER_FUNC(GetMoverSelectionParams, 5)
EVT_DECLARE_USER_FUNC(GetExitBeroName, 1)
EVT_DECLARE_USER_FUNC(UpdateExitDestinationImpl, 1)

// Event that plays "get partner" fanfare.
EVT_BEGIN(PartnerFanfareEvt)
USER_FUNC(ttyd::evt_snd::evt_snd_bgmoff, 0x400)
USER_FUNC(ttyd::evt_snd::evt_snd_bgmon, 1, PTR("BGM_FF_GET_PARTY1"))
WAIT_MSEC(2000)
RETURN()
EVT_END()

// Event that handles a chest being opened, rewarding the player with
// items / partners (1 ~ 5 based on the mod's settings, +1 for boss floors).
EVT_BEGIN(ChestOpenEvt)
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
USER_FUNC(GetNumChestRewards, LW(13))
DO(0)
    SUB(LW(13), 1)
    IF_SMALL(LW(13), 0)
        // Fully heal the player's party (if option is set).
        USER_FUNC(FullyHealParty)
        DO_BREAK()
    END_IF()
    USER_FUNC(GetChestReward, LW(1))
    // If reward < 0, then reward a partner (-1 to -7 = partners 1 to 7).
    IF_SMALL(LW(1), 0)
        MUL(LW(1), -1)
        WAIT_MSEC(100)  // If the second+ reward
        USER_FUNC(ttyd::evt_mobj::evt_mobj_wait_animation_end, PTR("box"))
        USER_FUNC(ttyd::evt_mario::evt_mario_normalize)
        USER_FUNC(ttyd::evt_mario::evt_mario_goodbye_party, 0)
        WAIT_MSEC(500)
        USER_FUNC(ttyd::evt_pouch::evt_pouch_party_join, LW(1))
        USER_FUNC(partner::InitializePartyMember, LW(1))
        // TODO: Reposition partner by box? At origin is fine...
        USER_FUNC(ttyd::evt_mario::evt_mario_set_party_pos, 0, LW(1), 0, 0, 0)
        RUN_EVT_ID(PartnerFanfareEvt, LW(11))
        USER_FUNC(
            ttyd::evt_eff::evt_eff,
            PTR("sub_bg"), PTR("itemget"), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_toge, 1, 0, 0, 0)
        USER_FUNC(
            ttyd::evt_msg::evt_msg_print, 0, PTR("pit_reward_party_join"), 0, 0)
        CHK_EVT(LW(11), LW(12))
        IF_EQUAL(LW(12), 1)
            DELETE_EVT(LW(11))
            USER_FUNC(ttyd::evt_snd::evt_snd_bgmoff, 0x201)
        END_IF()
        USER_FUNC(ttyd::evt_eff::evt_eff_softdelete, PTR("sub_bg"))
        USER_FUNC(ttyd::evt_snd::evt_snd_bgmon, 0x120, 0)
        USER_FUNC(
            ttyd::evt_snd::evt_snd_bgmon_f, 0x300, PTR("BGM_STG0_100DN1"), 1500)
        WAIT_MSEC(500)
        USER_FUNC(ttyd::evt_party::evt_party_run, 0)
        USER_FUNC(ttyd::evt_party::evt_party_run, 1)
    ELSE()
        // Reward is an item; spawn it item normally.
        USER_FUNC(
            ttyd::evt_mobj::evt_mobj_get_position,
            PTR("box"), LW(10), LW(11), LW(12))
        USER_FUNC(GetUniqueItemName, LW(14))
        USER_FUNC(
            ttyd::evt_item::evt_item_entry,
            LW(14), LW(1), LW(10), LW(11), LW(12), 17, -1, 0)
        WAIT_MSEC(300)  // If the second+ reward
        USER_FUNC(ttyd::evt_mobj::evt_mobj_wait_animation_end, PTR("box"))
        USER_FUNC(ttyd::evt_item::evt_item_get_item, LW(14))
        // If the item was a Crystal Star / Magical Map, unlock its Star Power.
        // If this is the first level 2+ move unlocked, show the level tutorial.
        USER_FUNC(AddItemStarPower, LW(1), LW(11))
        IF_EQUAL(LW(11), 1)
            USER_FUNC(
                ttyd::evt_msg::evt_msg_print, 0, PTR("pit_move_level"), 0, 0)
        END_IF()
    END_IF()
WHILE()
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
RETURN()
EVT_END()

// Wrapper for modified chest-opening event.
EVT_BEGIN(ChestOpenEvtHook)
RUN_CHILD_EVT(ChestOpenEvt)
RETURN()
EVT_END()

// Event that sets up the boss floor (adds a signboard by the chest).
EVT_BEGIN(BossSetupEvt)
SET(LW(0), REL_PTR(ModuleId::JON, g_jon_bero_boss_EntryBeroEntryOffset))
USER_FUNC(ttyd::evt_bero::evt_bero_get_info)
RUN_CHILD_EVT(ttyd::evt_bero::evt_bero_info_run)
USER_FUNC(
    ttyd::evt_mobj::evt_mobj_signboard, PTR("board"), 190, 0, 200,
    REL_PTR(ModuleId::JON, g_jon_evt_kanban2_ReturnSignEvtOffset), LSWF(0))
RETURN()
EVT_END()

// Wrapper for modified boss floor setup event.
EVT_BEGIN(BossSetupEvtHook)
RUN_CHILD_EVT(BossSetupEvt)
RETURN()
EVT_END()

// Event that states an exit is disabled.
EVT_BEGIN(DisabledBeroEvt)
INLINE_EVT()
USER_FUNC(ttyd::evt_cam::evt_cam_ctrl_onoff, 4, 0)
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
USER_FUNC(ttyd::evt_bero::evt_bero_exec_wait, 0x10000)
WAIT_MSEC(750)
USER_FUNC(ttyd::evt_msg::evt_msg_print, 0, PTR("pit_disabled_return"), 0, 0)
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
USER_FUNC(ttyd::evt_cam::evt_cam_ctrl_onoff, 4, 1)
END_INLINE()
SET(LW(0), 0)
RETURN()
EVT_END()

// Event that runs before advancing to the next floor.
// On reward floors, checks to see if the player has claimed their reward, then
// prompts the player to save.  If conditions are met to continue, increments
// the floor counter in the mod's state, as well as GSW(1321).
EVT_BEGIN(FloorIncrementEvt)
SET(LW(0), GSW(1321))
ADD(LW(0), 1)
MOD(LW(0), 10)
IF_EQUAL(LW(0), 0)
    USER_FUNC(CheckRewardClaimed, LW(0))
    IF_EQUAL(LW(0), 0)
        // If reward not claimed, disable player from continuing.
        SET(LW(0), 1)
        USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
        WAIT_MSEC(50)
        USER_FUNC(
            ttyd::evt_msg::evt_msg_print, 0, PTR("pit_chest_unclaimed"), 0, 0)
        WAIT_MSEC(50)
        USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
    ELSE()
        USER_FUNC(CheckPromptSave, LW(0))
        IF_EQUAL(LW(0), 1)
            // If prompting the user to save, disable player from continuing.
            RUN_CHILD_EVT(ttyd::evt_mobj::mobj_save_blk_sysevt)
            SET(LW(0), 1)
        ELSE()    
            SET(LW(0), 0)
            USER_FUNC(IncrementInfinitePitFloor, 1)
        END_IF()
    END_IF()
ELSE()
    // Set LW(0) back to 0 to allow going through pipe.
    SET(LW(0), 0)
    USER_FUNC(IncrementInfinitePitFloor, 1)
END_IF()
RETURN()
EVT_END()

// Wrapper for modified floor-incrementing event.
EVT_BEGIN(FloorIncrementEvtHook)
RUN_CHILD_EVT(FloorIncrementEvt)
RETURN()
EVT_END()

// Chet Rippo item-selling event.
EVT_BEGIN(ChetRippoSellItemsEvent)
USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_no_items"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_which_item"))
USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
USER_FUNC(ttyd::evt_window::evt_win_item_select, 1, 3, LW(1), LW(4))
IF_SMALL_EQUAL(LW(1), 0)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_msg::evt_msg_print,
        0, PTR("rippo_exit"), 0, PTR(kChetRippoName))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_shop::name_price, LW(1), LW(2), LW(3))
USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("rippo_item_ok"), LW(3))
USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR(kChetRippoName))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_different"))
    GOTO(0)
END_IF()
USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
USER_FUNC(TrackChetRippoSellActionType, 0)
USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
WAIT_MSEC(200)
USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_thanks_last"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_thanks_next"))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_exit"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_continue)
USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
GOTO(0)
RETURN()
EVT_END()

// Chet Rippo badge-selling event.
EVT_BEGIN(ChetRippoSellBadgesEvent)
USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_no_badges"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_which_item"))
USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
USER_FUNC(ttyd::evt_window::evt_win_other_select, 12)
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_msg::evt_msg_print,
        0, PTR("rippo_exit"), 0, PTR(kChetRippoName))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("rippo_item_ok"), LW(3))
USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR(kChetRippoName))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_different"))
    GOTO(0)
END_IF()
USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
USER_FUNC(TrackChetRippoSellActionType, 1)
USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
WAIT_MSEC(200)
USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_thanks_last"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_item_thanks_next"))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_exit"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_continue)
USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
GOTO(0)
RETURN()
EVT_END()

// Chet Rippo stat-selling event.
EVT_BEGIN(ChetRippoSellStatsEvent)
USER_FUNC(CheckAnyStatsDowngradeable, LW(0))
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_no_stats"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_which_stat"))
LBL(0)
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_stat_menu"))
SWITCH(LW(0))
    CASE_EQUAL(3)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_exit"))
        RETURN()
END_SWITCH()
USER_FUNC(CheckStatDowngradeable, LW(0), LW(1))
IF_EQUAL(LW(1), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_stat_too_low"))
    GOTO(0)
END_IF()
IF_EQUAL(LW(1), 2)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_no_free_bp"))
    RETURN()
END_IF()
SWITCH(LW(0))
    CASE_EQUAL(0)
        SET(LW(1), PTR("rippo_confirm_hp"))
    CASE_EQUAL(1)
        SET(LW(1), PTR("rippo_confirm_fp"))
    CASE_ETC()
        SET(LW(1), PTR("rippo_confirm_bp"))
END_SWITCH()
SET(LW(2), LW(0))
USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, LW(1))
USER_FUNC(TrackChetRippoSellActionType, 2)
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_stat_different"))
    GOTO(0)
END_IF()
USER_FUNC(DowngradeStat, LW(2))
USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, 39)
USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
WAIT_MSEC(200)
USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
USER_FUNC(CheckAnyStatsDowngradeable, LW(0))
IF_EQUAL(LW(0), 0)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_stat_thanks_last"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_stat_thanks_next"))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_yes_no"))
IF_EQUAL(LW(0), 1)
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_exit"))
    RETURN()
END_IF()
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_which_stat"))
GOTO(0)
RETURN()
EVT_END()

// Chet Rippo NPC talking event.
EVT_BEGIN(ChetRippoTalkEvt)
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
USER_FUNC(ttyd::evt_win::unitwin_get_work_ptr, LW(10))
USER_FUNC(ttyd::evt_msg::evt_msg_print,
    0, PTR("rippo_intro"), 0, PTR(kChetRippoName))
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("rippo_top_menu"))
SWITCH(LW(0))
    CASE_EQUAL(0)
        RUN_CHILD_EVT(ChetRippoSellItemsEvent)
    CASE_EQUAL(1)
        RUN_CHILD_EVT(ChetRippoSellBadgesEvent)
    CASE_EQUAL(2)
        RUN_CHILD_EVT(ChetRippoSellStatsEvent)
    CASE_EQUAL(3)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("rippo_exit"))
END_SWITCH()
USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
RETURN()
EVT_END()

// Mover NPC talking event.
EVT_BEGIN(MoverTalkEvt)
USER_FUNC(ttyd::evt_msg::evt_msg_print, 0, PTR("msg_jon_mover"), 0, 0)
USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("msg_jon_mover_select"))
// If window cancelled, end conversation.
IF_EQUAL(LW(0), 3)
    USER_FUNC(ttyd::evt_msg::evt_msg_continue)
    RETURN()
END_IF()
// LW(1) = coin cost, LW(2) = floors to skip, LW(3) = map, LW(4) = bero.
USER_FUNC(GetMoverSelectionParams, LW(0), LW(1), LW(2), LW(3), LW(4))
USER_FUNC(ttyd::evt_pouch::evt_pouch_get_coin, LW(5))
IF_SMALL(LW(5), LW(1))
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("msg_jon_mover_nocoin"))
    RETURN()
END_IF()
MUL(LW(1), -1)
USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(1))
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 1, PTR("<scrl_auto><once_stop>"))
USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("msg_jon_mover_2"))
INLINE_EVT()
    UNCHECKED_USER_FUNC(REL_PTR(ModuleId::JON, g_jon_evt_raster_FuncOffset))
END_INLINE()
WAIT_MSEC(5000)
USER_FUNC(ttyd::evt_fade::evt_fade_set_mapchange_type, 0, 10, 300, 9, 300)
USER_FUNC(ttyd::evt_fade::evt_fade_set_mapchange_type, 1, 10, 300, 9, 300)
USER_FUNC(ttyd::evt_mario::evt_mario_goodbye_party, 0)
USER_FUNC(IncrementInfinitePitFloor, LW(2))
USER_FUNC(ttyd::evt_bero::evt_bero_mapchange, LW(3), LW(4))
RETURN()
EVT_END()

// Wrapper for Move talk event.
EVT_BEGIN(MoverTalkEvtHook)
RUN_CHILD_EVT(MoverTalkEvt)
RETURN()
EVT_END()

// Chet Rippo / Mover NPC spawning event.
EVT_BEGIN(ChetRippoMoverSetupEvt)
USER_FUNC(CheckChetRippoSpawn, LW(0))
IF_EQUAL(LW(0), 1)
    USER_FUNC(
        ttyd::evt_npc::evt_npc_entry, PTR(kChetRippoName), PTR("c_levela"))
    USER_FUNC(ttyd::evt_npc::evt_npc_set_tribe,
        PTR(kChetRippoName), PTR(kChetRippoName))
    USER_FUNC(ttyd::evt_npc::evt_npc_setup, PTR(g_ChetRippoNpcSetupInfo))
    USER_FUNC(ttyd::evt_npc::evt_npc_set_position,
        PTR(kChetRippoName), -160, 0, 110)
END_IF()
USER_FUNC(CheckMoverSpawn, LW(0))
IF_EQUAL(LW(0), 1)
    USER_FUNC(
        ttyd::evt_npc::evt_npc_entry, PTR(kMoverName), PTR("c_pjoho"))
    USER_FUNC(ttyd::evt_npc::evt_npc_set_tribe,
        PTR(kMoverName), PTR(kMoverTribeName))
    USER_FUNC(ttyd::evt_npc::evt_npc_setup, 
        REL_PTR(ModuleId::JON, g_jon_npcEnt_idouya_Offset))
    USER_FUNC(ttyd::evt_npc::evt_npc_set_position,
        PTR(kMoverName), -100, 0, 0)
END_IF()
RETURN()
EVT_PATCH_END()

// "Mario alone" fight start event for fighting Bonetail with no partner.
EVT_BEGIN(BonetailMarioAloneEntryEvt)
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_prilimit, 1)
USER_FUNC(ttyd::evt_map::evt_map_replayanim, 0, PTR("dontyo"))
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_mode, 1, 3)
USER_FUNC(
    ttyd::battle_camera::evt_btl_camera_set_moveto, 
    1, 0, 110, 1080, 0, 93, -2, 1, 0)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetStageSize, 
    LW(6), EVT_HELPER_POINTER_BASE, EVT_HELPER_POINTER_BASE)
MUL(LW(6), -1)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetHomePos, -3, LW(0), LW(1), LW(2))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_SetPos, -3, LW(6), LW(1), LW(2))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetStageSize,
    EVT_HELPER_POINTER_BASE, LW(3), EVT_HELPER_POINTER_BASE)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
ADD(LW(1), LW(3))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
WAIT_FRM(60)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetHomePos, -3, LW(0), LW(1), LW(2))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_SetMoveSpeed, -3, FLOAT(6.00))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_MovePosition,
    -3, LW(0), LW(1), LW(2), 0, -1, 0)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_Y"))
WAIT_FRM(2)
USER_FUNC(
    ttyd::battle_event_cmd::btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_F_3"))
WAIT_FRM(1)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
WAIT_FRM(10)
INLINE_EVT()
    WAIT_FRM(5)
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_O"))
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_SetFallAccel, -3, FLOAT(0.30))
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_GetPos, -3, LW(0), LW(1), LW(2))
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_snd_se, -3, 
        PTR("SFX_VOICE_MARIO_SURPRISED2_2"),
        EVT_HELPER_POINTER_BASE, 0, EVT_HELPER_POINTER_BASE)
    USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_JumpPosition,
        -3, LW(0), LW(1), LW(2), 25, -1)
END_INLINE()
USER_FUNC(ttyd::evt_snd::evt_snd_sfxon, PTR("SFX_BOSS_GNB_APPEAR1"), 0)
USER_FUNC(ttyd::battle_camera::evt_btl_camera_shake_h, 1, 8, 0, 20, 13)
WAIT_FRM(30)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_Y"))
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_mode, 1, 3)
USER_FUNC(
    ttyd::battle_camera::evt_btl_camera_set_moveto,
    1, -233, 45, 452, 56, 125, 37, 60, 0)
WAIT_FRM(60)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StatusWindowOnOff, 0)
USER_FUNC(ttyd::evt_msg::evt_msg_print, 2, PTR("tik_boss_12"), 0, -2)
WAIT_MSEC(300)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartWaitEvent, -3)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartWaitEvent, -2)
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StatusWindowOnOff, 1)
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_prilimit, 0)
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_mode, 0, 0)
USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_moveSpeedLv, 0, 3)
RETURN()
EVT_END()

// Updates a floor's exit destination to be consistent with the floor number.
EVT_BEGIN(UpdateExitDestinationEvt)
USER_FUNC(GetExitBeroName, LW(0))
RUN_CHILD_EVT(ttyd::evt_bero::bero_case_switch_off)
USER_FUNC(GetExitBeroName, LW(0))
USER_FUNC(UpdateExitDestinationImpl, LW(0))
RUN_CHILD_EVT(ttyd::evt_bero::bero_case_switch_on)
RETURN()
EVT_END()

// Returns the number of chest rewards to spawn based on the floor number.
EVT_DEFINE_USER_FUNC(GetNumChestRewards) {
    int32_t num_rewards = g_Mod->inf_state_.GetOptionNumericValue(INF_OPT_CHEST_REWARDS);
    if (num_rewards > 0) {
        // Add a bonus reward for beating a boss (Atomic Boo or Bonetail).
        if (g_Mod->inf_state_.floor_ % 50 == 49) ++num_rewards;
    } else {
        // Pick a number of rewards randomly from 1 ~ 7.
        num_rewards = g_Mod->inf_state_.Rand(7, INF_RNG_CHEST) + 1;
    }
    evtSetValue(evt, evt->evtArguments[0], num_rewards);
    return 2;
}

// Returns the item or partner to spawn from the chest on a Pit reward floor.
EVT_DEFINE_USER_FUNC(GetChestReward) {
    // DEPRECATED: Deleted custom_chest_reward.
    evtSetValue(evt, evt->evtArguments[0], ItemType::QUICK_CHANGE);
    return 2;
}

// Fully heals the player's party.
EVT_DEFINE_USER_FUNC(FullyHealParty) {
    if (!g_Mod->inf_state_.GetOptionNumericValue(INF_OPT_DISABLE_CHEST_HEAL)) {
        PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
        pouch.current_hp = pouch.max_hp;
        pouch.current_fp = pouch.max_fp;
        pouch.current_sp = pouch.max_sp;
        for (int32_t i = 0; i < 8; ++i) {
            pouch.party_data[i].current_hp = pouch.party_data[i].max_hp;
        }
    }
    return 2;
}

// Returns whether or not the current floor's reward has been claimed.
EVT_DEFINE_USER_FUNC(CheckRewardClaimed) {
    bool reward_claimed = false;
    for (uint32_t i = 0x13d3; i <= 0x13dc; ++i) {
        if (ttyd::swdrv::swGet(i)) reward_claimed = true;
    }
    evtSetValue(evt, evt->evtArguments[0], reward_claimed);
    return 2;
}

// Returns whether or not to prompt the player to save.
EVT_DEFINE_USER_FUNC(CheckPromptSave) {
    evtSetValue(evt, evt->evtArguments[0], core::GetShouldPromptSave());
    if (core::GetShouldPromptSave()) {
        g_Mod->inf_state_.SaveCurrentTime();
        g_Mod->inf_state_.Save();
        core::SetShouldPromptSave(false);
    }
    return 2;
}

// Increments the actual current Pit floor, and the corresponding GSW value.
EVT_DEFINE_USER_FUNC(IncrementInfinitePitFloor) {
    int32_t inc = evtGetValue(evt, evt->evtArguments[0]);
    int32_t actual_floor = g_Mod->inf_state_.floor_ += inc;
    // Update the floor number used by the game.
    // Floors 101+ are treated as looping 81-90 nine times + 91-100.
    int32_t gsw_floor = actual_floor;
    if (actual_floor >= 100) {
        gsw_floor = actual_floor % 10;
        gsw_floor += ((actual_floor / 10) % 10 == 9) ? 90 : 80;
    }
    ttyd::swdrv::swByteSet(1321, gsw_floor);
    if (g_Mod->inf_state_.CheckOptionValue(INF_OPTVAL_STAGE_RANK_30_FLOORS)) {
        // Set stage rank based on passing floors 30, 60, 90.
        int32_t rank = 0;
        if (actual_floor >= 90) {
            rank = 3;
        } else if (actual_floor >= 60) {
            rank = 2;
        } else if (actual_floor >= 30) {
            rank = 1;
        }
        ttyd::mario_pouch::pouchGetPtr()->rank = rank;
    }
    if (inc > 1) {
        // If more than one floor incremented, a Mover must have been used.
        g_Mod->inf_state_.ChangeOption(INF_STAT_MOVERS_USED, 1);
        // Track how many battles were skipped by taking a Mover.
        // Ignore the current floor, as well as a rest floor if one was passed.
        int32_t skipped_battles = inc - 1;
        if (actual_floor % 10 < inc) --skipped_battles;
        g_Mod->inf_state_.ChangeOption(INF_STAT_BATTLES_SKIPPED, skipped_battles);
    }
    return 2;
}

// Gets a unique id for items that spawn from chests.
EVT_DEFINE_USER_FUNC(GetUniqueItemName) {
    static int32_t id = 0;
    static char name[16];
    
    id = (id + 1) % 1000;
    sprintf(name, "ch_item_%03" PRId32, id);
    evtSetValue(evt, evt->evtArguments[0], PTR(name));
    return 2;
}

// If the item is a Crystal Star, gives the player +0.50 max SP (to a max of 10)
// and enable / level-up the item's respective Star Power.
EVT_DEFINE_USER_FUNC(AddItemStarPower) {
    int16_t item = evtGetValue(evt, evt->evtArguments[0]);
    if (item == ItemType::MAGICAL_MAP ||
        (item >= ItemType::DIAMOND_STAR && item <= ItemType::CRYSTAL_STAR)) {
        PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
        if (pouch.max_sp < 1000) pouch.max_sp += 50;
        pouch.current_sp = pouch.max_sp;
        
        int32_t star_power_type =
            item == ItemType::MAGICAL_MAP ? 0 : item - ItemType::DIAMOND_STAR + 1;
        pouch.star_powers_obtained |= (1 << star_power_type);
        g_Mod->inf_state_.star_power_levels_ += (1 << (2 * star_power_type));
        
        // Check whether to show the Star Power level tutorial.
        if (g_Mod->inf_state_.GetStarPowerLevel(star_power_type) > 1 &&
            !g_Mod->inf_state_.GetOptionNumericValue(INF_OPT_SEEN_MOVE_LEVEL_TUT)) {
            evtSetValue(evt, evt->evtArguments[1], 1);
            g_Mod->inf_state_.SetOption(INF_OPT_SEEN_MOVE_LEVEL_TUT, true);
        }
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(CheckChetRippoSpawn) {
    uint32_t floor = g_Mod->inf_state_.floor_ + 1;
    bool can_spawn = floor % 10 == 0 && floor % 100 != 0;
    switch (g_Mod->inf_state_.GetOptionValue(INF_OPT_CHET_RIPPO_APPEARANCE)) {
        case INF_OPTVAL_CHET_RIPPO_GUARANTEE: {
            break;
        }
        case INF_OPTVAL_CHET_RIPPO_RANDOM: {
            // Chance of spawning is higher the farther you progress into the
            // Pit, capping at 75% per reward floor.
            if (can_spawn) {
                if (floor > 75) floor = 75;
                can_spawn = g_Mod->inf_state_.Rand(100, INF_RNG_CHET_RIPPO) < floor;
            }
            break;
        }
    }
    evtSetValue(evt, evt->evtArguments[0], can_spawn);
    return 2;
}

EVT_DEFINE_USER_FUNC(CheckMoverSpawn) {
    // Default to assuming a Mover will not spawn.
    evtSetValue(evt, evt->evtArguments[0], false);
    
    // If Movers are disabled, they cannot spawn.
    if (!g_Mod->inf_state_.GetOptionNumericValue(INF_OPT_MOVERS_ENABLED)) return 2;
    
    uint32_t floor = g_Mod->inf_state_.floor_ + 1;
    // Cannot spawn on rest floors.
    if (floor % 10 == 0) return 2;
    // Cannot spawn < floor 30 (so you're forced to have a partner, if enabled).
    if (floor < 30) return 2;
    // Cannot spawn on Atomic Boo's floor (he can still be skipped, though).
    if (floor % 100 == 49) return 2;
    // Cannot spawn if even the cheapest warp would skip Bonetail.
    // (custom_strings ensures that no options appear that would skip Bonetail).
    if (floor % 100 > 97) return 2;
    
    // Assuming all other criteria are met, spawn Movers 10% of the time, unless
    // naive RNG checks of the previous 5 floors show that one would've spawned.
    // This should make Movers appear on ~5% of eligible floors, similar to
    // TTYD, without allowing them to appear in quick succession if skipped.
    
    // Failed the RNG check for this floor.
    if (g_Mod->inf_state_.Rand(100, INF_RNG_MOVER) >= 10) return 2;
    for (int32_t i = 0; i < 5; ++i) {
        if (g_Mod->inf_state_.Rand(100, INF_RNG_MOVER) < 10) {
            // Passed the RNG check for a recent prior floor.
            return 2;
        }
    }

    // A Mover should spawn; mark one battle skipped automatically (this floor).
    evtSetValue(evt, evt->evtArguments[0], true);
    g_Mod->inf_state_.ChangeOption(INF_STAT_BATTLES_SKIPPED, 1);
    return 2;
}

// Returns whether any stat upgrades can be sold.
EVT_DEFINE_USER_FUNC(CheckAnyStatsDowngradeable) {
    bool can_downgrade = ttyd::mario_pouch::pouchGetPtr()->level > 1 &&
        !g_Mod->inf_state_.GetOptionNumericValue(INF_OPT_NO_EXP_MODE);
    evtSetValue(evt, evt->evtArguments[0], can_downgrade);
    return 2;
}

// Returns whether the stat is high enough to downgrade, or 2 if the stat is
// high enough but there isn't enough free BP available to downgrade.
EVT_DEFINE_USER_FUNC(CheckStatDowngradeable) {
    PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
    int32_t downgrade_state = 0;
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 0: {
            downgrade_state = pouch.base_max_hp > 10 ? 1 : 0;
            break;
        }
        case 1: {
            downgrade_state = pouch.base_max_fp > 5 ? 1 : 0;
            break;
        }
        case 2: {
            if (pouch.total_bp > 3) {
                downgrade_state = pouch.unallocated_bp >= 3 ? 1 : 2;
            }
            break;
        }
    }
    evtSetValue(evt, evt->evtArguments[1], downgrade_state);
    return 2;
}

// Downgrades the selected stat.
EVT_DEFINE_USER_FUNC(DowngradeStat) {
    PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 0: {
            pouch.base_max_hp -= 5;
            break;
        }
        case 1: {
            pouch.base_max_fp -= 5;
            break;
        }
        case 2: {
            pouch.total_bp -= 3;
            pouch.unallocated_bp -= 3;
            break;
        }
    }
    pouch.level -= 1;
    ttyd::mario_pouch::pouchReviseMarioParam();
    return 2;
}

// Tracks item / badge / level sold actions in play stats.
EVT_DEFINE_USER_FUNC(TrackChetRippoSellActionType) {
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 0:     g_Mod->inf_state_.ChangeOption(INF_STAT_ITEMS_SOLD);    break;
        case 1:     g_Mod->inf_state_.ChangeOption(INF_STAT_BADGES_SOLD);   break;
        case 2:     g_Mod->inf_state_.ChangeOption(INF_STAT_LEVELS_SOLD);   break;
    }
    return 2;
}

// Returns the name of the map used for a given floor (one-indexed).
const char* GetFloorMapName(int32_t floor) {
    const char* map;
    if (floor % 10) {
        if (floor < 50) {
            map = "jon_00";
        } else if (floor < 80) {
            map = "jon_01";
        } else {
            map = "jon_02";
        }
    } else {
        if (floor < 50) {
            map = "jon_03";
        } else if (floor < 80) {
            map = "jon_04";
        } else if (floor % 100) {
            map = "jon_05";
        } else {
            map = "jon_06";
        }
    }
    return map;
}

// Gets the coin cost, floors skipped, and warp destination for a Mover option.
EVT_DEFINE_USER_FUNC(GetMoverSelectionParams) {
    int32_t floor = g_Mod->inf_state_.floor_ + 1;
    int32_t cost = (floor > 90 ? 90 : floor) / 10 + 1;
    int32_t floors_to_increment = 0;
    
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 0: {
            cost *= 5;
            floors_to_increment = 3;
            break;
        }
        case 1: {
            cost *= 10;
            floors_to_increment = 5;
            break;
        }
        case 2: {
            cost *= 20;
            floors_to_increment = 10;
            break;
        }
        // Should not be reached.
        default: return 2;
    }
    
    // Determine which floor to warp to based on the destination floor.
    const char* map = GetFloorMapName(floor + floors_to_increment);
    
    evtSetValue(evt, evt->evtArguments[1], cost);
    evtSetValue(evt, evt->evtArguments[2], floors_to_increment);
    evtSetValue(evt, evt->evtArguments[3], PTR(map));
    evtSetValue(evt, evt->evtArguments[4], PTR("dokan_2"));
    return 2;
}

EVT_DEFINE_USER_FUNC(GetExitBeroName) {
    // Get the name of the exit loading zone for the current map.
    // (Chest floors' exits use a different name from others).
    const char* exit_bero = "dokan_1";
    const char* current_map = GetCurrentMap();
    if (!strcmp(current_map, "jon_03") || !strcmp(current_map, "jon_04") ||
        !strcmp(current_map, "jon_05")) {
        exit_bero = "dokan_3";
    }
    evtSetValue(evt, evt->evtArguments[0], PTR(exit_bero));
    return 2;
}

EVT_DEFINE_USER_FUNC(UpdateExitDestinationImpl)  {
    const char* exit_bero =
        reinterpret_cast<const char*>(evtGetValue(evt, evt->evtArguments[0]));
        
    // Get the correct map name for the next floor number.
    int32_t floor = g_Mod->inf_state_.floor_ + 1;
    const char* next_map = GetFloorMapName(floor + 1);
    
    // Update the destination of the exit loading zone to match the floor.
    BeroEntry** entries = ttyd::evt_bero::BeroINFOARR;
    for (int32_t i = 0; i < 16; ++i) {
        if (entries[i] && !strcmp(entries[i]->name, exit_bero)) {
            entries[i]->target_map = next_map;
            entries[i]->target_bero = "dokan_2";
            break;
        }
    }
    return 2;
}

}

void ApplyFixedPatches() {        
    // Correcting heights in NPC tribe description data.
    NpcTribeDescription* tribe_descs = ttyd::npc_data::npcTribe;
    // Shady Paratroopa
    tribe_descs[291].height = 30;
    // Fire Bro
    tribe_descs[293].height = 40;
    // Boomerang Bro
    tribe_descs[294].height = 40;
    // Craw
    tribe_descs[298].height = 40;
    // Atomic Boo
    tribe_descs[148].height = 100;
    
    // Copying tribe description data for Bob-omb, Atomic Boo over slots for
    // Bald + Hyper Bald Clefts, so they can be used for variants.
    memcpy(&tribe_descs[238], &tribe_descs[283], sizeof(NpcTribeDescription));
    memcpy(&tribe_descs[288], &tribe_descs[148], sizeof(NpcTribeDescription));
    // Set unique names + model filenames.
    tribe_descs[238].nameJp = "hyper_bomb";
    tribe_descs[238].modelName = "c_bomhey_h";
    tribe_descs[288].nameJp = "cosmic_boo";
    tribe_descs[288].modelName = "c_atmic_trs_p";
    
    // TODO: Move to patches_ui.h?
    // Fix captures / location information in Tattle menu.
    auto* tattle_inf = ttyd::battle_monosiri::battleGetUnitMonosiriPtr(0);
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].model_name = "c_atmic_trs_p";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].pose_name = "Z_1";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].location_name = "menu_monosiri_shiga";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].model_name = "c_bomhey_h";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].pose_name = "BOM_Z_1";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].location_name = "menu_monosiri_shiga";
}

void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id) {
    if (module_id != ModuleId::JON || !module_ptr) return;
    const uint32_t module_start = reinterpret_cast<uint32_t>(module_ptr);
    
    // Reset RNG states that reset every floor.
    StateManager_v2& state = g_Mod->inf_state_;
    state.rng_sequences_[INF_RNG_CHEST] = 0;
    state.rng_sequences_[INF_RNG_ENEMY] = 0;
    state.rng_sequences_[INF_RNG_ITEM] = 0;
    state.rng_sequences_[INF_RNG_CONDITION] = 0;
    state.rng_sequences_[INF_RNG_CONDITION_ITEM] = 0;
    state.rng_sequences_[INF_RNG_CHET_RIPPO] = 0;
        
    // Clear current floor turn count.
    state.SetOption(INF_STAT_MOST_TURNS_CURRENT, 0);
    
    // Apply custom logic to box opening event to allow spawning partners.
    mod::patch::writePatch(
        reinterpret_cast<void*>(module_start + g_jon_evt_open_box_EvtOffset),
        ChestOpenEvtHook, sizeof(ChestOpenEvtHook));
        
    // Patch over boss floor setup script to spawn a sign in the boss room.
    mod::patch::writePatch(
        reinterpret_cast<void*>(module_start + g_jon_setup_boss_EvtOffset),
        BossSetupEvtHook, sizeof(BossSetupEvtHook));
        
    // Disable reward floors' return pipe and display a message if entered.
    BeroEntry* return_bero = reinterpret_cast<BeroEntry*>(
        module_start + g_jon_bero_return_ReturnBeroEntryOffset);
    return_bero->target_map = nullptr;
    return_bero->target_bero = return_bero->name;
    return_bero->out_evt_code = reinterpret_cast<void*>(
        const_cast<int32_t*>(DisabledBeroEvt));
    
    // Change destination of boss room pipe to be the usual 80+ floor.
    BeroEntry* boss_bero = reinterpret_cast<BeroEntry*>(
        module_start + g_jon_bero_boss_ReturnBeroEntryOffset);
    boss_bero->target_map = "jon_02";
    boss_bero->target_bero = "dokan_2";
    boss_bero->out_evt_code = reinterpret_cast<void*>(
        module_start + g_jon_floor_inc_EvtOffset);
    
    // Update the actual Pit floor alongside GSW(1321); also, check for
    // unclaimed rewards and prompt the player to save on X0 floors.
    mod::patch::writePatch(
        reinterpret_cast<void*>(module_start + g_jon_floor_inc_EvtOffset),
        FloorIncrementEvtHook, sizeof(FloorIncrementEvtHook));
            
    // Set Chet Rippo NPC information.
    memset(&g_ChetRippoNpcSetupInfo, 0, sizeof(g_ChetRippoNpcSetupInfo));
    g_ChetRippoNpcSetupInfo[0].nameJp = kChetRippoName;
    g_ChetRippoNpcSetupInfo[0].flags = 0x10000600;
    g_ChetRippoNpcSetupInfo[0].regularEvtCode = nullptr;
    g_ChetRippoNpcSetupInfo[0].talkEvtCode =
        const_cast<int32_t*>(ChetRippoTalkEvt);
    // Replace call to vanilla Mover init event with call to custom
    // Chet Rippo / Mover setup function.
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            module_start + g_jon_init_evt_MoverSetupHookOffset),
        reinterpret_cast<int32_t>(ChetRippoMoverSetupEvt));
    // Patch Mover NPC talk hook event on top of the vanilla Mover talk event.
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            module_start + g_jon_talk_idouya_EvtOffset),
        MoverTalkEvtHook, sizeof(MoverTalkEvtHook));
    
    // If normal fight floor, reset Pit-related flags and save-related status.
    if (state.floor_ % 10 != 9) {
        // Clear "chest open" and "Bonetail beaten" flags.
        for (uint32_t i = 0x13d3; i <= 0x13dd; ++i) {
            ttyd::swdrv::swClear(i);
        }
        
        core::SetShouldPromptSave(true);

        const PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
        bool has_partner = false;
        for (int32_t i = 0; i < 8; ++i) {
            if (pouch.party_data[i].flags & 1) {
                has_partner = true;
                break;
            }
        }
        // Enable "P" badges only after obtaining the first one.
        state.SetOption(INF_OPT_ENABLE_P_BADGES, has_partner);
        // Only one partner / upgrade reward each allowed per reward floor,
        // re-enable them for the next reward floor.
        state.SetOption(INF_OPT_ENABLE_UPGRADE_REWARD, true);
        state.SetOption(INF_OPT_ENABLE_PARTNER_REWARD, true);
    } else if (state.floor_ % 100 == 99) {
        // If Bonetail floor, patch in the Mario-alone variant of the
        // battle entry event if partners are not available.
        const PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
        bool has_partner = false;
        for (int32_t i = 0; i < 8; ++i) {
            if (pouch.party_data[i].flags & 1) {
                has_partner = true;
                break;
            }
        }
        if (!has_partner) {
            mod::patch::writePatch(
                reinterpret_cast<void*>(
                    module_start + g_jon_unit_boss_zonbaba_battle_entry_event),
                BonetailMarioAloneEntryEvt, sizeof(BonetailMarioAloneEntryEvt));
        }
    }
}

void LinkCustomEvts(void* module_ptr, ModuleId::e module_id, bool link) {
    if (module_id != ModuleId::JON || !module_ptr) return;
    
    if (link) {
        LinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(BossSetupEvt));
        LinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(ChetRippoMoverSetupEvt));
        LinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(MoverTalkEvt));
    } else {
        UnlinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(BossSetupEvt));
        UnlinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(ChetRippoMoverSetupEvt));
        UnlinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(MoverTalkEvt));
    }
}

void UpdateExitDestination() {
    ttyd::evtmgr::evtEntry(const_cast<int32_t*>(UpdateExitDestinationEvt), 0, 0);
}

}  // namespace field
}  // namespace mod::infinite_pit