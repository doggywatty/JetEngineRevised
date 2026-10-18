// source/game/challenges/ChallengeMgr.cpp
//
// Minimal, deliberately conservative port: only the two methods Menu_Pause::UpdateExit() needs
// (verified @0x48d094) to pop back to the main menu. Challenge_IsActive() is verified @0x591aa8 to be
// `OnlineUser_IsSet() && m_challenge && m_challengeStarted && IsNetworkReady()` -- none of
// OnlineUser_IsSet/IsNetworkReady/the online challenge system exist in this tree, so this always returns
// false, which is accurate (nothing can ever set up an active challenge yet), not a guess.
// ClearCurrentChallenge() is verified @0x59162c to just set a deferred-delete flag that
// _ClearCurrentChallenge() (692 bytes, not ported) later consumes -- since Challenge_IsActive() always
// reports false, nothing calls it in practice, so the flag alone is a safe, honest no-op for now.
#include "ChallengeMgr.h"

const int ChallengeMgr::k_refreshChallengesTime = 0;

ChallengeMgr::ChallengeMgr() : m_challenge(0), m_deletingChallenge(false), m_refreshChallenge(false), m_challengeStarted(false) {}
ChallengeMgr::~ChallengeMgr() {}

bool ChallengeMgr::Challenge_IsActive() { return false; }
// Challenge_IsActive is never true here (it needs the login), so no challenge of the run exists to be completed
bool ChallengeMgr::Challenge_IsCompleted() { return false; }
void ChallengeMgr::ClearCurrentChallenge() { m_deletingChallenge = true; }
