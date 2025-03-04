#include <X11/XF86keysym.h>
/* appearance */
static const int sloppyfocus               = 1;  /* focus follows mouse */
static const int mousefollowsfocus         = 1;  /* mouse follows focus */
static const int bypass_surface_visibility = 0;  /* 1 means idle inhibitors will disable idle tracking even if it's surface isn't visible  */
static const int smartgaps                 = 0;  /* 1 means no outer gap when there is only one window */
static const int monoclegaps               = 0;  /* 1 means outer gaps in monocle layout */
static const unsigned int borderpx         = 1;  /* border pixel of windows */
static const unsigned int gappih           = 5; /* horiz inner gap between windows */
static const unsigned int gappiv           = 5; /* vert inner gap between windows */
static const unsigned int gappoh           = 5; /* horiz outer gap between windows and screen edge */
static const unsigned int gappov           = 5; /* vert outer gap between windows and screen edge */
static const float bordercolor[]           = {0.5, 0.5, 0.5, 0};
static const float focuscolor[]            = {1.0, 0.0, 0.0, 0};
/* To conform the xdg-protocol, set the alpha to zero to restore the old behavior */
static const float fullscreen_bg[]         = {0.1, 0.1, 0.1, 1.0};
static const int noidlefullscreen          = 1;  /* disable idle when fullscreen */

/* Autostart */
static const char *const autostart[] = {
	"systemctl", "--user", "import-environment", "WAYLAND_DISPLAY", "XDG_CURRENT_DESKTOP", NULL,
        "dbus-update-activation-environment", "--systemd", "WAYLAND_DISPLAY", "XDG_CURRENT_DESKTOP=dwl", NULL,
        "waybar", NULL,
        "swaybg", "-o", "*", "-i", "/usr/share/backgrounds/f30/extras/i-see-stars.jpg", "-m", "fill", NULL,
        "blueman-applet", NULL,
		"fcitx5", NULL,
        "nm-applet", NULL,
        "dunst", NULL,
        "imwheel", NULL,
		"/usr/libexec/polkit-gnome-authentication-agent-1", NULL,
        "dropbox", "start", "-i", NULL,
        "wljoywake", NULL,
		"sh", "/home/fedora/.local/bin/swayidle-lock", NULL,
        NULL /* terminate */
};

/* tagging - tagcount must be no greater than 31 */
static const int tagcount = 9;

static const Rule rules[] = {
	/* app_id     title       tags mask     isfloating   monitor */
	/* examples:
	{ "Gimp",     NULL,       0,            1,           -1 },
	*/
//	{ "firefox",  NULL,       1 << 8,       0,           -1 },
	{ "Gimp",     NULL,       0,            1,           -1 },
};

/* layout(s) */
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* monitors */
static const MonitorRule monrules[] = {
	/* name       mfact nmaster scale layout       rotate/reflect                x    y */
	/* example of a HiDPI laptop monitor:
	{ "eDP-1",    0.5,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
	*/
	{ "DP-3",    0.5,  1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   0,  0 },
	{ "DP-4",    0.5,  1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   2560,  0 },
	/* defaults */
	{ NULL,       0.55, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.options = NULL,
};

static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;
/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */

// Media commands
static const char *mediaplay[] = { "playerctl", "play-pause", NULL };
static const char *mediastop[] = { "playerctl", "stop", NULL };
static const char *mediaforward[] = { "playerctl", "next", NULL };
static const char *mediaprevious[] = { "playerctl", "previous", NULL };

// for changing the volume via alsa amixer //
// volume-helper is located at ~/.local/bin/volume-helper
static const char *upvol[] = { "/home/fedora/.local/bin/volume-helper", "--increase", "5", NULL };
static const char *downvol[] = { "/home/fedora/.local/bin/volume-helper", "--decrease", "5", NULL };
static const char *mute[] = { "sh", "-c", "pactl set-sink-mute @DEFAULT_SINK@ toggle && volume-helper", NULL };

static const char *termcmd[] = { "alacritty", NULL };
//static const char *menucmd[] = { "rofi -terminal alacritty -modi \"drun,run\" -font \'DejaVu Sans 10\' -show drun -show-icons", NULL };
static const char *menucmd[] = { 
	"rofi",
	"-terminal", "alacritty",
	"-modi", "drun,run",
	"-show", "drun",
	"-font", "DejaVu Sans 10",
	"-show-icons",
	NULL 
};
static const char *lockcmd[] = { "loginctl", "lock-session", NULL };
static const char *powermenucmd[] = { "rofi", "-show", "power-menu", "-modi", "power-menu:/home/fedora/.local/bin/rofi-power-menu", NULL };

static const Key keys[] = {
	/* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
	/* modifier                  key                 function        argument */
	{ MODKEY,                    XKB_KEY_p,          spawn,          {.v = menucmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Return,     spawn,          {.v = termcmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_L,          spawn,       	 {.v = lockcmd} },
	{ MODKEY,                    XKB_KEY_Tab,        focusstack,     {.i = +1} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_ISO_Left_Tab,        focusstack,     {.i = -1} },
	// { MODKEY,                    XKB_KEY_i,          incnmaster,     {.i = +1} },
	// { MODKEY,                    XKB_KEY_d,          incnmaster,     {.i = -1} },
	{ MODKEY,                    XKB_KEY_h,          setmfact,       {.f = -0.05} },
	{ MODKEY,                    XKB_KEY_l,          setmfact,       {.f = +0.05} },
	// { MODKEY|WLR_MODIFIER_LOGO,  XKB_KEY_h,          incgaps,       {.i = +1 } },
	// { MODKEY|WLR_MODIFIER_LOGO,  XKB_KEY_l,          incgaps,       {.i = -1 } },
	// { MODKEY|WLR_MODIFIER_LOGO|WLR_MODIFIER_SHIFT,   XKB_KEY_H,      incogaps,      {.i = +1 } },
	// { MODKEY|WLR_MODIFIER_LOGO|WLR_MODIFIER_SHIFT,   XKB_KEY_L,      incogaps,      {.i = -1 } },
	// { MODKEY|WLR_MODIFIER_LOGO|WLR_MODIFIER_CTRL,    XKB_KEY_h,      incigaps,      {.i = +1 } },
	// { MODKEY|WLR_MODIFIER_LOGO|WLR_MODIFIER_CTRL,    XKB_KEY_l,      incigaps,      {.i = -1 } },
	// { MODKEY|WLR_MODIFIER_LOGO,  XKB_KEY_0,          togglegaps,     {0} },
	// { MODKEY|WLR_MODIFIER_LOGO|WLR_MODIFIER_SHIFT,   XKB_KEY_parenright,defaultgaps,    {0} },
	// { MODKEY,                    XKB_KEY_y,          incihgaps,     {.i = +1 } },
	// { MODKEY,                    XKB_KEY_o,          incihgaps,     {.i = -1 } },
       /*
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_y,          incivgaps,     {.i = +1 } },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_o,          incivgaps,     {.i = -1 } },
	{ MODKEY|WLR_MODIFIER_LOGO,  XKB_KEY_y,          incohgaps,     {.i = +1 } },
	{ MODKEY|WLR_MODIFIER_LOGO,  XKB_KEY_o,          incohgaps,     {.i = -1 } },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Y,          incovgaps,     {.i = +1 } },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_O,          incovgaps,     {.i = -1 } },
	*/
	{ MODKEY,                    XKB_KEY_Return,     zoom,           {0} },
	// { MODKEY,                    XKB_KEY_v,          view,           {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_C,          killclient,     {0} },
	{ MODKEY,                    XKB_KEY_t,          setlayout,      {.v = &layouts[0]} },
	// { MODKEY,                    XKB_KEY_f,          setlayout,      {.v = &layouts[1]} },
	// { MODKEY,                    XKB_KEY_m,          setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                    XKB_KEY_space,      setlayout,      {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,      togglefloating, {0} },
	{ MODKEY,                    XKB_KEY_f,          togglefullscreen, {0} },
	{ MODKEY,                    XKB_KEY_0,          view,           {.ui = ~0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_parenright, tag,            {.ui = ~0} },
	{ MODKEY,                    XKB_KEY_w,          focusmon,       {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY,                    XKB_KEY_e,          focusmon,       {.i = WLR_DIRECTION_RIGHT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_W,       tagmon,         {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_E,    tagmon,         {.i = WLR_DIRECTION_RIGHT} },
	TAGKEYS(          XKB_KEY_1, XKB_KEY_exclam,                     0),
	TAGKEYS(          XKB_KEY_2, XKB_KEY_at,                         1),
	TAGKEYS(          XKB_KEY_3, XKB_KEY_numbersign,                 2),
	TAGKEYS(          XKB_KEY_4, XKB_KEY_dollar,                     3),
	TAGKEYS(          XKB_KEY_5, XKB_KEY_percent,                    4),
	TAGKEYS(          XKB_KEY_6, XKB_KEY_asciicircum,                5),
	TAGKEYS(          XKB_KEY_7, XKB_KEY_ampersand,                  6),
	TAGKEYS(          XKB_KEY_8, XKB_KEY_asterisk,                   7),
	TAGKEYS(          XKB_KEY_9, XKB_KEY_parenleft,                  8),
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Q,          spawn,         {.v = powermenucmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_U,          quit,           {0} },

	/* Ctrl-Alt-Backspace and Ctrl-Alt-Fx used to be handled by X server */
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Terminate_Server, quit, {0} },
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_XF86Switch_VT_##n, chvt, {.ui = (n)} }
	CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
	CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
	{ 0,XF86XK_AudioPlay, spawn,{.v = mediaplay } },
	{ 0,XF86XK_AudioStop, spawn,{.v = mediastop } },
	{ 0,XF86XK_AudioNext, spawn,{.v = mediaforward } },
	{ 0,XF86XK_AudioPrev, spawn,{.v = mediaprevious } },
	{ 0,XF86XK_AudioRaiseVolume, spawn,{.v = upvol } },
	{ 0,XF86XK_AudioLowerVolume, spawn,{.v = downvol } },
	{ 0,XF86XK_AudioMute,spawn,{.v = mute } },
};

static const Button buttons[] = {
	{ MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ MODKEY, BTN_MIDDLE, togglefloating, {0} },
	{ MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
};
