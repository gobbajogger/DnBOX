/* SPDX-License-Identifier: GPL-3.0-only */
/* SuperJ6 / Synth One arp + 16-step ARP SEQ: the UI's view of Synths' hooks
 * (page-compliance/SYNTHS-HOOKS.md, Synths' ARP-HOOKS-API.md H1-H5).
 * X0X_ARP_HOOKS 0 (default, device): the ARPEGGIATOR group and ARP SEQ page
 * are compiled out, nothing half-works. 1: the pages call engine_arp.h, or,
 * on host tests, the fake named by X0X_ARP_FAKE_H (same signatures). */
#pragma once
#ifndef X0X_ARP_HOOKS
#define X0X_ARP_HOOKS 0
#endif
#if X0X_ARP_HOOKS
#ifdef X0X_ARP_FAKE_H
#include X0X_ARP_FAKE_H
#else
#include "engine_arp.h"
#endif
/* Screen domains: one UI domain for both engines; the engine clamps per engine
 * (S1 offset +-12, SJ6 +-24). */
enum { UI_ARP_OFFSET_MIN = -24, UI_ARP_OFFSET_MAX = 24, UI_ARP_ROOT = 48 };
#endif
