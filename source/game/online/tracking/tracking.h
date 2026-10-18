// source/game/online/tracking/tracking.h
#ifndef SOURCE_GAME_ONLINE_TRACKING_TRACKING_H
#define SOURCE_GAME_ONLINE_TRACKING_TRACKING_H

// PLACEHOLDER, not verified against the real binary: TRACKING_SCREEN (and the ~20 sibling TRACKING_*
// enums referenced across source/game/online/tracking/GameTrackingMgr.h and elsewhere, e.g.
// TRACKING_PN_TYPE, TRACKING_PURCHASE_TYPE, TRACKING_CURRENCY...) are used in ~35 places but were
// never declared anywhere -- the PDB extraction this project is based on didn't carry their values.
// Unlike control-flow-driving enums (see Menu_Shop::EShopTabs, recovered from ARM disassembly in
// re/NOTES.md), these are opaque, write-only analytics event payload IDs: nothing in the game's own
// logic branches on a TRACKING_SCREEN value, they only ever get sent to GameTrackingMgr to log
// (to a backend this port doesn't have either), so a single placeholder value here cannot corrupt
// gameplay/rendering behavior the way a guessed EShopTabs could. Only declared what's needed to
// compile Menu_Shop so far; add more here (same pattern) as more menus get ported, and replace with
// the real values if they're ever recovered (symbol names in the .so for the tracking event string
// constants, e.g. via `armdis.py <so> -s TRACKING` or `-s SendVisitScreenEvent`, are the place to look).
enum TRACKING_SCREEN { TRACKING_SCREEN_Unknown = 0 };

#endif
