// Copyright 2026 Andreas Eger
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

enum layers {
    _BASE,
    _FN,
};

enum custom_keycodes {
    PASTE = SAFE_RANGE,
};

enum tap_dances {
    TD_ENTER,
    TD_COPY,
};

// Tap: screenshot, hold: _FN. The tap keycode is replaced in process_record_user.
#define SHOT_FN LT(_FN, KC_NO)

static bool is_mac(void) {
    os_variant_t os = detected_host_os();
    return os == OS_MACOS || os == OS_IOS;
}

// Cmd+<key> on macOS, Ctrl+<key> everywhere else.
static void tap_shortcut(uint16_t keycode) {
    tap_code16(is_mac() ? G(keycode) : C(keycode));
}

static void tap_screenshot(void) {
    if (is_mac()) {
        // Area selection, copied to clipboard
        tap_code16(LCTL(LSFT(LGUI(KC_4))));
    } else {
        // Snipping Tool area selection, copied to clipboard
        tap_code16(LSFT(LGUI(KC_S)));
    }
}

// Key 1: tap Enter, double tap Cmd/Ctrl+Enter, hold Esc
static bool enter_held;

static void td_enter_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1 && state->pressed && !state->interrupted) {
        register_code(KC_ESC);
        enter_held = true;
    } else if (state->count == 1) {
        tap_code(KC_ENT);
    } else {
        tap_shortcut(KC_ENT);
    }
}

static void td_enter_reset(tap_dance_state_t *state, void *user_data) {
    if (enter_held) {
        unregister_code(KC_ESC);
        enter_held = false;
    }
}

// Key 3: tap copy, double tap cut
static void td_copy_finished(tap_dance_state_t *state, void *user_data) {
    tap_shortcut(state->count == 1 ? KC_C : KC_X);
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_ENTER] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_enter_finished, td_enter_reset),
    [TD_COPY]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_copy_finished, NULL),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(TD(TD_ENTER), SHOT_FN, TD(TD_COPY)),
    [_FN]   = LAYOUT(_______,      _______, PASTE),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SHOT_FN:
            if (record->tap.count) {
                if (record->event.pressed) {
                    tap_screenshot();
                }
                return false;
            }
            return true;
        case PASTE:
            if (record->event.pressed) {
                tap_shortcut(KC_V);
            }
            return false;
    }
    return true;
}

// Lighting: base colour shows the detected OS, orange while _FN is held.
static void update_lighting(layer_state_t state) {
    if (get_highest_layer(state) == _FN) {
        rgblight_sethsv_noeeprom(HSV_ORANGE);
        return;
    }
    switch (detected_host_os()) {
        case OS_MACOS:
        case OS_IOS:
            rgblight_sethsv_noeeprom(HSV_WHITE);
            break;
        case OS_WINDOWS:
            rgblight_sethsv_noeeprom(HSV_BLUE);
            break;
        default:
            rgblight_sethsv_noeeprom(HSV_PURPLE);
            break;
    }
}

void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    update_lighting(layer_state);
}

bool process_detected_host_os_user(os_variant_t os) {
    update_lighting(layer_state);
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_lighting(state);
    return state;
}
