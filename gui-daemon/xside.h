#ifndef QUBES_XSIDE_H
#define QUBES_XSIDE_H QUBES_XSIDE_H
/*
 * The Qubes OS Project, http://www.qubes-os.org
 *
 * Copyright (C) 2010  Rafal Wojtczuk  <rafal@invisiblethingslab.com>
 * Copyright (C) 2010  Joanna Rutkowska <joanna@invisiblethingslab.com>
 * Copyright (C) 2016  Marek Marczykowski-Górecki <marmarek@invisiblethingslab.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/* various file paths */
#define GUID_CONFIG_FILE "/etc/qubes/guid.conf"
#define GUID_CONFIG_DIR "/etc/qubes"
#define QUBES_CLIPBOARD_FILENAME "/var/run/qubes/qubes-clipboard.bin"
#define QREXEC_CLIENT "qrexec-client"
#define QREXEC_CLIENT_VM QREXEC_CLIENT "-vm"
#define QREXEC_CLIENT_PATH "/usr/lib/qubes/" QREXEC_CLIENT
#define QREXEC_CLIENT_VM_PATH "/usr/bin/" QREXEC_CLIENT_VM
#define QVM_KILL_PATH "/usr/bin/qvm-kill"
#define KDIALOG_PATH "/usr/bin/kdialog"
#define ZENITY_PATH "/usr/bin/zenity"
#define QUBES_RELEASE "/etc/qubes-release"

/* dom0 policy evaluation result */
#define QUBES_POLICY_ACCESS_ALLOWED ("result=allow\n")
#define QUBES_POLICY_ACCESS_DENIED  ("result=deny\n")
#define QUBES_POLICY_ACCESS_ALLOWED_LEN (sizeof QUBES_POLICY_ACCESS_ALLOWED - 1)
#define QUBES_POLICY_ACCESS_DENIED_LEN (sizeof QUBES_POLICY_ACCESS_DENIED - 1)

/* qrexec prefix for qrexec-client */
#define QREXEC_COMMAND_PREFIX "DEFAULT:QUBESRPC "

/* qrexec service names */
#define QUBES_SERVICE_CLIPBOARD_COPY "qubes.ClipboardCopy"
#define QUBES_SERVICE_CLIPBOARD_PASTE "qubes.ClipboardPaste"
#define QUBES_SERVICE_EVAL_SIMPLE "policy.EvalSimple"
#define QUBES_SERVICE_EVAL_GUI "policy.EvalGUI"

/* default width of forced colorful border */
#define BORDER_WIDTH 2

/* maximum percentage of the screen that an override_redirect window can cover */
#define MAX_OVERRIDE_REDIRECT_PERCENTAGE 90

/* this makes any X11 error fatal (i.e. cause exit(1)). This behavior was the
 * case for a long time before introducing this option, so nothing really have
 * changed  */
#define MAKE_X11_ERRORS_FATAL

// Mod2 excluded as it is Num_Lock
#define SPECIAL_KEYS_MASK (Mod1Mask | Mod3Mask | Mod4Mask | ShiftMask | ControlMask )

// Special window ID meaning "whole screen"
#define FULLSCREEN_WINDOW_ID 0

#define MAX_EXTRA_PROPS 10

#define MAX_SCREENSAVER_NAMES 10

#define VCHAN_DEFAULT_POLL_DURATION 1000

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/queue.h>
#include <libvchan.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/shm.h>
#include <qubes-gui-protocol.h>
#include "util.h"

#define QUBES_POLICY_EVAL_SIMPLE_SOCKET ("/etc/qubes-rpc/" QUBES_SERVICE_EVAL_SIMPLE)
#define QREXEC_PRELUDE_CLIPBOARD_PASTE (QUBES_SERVICE_EVAL_SIMPLE "+" QUBES_SERVICE_CLIPBOARD_PASTE " dom0 keyword adminvm")

enum clipboard_op {
    CLIPBOARD_COPY,
    CLIPBOARD_PASTE
};

enum trayicon_mode {
    TRAY_BORDER,
    TRAY_BACKGROUND,
    TRAY_TINT,
};

/* per-window data */
struct windowdata {
    unsigned width;
    unsigned height;
    int x;
    int y;
    int is_mapped;
    int is_docked;        /* is it docked tray icon */
    XID remote_winid;    /* window id on VM side */
    Window local_winid;    /* window id on X side */
    Window local_frame_winid; /* window id of frame window created by window manager */
    struct windowdata *transient_for;    /* transient_for hint for WM, see http://tronche.com/gui/x/icccm/sec-4.html#WM_TRANSIENT_FOR */
    int override_redirect;    /* see http://tronche.com/gui/x/xlib/window/attributes/override-redirect.html */
    xcb_shm_seg_t shmseg; /* X Shared Memory segment, or ((xcb_shm_seg_t)-1) if there is none */
    int image_height;    /* size of window content, not always the same as window in dom0! */
    int image_width;
    int have_queued_configure;    /* have configure request been sent to VM - waiting for confirmation */
    int fullscreen_maximize_requested; /* window have requested fullscreen,
                                          which was converted to maximize
                                          request - translate it back when WM
                                          acknowledge maximize */
    uint32_t flags_set;    /* window flags acked to gui-agent */
};

/* extra X11 property to set on every window, prepared parameters for
 * XChangeProperty */
struct extra_prop {
    char *raw_option; /* raw command line option, not parsed yet */
    Atom prop; /* property name */
    Atom type; /* property type */
    int format; /* data format (8, 16, 32) */
    void *data; /* actual data */
    int nelements; /* data size, in "format" units */
};

struct ebuf_entry {
    XEvent xev;
    int64_t time;
    TAILQ_ENTRY(ebuf_entry) entries;
};

union ebuf_rand {
    uint32_t val;
    char raw[sizeof(uint32_t)];
};

/* global variables
 * keep them in this struct for readability
 */
struct _global_handles {
    /* local X server handles and attributes */
    Display *display;
    XTextProperty hostname;   /* dom0 or GUIVM hostname */
    pid_t pid;                /* GUI daemon PID */
    int screen;        /* shortcut to the default screen */
    Window root_win;    /* root attributes */
    int root_width;        /* size of root window */
    int root_height;
    GC context;        /* context for pixmap operations */
    GC frame_gc;        /* graphic context to paint window frame */
    GC tray_gc;        /* graphic context to paint tray background - only in TRAY_BACKGROUND mode */
    double tint_h;  /* precomputed H and S for tray coloring - only in TRAY_TINT mode */
    double tint_s;
    /* atoms for comunitating with xserver */
    Atom wmDeleteMessage;    /* Atom: WM_DELETE_WINDOW */
    Atom tray_selection;    /* Atom: _NET_SYSTEM_TRAY_SELECTION_S<creen number> */
    Atom tray_opcode;    /* Atom: _NET_SYSTEM_TRAY_MESSAGE_OPCODE */
    Atom xembed_message;    /* Atom: _XEMBED */
    Atom xembed_info;    /* Atom: _XEMBED_INFO */
    Atom wm_state;         /* Atom: _NET_WM_STATE */
    Atom wm_state_fullscreen; /* Atom: _NET_WM_STATE_FULLSCREEN */
    Atom wm_state_demands_attention; /* Atom: _NET_WM_STATE_DEMANDS_ATTENTION */
    Atom wm_state_hidden;    /* Atom: _NET_WM_STATE_HIDDEN */
    Atom wm_workarea;      /* Atom: _NET_WORKAREA */
    Atom net_current_desktop;     /* Atom: _NET_CURRENT_DESKTOP */
    Atom frame_extents; /* Atom: _NET_FRAME_EXTENTS */
    Atom wm_state_maximized_vert; /* Atom: _NET_WM_STATE_MAXIMIZED_VERT */
    Atom wm_state_maximized_horz; /* Atom: _NET_WM_STATE_MAXIMIZED_HORZ */
    Atom wm_user_time_window; /* Atom: _NET_WM_USER_TIME_WINDOW */
    Atom wm_user_time; /* Atom: _NET_WM_USER_TIME */
    Atom wm_pid; /* Atom: _NET_WM_PID */
    Atom wm_ping; /* Atom: _NET_WM_PING */
    Atom net_wm_name; /* Atom: _NET_WM_NAME */
    Atom net_wm_icon_name; /* Atom: _NET_WM_ICON_NAME */
    Atom utf8_string; /* Atom: UTF8_STRING */
    int shm_major_opcode;   /* MIT-SHM extension opcode */
    /* shared memory handling */
    struct shm_args_hdr *shm_args;    /* shared memory with Xorg */
    uint32_t cmd_shmid;        /* shared memory id - received from shmoverride.so through shm.id.$DISPLAY file */
    int inter_appviewer_lock_fd; /* FD of lock file used to synchronize shared memory access */
    /* Client VM parameters */
    libvchan_t *vchan;
    char vmname[32];    /* name of VM */
    int domid;        /* Xen domain id (GUI) */
    int target_domid;        /* Xen domain id (VM) - can differ from domid when GUI is stubdom */
    uint32_t protocol_version;  /* Negotiated protocol version.  Must be uint32_t
                                   as it is used as a protocol message. */
    char *cmdline_color;    /* color of frame */
    uint32_t label_color_rgb; /* color of the frame in RGB */
    char *cmdline_icon;    /* icon hint for WM */
    unsigned long *icon_data; /* loaded icon image, ready for _NEW_WM_ICON property */
    int icon_data_len; /* size of icon_data, in sizeof(*icon_data) units */
    int label_index;    /* label (frame color) hint for WM */
    struct windowdata *screen_window; /* window of whole VM screen */
    struct extra_prop extra_props[MAX_EXTRA_PROPS];
    /* lists of windows: */
    /*   indexed by remote window id */
    struct genlist *remote2local;
    /*   indexed by local window id */
    struct genlist *wid2windowdata;
    /* counters and other state */
    int clipboard_requested;    /* if clippoard content was requested by dom0 */
    Time clipboard_xevent_time;  /* timestamp of keypress which triggered last copy/paste */
    Window time_win; /* Window to set _NET_WM_USER_TIME on */
    /* signal was caught */
    int volatile reload_requested;
    pid_t pulseaudio_pid;
    /* configuration */
    char config_path[64]; /* configuration file path (initialized to default) */
    int log_level;        /* log level */
    int startup_timeout;
    int nofork;               /* do not fork into background - used during guid restart */
    int invisible;            /* do not show any VM window */
    pid_t kill_on_connect;  /* pid to kill when connection to gui agent is established */
    int allow_utf8_titles;    /* allow UTF-8 chars in window title */
    int allow_fullscreen;   /* allow fullscreen windows without decoration */
    int override_redirect_protection; /* disallow override_redirect windows to cover more than
					 MAX_OVERRIDE_REDIRECT_PERCENTAGE percent of the screen */
    int copy_seq_mask;    /* modifiers mask for secure-copy key sequence */
    KeySym copy_seq_key;    /* key for secure-copy key sequence */
    int paste_seq_mask;    /* modifiers mask for secure-paste key sequence */
    KeySym paste_seq_key;    /* key for secure-paste key sequence */
    unsigned int clipboard_buffer_size;    /* maximum clipboard size limit */
    int qrexec_clipboard;    /* 0: use GUI protocol to fetch/put clipboard, 1: use qrexec */
    int use_kdialog;    /* use kdialog for prompts (default on KDE) or zenity (default on non-KDE) */
    int prefix_titles;     /* prefix windows titles with VM name (for WM without support for _QUBES_VMNAME property) */
    enum trayicon_mode trayicon_mode; /* trayicon coloring mode */
    int trayicon_border; /* position of trayicon border - 0 - no border, 1 - at the edges, 2 - 1px from the edges */
    bool trayicon_tint_reduce_saturation; /* reduce trayicon saturation by 50% (available only for "tint" mode) */
    bool trayicon_tint_whitehack; /* replace white pixels with almost-white 0xfefefe (available only for "tint" mode) */
    const char *window_background_color_pre_parse; /* user-provided description of window background pixel */
    unsigned long window_background_pixel;         /* parsed version of the above */
    bool disable_override_redirect; /* Disable “override redirect” windows */
    char *screensaver_names[MAX_SCREENSAVER_NAMES]; /* WM_CLASS names for windows detected as screensavers */
    Cursor *cursors;  /* preloaded cursors (using XCreateFontCursor) */
    xcb_connection_t *cb_connection; /**< XCB connection */
    xcb_gcontext_t gc; /**< XCB graphics context */
    int work_x, work_y, work_width, work_height;  /* do not allow a window to go beyond these bounds */
    Atom qubes_label, qubes_label_color, qubes_vmname, qubes_vmwindowid, net_wm_icon;
    bool in_dom0; /* true if we are in dom0, otherwise false */
    Atom net_supported;
    int xen_fd; /* O_PATH file descriptor to /dev/xen/gntdev */
    int xen_dir_fd; /* file descriptor to /dev/xen */
    bool permit_subwindows : 1; /* Permit subwindows */
    uint32_t ebuf_max_delay;
    /* ebuf state */
    TAILQ_HEAD(tailhead, ebuf_entry) ebuf_head;
    int64_t ebuf_prev_release_time;
    int ebuf_next_timeout;
};

typedef struct _global_handles Ghandles;

#define ASSERT_HEIGHT_UNSIGNED(h)                                                      \
    do {                                                                               \
        __typeof__(h) _h = (h);                                                        \
        static_assert((__typeof__(h))(-1) > 1, "must use an unsigned type for " #h);   \
        assert(_h <= MAX_WINDOW_HEIGHT && "excessive height must be rejected earlier");\
    } while (0)

#define ASSERT_WIDTH_UNSIGNED(w)                                                       \
    do {                                                                               \
        __typeof__(w) _w = (w);                                                        \
        static_assert((__typeof__(w))(-1) > 1, "must use an unsigned type for " #w);   \
        assert(_w <= MAX_WINDOW_WIDTH && "excessive width must be rejected earlier");  \
    } while (0)

#define ASSERT_HEIGHT(h)                                                               \
    do {                                                                               \
        __typeof__(h) _h = (h);                                                        \
        assert(_h >= 0 && "negative height must be rejected earlier");                 \
        assert(_h <= MAX_WINDOW_HEIGHT && "excessive height must be rejected earlier");\
    } while (0)

#define ASSERT_WIDTH(w)                                                                \
    do {                                                                               \
        __typeof__(w) _w = (w);                                                        \
        assert(_w >= 0 && "negative width must be rejected earlier");                  \
        assert(_w <= MAX_WINDOW_WIDTH && "excessive width must be rejected earlier");  \
    } while (0)
#pragma GCC poison _w _h

static inline void put_shm_image(
        Ghandles *g,
        xcb_drawable_t drawable,
        struct windowdata *vm_window,
        uint16_t src_x,
        uint16_t src_y,
        uint16_t w,
        uint16_t h,
        int16_t dst_x,
        int16_t dst_y) {
    ASSERT_WIDTH(vm_window->image_width);
    ASSERT_HEIGHT(vm_window->image_height);
    check_xcb_void(
        xcb_shm_put_image(g->cb_connection,
                      drawable,
                      g->gc,
                      (uint16_t)vm_window->image_width,
                      (uint16_t)vm_window->image_height,
                      src_x,
                      src_y,
                      w,
                      h,
                      dst_x,
                      dst_y,
                      24,
                      XCB_IMAGE_FORMAT_Z_PIXMAP,
                      0,
                      vm_window->shmseg,
                      0),
        "xcb_shm_put_image");
}

/* Inter-VM shared clipboard metadata (output is JSON)
 *
 * Refer to https://www.qubes-os.org/doc/gui/ for detailed description. */
struct clipboard_metadata {
    char vmname[32];            /* VM which global clipboard was copied from or pasted to */
    Time xevent_timestamp;      /* timestamp of keypress which triggered copy or paste event */
    bool successful;            /* if operation was successful */
    bool copy_action;           /* if it was a copy action */
    bool paste_action;          /* if it was a paste action */
    bool malformed_request;     /* if VM tried to violate its protocol specs */
    bool oversized_request;     /* if VM tried to send over its set limit */
    bool cleared;               /* if clipboard data is cleared (e.g after paste or empty source clipboard for copy action */
    bool qrexec_clipboard;      /* if clipboard data is from MS-Windows or non-X11 aware OS */
    unsigned int sent_size;     /* number of bytes sent by vm during copy action or pasted to destination */
    unsigned int buffer_size;   /* maximum allowed clipboard size for this vm */
    uint32_t protocol_version_vmside;    /* inter-vm clipboard GUI protocol version - vmside */
    uint32_t protocol_version_xside;     /* inter-vm clipboard GUI protocol version - xside */
};

#define QUBES_NO_SHM_SEGMENT ((xcb_shm_seg_t)-1)

#endif /* QUBES_XSIDE_H */
