#include "mod_cheats.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"
#include "mod.h"
#include "mod_debug.h"
#include "mod_menu.h"
#include "mod_state.h"
#include "tot_state.h"

#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {
    
namespace {
    
namespace ItemType = ::ttyd::item_data::ItemType;

// Constants for secret codes.
uint32_t secretCode_BumpAttack      = 043652131;
uint32_t secretCode_BgmOnOff        = 031313141;
uint32_t secretCode_RtaTimer        = 034345566;
uint32_t secretCode_ShowAtkDef      = 023122312;
uint32_t secretCode_UnlockFxBadges  = 026122146;
uint32_t secretCode_ObfuscateItems  = 046362123;
uint32_t secretCode_DebugMode       = 036363636;

}

void CheatsManager::Update() {
    // Process cheat codes.
    static uint32_t code_history = 0;
    int32_t code = 0;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::A) code = 1;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::B) code = 2;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::L) code = 3;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::R) code = 4;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::X) code = 5;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::Y) code = 6;
    if (code) code_history = (code_history << 3) | code;
    
    if ((code_history & 0xFFFFFF) == secretCode_RtaTimer) {
        code_history = 0;
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        // Toggle between timer options.
        int32_t option = g_Mod->state_.GetOption(tot::OPT_TIMER_DISPLAY);
        option = (option + 1) % 3;
        g_Mod->state_.SetOption(tot::OPT_TIMER_DISPLAY, option);
    }
    if ((code_history & 0xFFFFFF) == secretCode_BgmOnOff) {
        code_history = 0;
        // Toggle on/off background music from playing or starting.
        bool toggle = !g_Mod->state_.GetOption(tot::OPT_BGM_DISABLED);
        g_Mod->state_.SetOption(tot::OPT_BGM_DISABLED, toggle);
        if (toggle) {
            ttyd::pmario_sound::psndStopAllFadeOut();
        }
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_ShowAtkDef) {
        code_history = 0;
        bool toggle = !g_Mod->state_.GetOption(tot::OPT_SHOW_ATK_DEF);
        g_Mod->state_.SetOption(tot::OPT_SHOW_ATK_DEF, toggle);
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_UnlockFxBadges) {
        code_history = 0;
        // Check Journal for whether the FX badges were already unlocked.
        bool has_fx_badges = ttyd::swdrv::swGet(
            ItemType::ATTACK_FX_R - ItemType::POWER_JUMP + 0x80);
        if (!has_fx_badges && ttyd::mario_pouch::pouchGetHaveBadgeCnt() < 196) {
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_P);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_G);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_B);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_Y);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_R);
            ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        }
    }
    if ((code_history & 0xFFFFFF) == secretCode_BumpAttack) {
        code_history = 0;
        // Check Journal for whether the badge was already unlocked.
        bool has_fx_badges = ttyd::swdrv::swGet(
            ItemType::BUMP_ATTACK - ItemType::POWER_JUMP + 0x80);
        if (!has_fx_badges && ttyd::mario_pouch::pouchGetHaveBadgeCnt() < 200) {
            ttyd::mario_pouch::pouchGetItem(ItemType::BUMP_ATTACK);
            ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        }
    }
    if ((code_history & 0xFFFFFF) == secretCode_ObfuscateItems) {
        code_history = 0;
        if (InMainGameModes() && 
            !g_Mod->state_.GetOption(tot::OPT_RUN_STARTED)) {
            g_Mod->state_.ChangeOption(tot::OPT_OBFUSCATE_ITEMS);
            ttyd::sound::SoundEfxPlayEx(0x3c, 0, 0x64, 0x40);
        }
    }

    // TODO: Disable this before public releases.
    if ((code_history & 0xFFFFFF) == secretCode_DebugMode) {
        code_history = 0;
        DebugManager::ChangeMode();
        g_Mod->state_.SetOption(tot::OPT_DEBUG_MODE_USED, 1);
    }
}

void CheatsManager::Draw() {}

}