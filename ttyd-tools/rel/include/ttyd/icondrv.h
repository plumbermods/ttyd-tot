#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::icondrv {
    
namespace IconType {
    enum e {
        ZAP_TAP = 0,
        MONEY_MONEY,
        W_EMBLEM,
        L_EMBLEM,
        FLOWER_FINDER,
        FP_PLUS,
        FP_DRAIN,
        FEELING_FINE,
        HEART_FINDER,
        HP_PLUS,
        HP_DRAIN,
        QUAKE_HAMMER,
        MEGA_QUAKE,
        PIERCING_BLOW,
        HAMMER_THROW,
        FIRE_DRIVE,
        HEAD_RATTLE,
        HAPPY_FLOWER,
        ITEM_HOG,
        ICE_POWER,
        DOUBLE_PAIN,
        ATTACK_FX_G,
        ATTACK_FX_B,
        ATTACK_FX_R,
        ATTACK_FX_P,
        ATTACK_FX_Y,
        CHILL_OUT,
        PRETTY_LUCKY,
        SPIKE_SHIELD,
        FIRST_ATTACK,
        BUMP_ATTACK,
        MULTIBOUNCE,
        POWER_BOUNCE,
        TORNADO_JUMP,
        SHRINK_STOMP,
        SLEEPY_STOMP,
        SOFT_STOMP,
        CHARGE,
        SUPER_CHARGE,
        DOUBLE_DIP,
        TRIPLE_DIP,
        QUICK_CHANGE,
        JUMPMAN,
        HAMMERMAN,
        DEFEND_BADGE,
        HAPPY_HEART,
        FLOWER_SAVER,
        POWER_PLUS,
        DEFEND_PLUS,
        DAMAGE_DODGE,
        PEEKABOO,
        TIMING_TUTOR,
        P_DOWN_D_UP,
        P_UP_D_DOWN,
        ALL_OR_NOTHING,
        MEGA_RUSH,
        LAST_STAND,
        CLOSE_CALL,
        LUCKY_DAY,
        PITY_FLOWER,
        UNUSED_ICE_BADGE,
        REFUND,
        LUCKY_START,
        RETURN_POSTAGE,
        SLOW_GO,
        SIMPLIFIER,
        UNSIMPLIFIER,
        MEGA_JUMP,
        POWER_JUMP,
        POWER_SMASH,
        MEGA_SMASH,
        SUPER_APPEAL,
        ICE_SMASH,
        POWER_RUSH,
        POWER_RUSH_D,
        POWER_RUSH_K,
        POWER_RUSH_A,
        POWER_RUSH_C,
        P_UP_D_DOWN_P,
        P_DOWN_D_UP_P,
        FEELING_FINE_P,
        LUCKY_DAY_P,
        PRETTY_LUCKY_P,
        CLOSE_CALL_P,
        LAST_STAND_P,
        FP_DRAIN_P,
        DEFEND_PLUS_P,
        DEFEND_BADGE_P,
        HP_PLUS_P,
        DAMAGE_DODGE_P,
        HP_DRAIN_P,
        HAPPY_HEART_P,
        FLOWER_SAVER_P,
        POWER_PLUS_P,
        HAPPY_FLOWER_P,
        CHARGE_P,
        SUPER_CHARGE_P,
        SUPER_APPEAL_P,
        MEGA_RUSH_P,
        ALL_OR_NOTHING_P,
        DOUBLE_DIP_P,
        TRIPLE_DIP_P,
        PITY_FLOWER_P,
        POWER_RUSH_P,
        POWER_RUSH_D_P,
        POWER_RUSH_K_P,
        POWER_RUSH_A_P,
        POWER_RUSH_C_P,
        A_BUTTON,
        A_BUTTON_PRESSED,
        B_BUTTON,
        B_BUTTON_PRESSED,
        X_BUTTON,
        X_BUTTON_PRESSED,
        Y_BUTTON,
        Y_BUTTON_PRESSED,
        C_STICK_SW,
        C_STICK_S,
        C_STICK_SE,
        C_STICK_W,
        C_STICK_CENTER,
        C_STICK_E,
        C_STICK_NW,
        C_STICK_N,
        C_STICK_NE,
        CONTROL_STICK_SW,
        CONTROL_STICK_S,
        CONTROL_STICK_SE,
        CONTROL_STICK_W,
        CONTROL_STICK_CENTER,
        CONTROL_STICK_E,
        CONTROL_STICK_NW,
        CONTROL_STICK_N,
        CONTROL_STICK_NE,
        L_BUTTON,
        L_BUTTON_PRESSED,
        R_BUTTON,
        R_BUTTON_PRESSED,
        Z_BUTTON,
        Z_BUTTON_PRESSED,
        START_BUTTON,
        START_BUTTON_PRESSED,
        D_PAD_UP,
        D_PAD_UP_PRESSED,
        D_PAD_DOWN,
        D_PAD_DOWN_PRESSED,
        D_PAD_VERTICAL,
        D_PAD_VERTICAL_PRESSED,
        AC_BAR_EMPTY,
        AC_BAR_GREEN,
        AC_BUTTON_BOARD_CENTER,
        AC_BUTTON_BOARD_LEFT,
        AC_BUTTON_BOARD_RIGHT,
        AC_LIGHT_OFF,
        AC_LIGHT_GREEN,
        AC_LIGHT_YELLOW,
        AC_LIGHT_ORANGE,
        AC_LIGHT_STAR,
        AC_OKAY,
        STATUS_HP_UP,
        STATUS_FP_UP,
        STATUS_MARIO_INDICATOR,
        STATUS_EXCLAMATION_BUTTON,
        STRANGE_SACK,
        HAMMER,
        SUPER_HAMMER,
        ULTRA_HAMMER,
        BOOTS,
        SUPER_BOOTS,
        ULTRA_BOOTS,
        GREEN_BOOTS,
        PLANE_CURSE,
        BOAT_CURSE,
        PAPER_CURSE,
        TUBE_CURSE,
        COCONUT_BOMB,
        DRIED_BOUQUET,
        ZESS_FRAPPE,
        ICICLE_POP,
        SHROOM_FRY,
        SHROOM_ROAST,
        SHROOM_STEAK,
        ZESS_DELUXE,
        COCONUT_MEAL_UNUSED,
        MISTAKE,
        SPICY_SOUP,
        SPACE_FOOD,
        SNOW_BUNNY,
        SNOW_BUNNY_2,
        HONEY_SHROOM,
        MAPLE_SHROOM,
        JELLY_SHROOM,
        HONEY_SUPER,
        MAPLE_SUPER,
        JELLY_SUPER,
        HONEY_ULTRA,
        MAPLE_ULTRA,
        JELLY_ULTRA,
        ZESS_TEA,
        ZESS_DINNER,
        ZESS_SPECIAL,
        ZESS_DYNAMITE,
        COURAGE_MEAL,
        SPITE_POUCH,
        MYSTIC_EGG,
        TURTLEY_LEAF,
        KEEL_MANGO,
        FRESH_PASTA,
        GOLDEN_LEAF,
        COCO_CANDY,
        HONEY_CANDY,
        JELLY_CANDY,
        HEALTHY_SALAD,
        METEOR_MEAL,
        INKY_PASTA,
        KOOPASTA,
        SHROOM_BROTH,
        HEARTFUL_CAKE,
        OMELETTE_MEAL,
        PEACH_TART,
        TRIAL_STEW,
        INKY_SAUCE,
        SHROOM_CREPE,
        POISON_SHROOM,
        FRESH_JUICE,
        KOOPA_TEA,
        HOT_SAUCE,
        SHROOM_CAKE,
        MANGO_DELIGHT,
        FRIED_EGG,
        CAKE_MIX,
        MOUSSE_CAKE,
        PEACHY_PEACH,
        FRUIT_PARFAIT,
        SPAGHETTI,
        CHOCO_CAKE,
        EGG_BOMB,
        HORSETAIL,
        KOOPA_BUN_UNUSED,
        ZESS_COOKIE,
        ELECTRO_POP,
        FIRE_POP,
        SPICY_PASTA,
        LOVE_PUDDING,
        COUPLES_CAKE,
        BLUE_KEY,
        RED_KEY,
        GROTTO_KEY,
        STATION_KEY,
        STATION_KEY_2,
        CASTLE_KEY,
        HOUSE_KEY,
        STORAGE_KEY,
        ELEVATOR_KEY_RIVERSIDE,
        STEEPLE_KEY_PURPLE,
        STEEPLE_KEY_WHITE,
        ELEVATOR_KEY_BLUE,
        ELEVATOR_KEY_RED,
        CARD_KEY_PINK,
        CARD_KEY_YELLOW,
        CARD_KEY_GREEN,
        CARD_KEY_BLUE,
        PALACE_KEY,
        PALACE_KEY_RIDDLE_TOWER,
        CAKE,
        POISON_CAKE,
        SHINE_SPRITE,
        CONTACT_LENS,
        SKULL_GEM,
        OLD_LETTER,
        RAGGED_DIARY,
        COG,
        HOT_DOG,
        SUPERBOMBOMB,
        BRIEFCASE,
        MAILBOX_SP,
        BLANKET,
        GALLEY_POT,
        NECKLACE,
        VITAL_PAPER,
        SHELL_EARRINGS,
        PUNI_ORB,
        GOLD_RING,
        WEDDING_RING,
        WEDDING_RING_SMALL,
        SUPER_LUIGI_GREEN,
        SUPER_LUIGI_BLUE,
        SUPER_LUIGI_PINK,
        SUPER_LUIGI_PURPLE,
        SUPER_LUIGI_ORANGE,
        AUTOGRAPH,
        GATE_HANDLE,
        INN_COUPON,
        BLIMP_TICKET,
        TRAIN_TICKET,
        ULTRA_STONE,
        CHUCKOLA_COLA,
        COCONUT,
        CHAMPS_BELT,
        MOON_STONE,
        SUN_STONE,
        GOLDBOB_GUIDE,
        GOLDBOB_GUIDE_RED,
        GOLDBOB_GUIDE_BLUE,
        HAM,
        COOKBOOK,
        THE_LETTER_P,
        BOX,
        BATTLE_TRUNKS,
        ROUTING_SLIP,
        WRESTLING_MAG,
        PRESENT,
        BLACK_KEY,
        MAGICAL_MAP,
        MAGICAL_MAP_LARGE,
        MARIO_WANTED_POSTER,
        STAPLED_PAPERS_UNUSED,
        DUBIOUS_PAPER,
        BLUE_POTION,
        RED_POTION,
        ORANGE_POTION,
        GREEN_POTION,
        STAR_KEY,
        SPECIAL_CARD,
        PLATINUM_CARD,
        GOLD_CARD,
        SILVER_CARD,
        PIANTA,
        LOTTERY_PICK,
        GOLD_BAR,
        GOLD_BAR_X3,
        POISON_SHROOM_ICON,
        UP_ARROW,
        WHACKA_BUMP,
        BOOT_ICON,
        SUPER_BOOT_ICON,
        ULTRA_BOOT_ICON,
        HAMMER_ICON,
        SUPER_HAMMER_ICON,
        ULTRA_HAMMER_ICON,
        ITEM_ICON,
        STAR_ICON,
        TACTICS_ICON,
        SHINE_SPRITE_ICON,
        PARTNER_MOVE_0,
        PARTNER_MOVE_1,
        PARTNER_MOVE_2,
        PARTNER_MOVE_3,
        ORANGE_PLACEHOLDER_F,
        GOOMBELLA,
        KOOPS,
        BOBBERY,
        YOSHI_GREEN,
        YOSHI_RED,
        YOSHI_BLUE,
        YOSHI_ORANGE,
        YOSHI_PINK,
        YOSHI_BLACK,
        YOSHI_WHITE,
        FLURRIE,
        VIVIAN,
        MS_MOWZ,
        DO_NOTHING,
        RUN_AWAY,
        APPEAL,
        DEFEND,
        CHARGE_ACTION,
        SUPER_CHARGE_ACTION,
        AUDIENCE_CAN,
        AUDIENCE_ROCK,
        AUDIENCE_BONE,
        AUDIENCE_HAMMER,
        COURAGE_SHELL,
        TRADE_OFF,
        FIRE_FLOWER_ROD_UNUSED,
        POWER_PUNCH,
        THUNDER_RAGE,
        FIRE_FLOWER,
        THUNDER_BOLT,
        DIZZY_DIAL,
        HONEY_SYRUP,
        POINT_SWAP,
        MUSHROOM,
        SUPER_SHROOM,
        ULTRA_SHROOM,
        LIFE_SHROOM,
        SLOW_SHROOM,
        DRIED_SHROOM,
        VOLT_SHROOM,
        GRADUAL_SYRUP,
        KOOPA_CURSE,
        SHOOTING_STAR,
        ICE_STORM,
        PILLOW_UNUSED,
        MAPLE_SYRUP,
        MINI_MR_MINI,
        MYSTERY,
        SLEEPY_SHEEP,
        STOPWATCH,
        HP_DRAIN_ITEM,
        EARTH_QUAKE,
        HEART,
        FLOWER,
        COIN,
        STAR_POINT,
        STAR_PIECE,
        POW_BLOCK,
        MR_SOFTENER,
        BOOS_SHEET,
        RUIN_POWDER,
        JAMMIN_JELLY,
        TASTY_TONIC,
        REPEL_CAPE,
        DATA_DISK,
        FRIGHT_MASK,
        DIAMOND_STAR,
        EMERALD_STAR,
        GARNET_STAR,
        GOLD_STAR,
        CRYSTAL_STAR,
        RUBY_STAR,
        SAPPHIRE_STAR,
        MARIO_HEAD,
        HP_ICON,
        FP_ICON,
        BP_ICON,
        PLAY_TIME_ICON,
        HUD_MARIO_HP,
        HUD_GOOMBELLA,
        HUD_KOOPS,
        HUD_BOBBERY,
        HUD_YOSHI_GREEN,
        HUD_YOSHI_RED,
        HUD_YOSHI_BLUE,
        HUD_YOSHI_ORANGE,
        HUD_YOSHI_PINK,
        HUD_YOSHI_BLACK,
        HUD_YOSHI_WHITE,
        HUD_FLURRIE,
        HUD_VIVIAN,
        HUD_MS_MOWZ,
        HUD_FP,
        HUD_SP,
        HUD_STAR_POINT,
        HUD_COIN,
        MENU_UP_POINTER,
        MENU_DOWN_POINTER,
        SP_ORB_YELLOW,
        SP_ORB_ORANGE,
        SP_ORB_RED,
        SP_ORB_PINK,
        SP_ORB_PURPLE,
        SP_ORB_BLUE,
        SP_ORB_AZURE,
        SP_ORB_GREEN,
        SP_ORB_EMPTY,
        SP_FILL_0,
        SP_FILL_1,
        SP_FILL_2,
        SP_FILL_3,
        SP_FILL_4,
        SP_FILL_5,
        SP_FILL_6,
        SP_FILL_7,
        SP_FILL_8,
        SP_FILL_9,
        SP_FILL_10,
        SP_FILL_11,
        SP_FILL_12,
        SP_FILL_13,
        SP_FILL_14,
        SP_FILL_15,
        HUD_STRIPE_PINK,
        HUD_STRIPE_ORANGE,
        HUD_STRIPE_BLUE,
        HUD_STRIPE_GREEN,
        WHITE_ROUNDED_SQUARE,
        NUMBER_TIMES,
        NUMBER_TIMES_SMALL,
        NUMBER_SLASH,
        NUMBER_SLASH_SMALL,
        NUMBER_0,
        NUMBER_0_SMALL,
        NUMBER_1,
        NUMBER_1_SMALL,
        NUMBER_2,
        NUMBER_2_SMALL,
        NUMBER_3,
        NUMBER_3_SMALL,
        NUMBER_4,
        NUMBER_4_SMALL,
        NUMBER_5,
        NUMBER_5_SMALL,
        NUMBER_6,
        NUMBER_6_SMALL,
        NUMBER_7,
        NUMBER_7_SMALL,
        NUMBER_8,
        NUMBER_8_SMALL,
        NUMBER_9,
        NUMBER_9_SMALL,
        NUMBER_PLUS,
        NUMBER_MINUS,
        GLOVE_POINTER_V,
        GLOVE_POINTER_H,
        VERTICAL_SPEECH_BUBBLE,
        FLAT_A_BUTTON,
        FLAT_B_BUTTON,
        FLAT_X_BUTTON,
        BLACK_WITH_GREY_AURA,
        LCD_0,
        LCD_1,
        LCD_2,
        LCD_3,
        LCD_4,
        LCD_5,
        LCD_6,
        LCD_7,
        LCD_8,
        LCD_9,
        LCD_COLON,
        LCD_SEPARATOR,
        PUNI_INDICATOR_BACKGROUND,
        PUNI_INDICATOR_WHITE,
        DOWN_ARROW_RED,
        AUDIENCE_INDICATOR,
        AC_A_BUTTON,
        AC_CONTROL_STICK_CENTER,
        AC_START_BUTTON,
        INFINITE_SYMBOL,
        STAR,
        BLACK_WITH_WHITE_CIRCLE,
        HUD_DPAD,
        PARTY_JP,
        GEAR_JP,
        BADGES_JP,
        JOURNAL_JP,
        FLAT_Z_BUTTON,
        BOWSER_NAME_KOOPA,
        BOWSER_NAME,
        BOWSER_TIME,
        BOWSER_TIME_DE,
        BOWSER_TIME_FR,
        BOWSER_TIME_ES,
        BOWSER_TIME_IT,
        BOWSER_WORLD,
        BOWSER_WORLD_DE,
        BOWSER_WORLD_FR,
        BOWSER_WORLD_ES,
        BOWSER_WORLD_IT,
        BOWSER_WORLD_JP,
        POINTS,
        PUNKTE,
        PUNTOS,
        PUNTI,
        PARTY,
        PARTY_DE,
        PARTY_FR,
        PARTY_ES,
        PARTY_IT,
        GEAR,
        GEAR_DE,
        GEAR_FR,
        GEAR_ES,
        GEAR_IT,
        BADGES,
        BADGES_DE,
        BADGES_FR,
        BADGES_ES,
        BADGES_IT,
        JOURNAL,
        JOURNAL_ES,
        
        // Custom icons.
        TOT_COIN_BLUE,
        SUPER_START,
        PERFECT_POWER,
        PERFECT_POWER_P,
        TOT_TIMER_COLON,
        TOT_TIMER_POINT,
        TOT_BLANK,
        PITY_STAR,
        PITY_STAR_P,
        HOTTEST_DOG,
        SUPER_PEEKABOO,
    };
}

extern "C" {

// iconNumberDispGx3D
void iconNumberDispGx(
    gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color);
// iconSetAlpha
// iconSetScale
// iconFlagOff
// iconFlagOn
// iconSetPos
// iconNameToPtr
// iconGX
// iconGetWidthHight
void iconGetTexObj(void* tex_obj, uint16_t icon_id);
void iconDispGxCol(
    gc::mtx34* matrix, uint32_t unk0, uint16_t icon, uint32_t* color);
// iconDispGx2
void iconDispGx(double scale, gc::vec3* pos, int16_t unk2, uint16_t icon);
void iconDispGxAlpha
    (double scale, gc::vec3* pos, int16_t unk2, uint16_t icon, uint8_t alpha);
// iconDisp
// iconChange
// iconDelete
// iconEntry2D
// iconEntry
// iconMain
// iconReInit
// iconTexSetup
// iconInit
// _callback_bin
// _callback_tpl

}

}