#ifndef RE4_ENTITY_RANDOM_H
#define RE4_ENTITY_RANDOM_H

// Entity randomizer: deterministic pool + selector (host-testable, no game headers).
//
// The pool was checked against the game's own tables (tools/check_entity_pool.py):
//   - src/game/read.cpp EmFileTbl[id]  -> every pool id has a module of its OWN (em<id>.drs).
//   - config/G4BE08/modules/em<id>     -> that module exists.
//   - ids 0x33 and 0x37 have NO module of their own (EmFileTbl points them at the Ganado
//     placeholder 0x18/0x19; em34 builds them from cModel::type), so EmCreate(0x33/0x37) would
//     load em10 and hand out a Ganado carrying a wrong id. They are never used.
//
// Protected (never replaced, never used as replacement): the merchant (0x18, so the shop is always
// there), Del Lago (0x2F) and Saddler (0x30), whose encounters need their own scripts. Define
// RE4_RANDOM_INCLUDE_PROTECTED to put them in the pool too.
// Everything else that has a module of its own is in the pool: Ganados, animals, traps,
// sub-bosses, bosses (including the giant Salazar 0x31), vehicles and the gallery targets.
// Object classes (0x40+: doors, boxes, barrels ...) are not actors with a module and are left alone.
//
// NOTE: a successful EmCreate() proves only that the module could be loaded; it does not prove
// that the room's script, route, animations or the boss phases suit the new actor.

#include <stdint.h>

static const uint8_t RE4_ACTOR_IDS[] = {
    // Ganado variants (em10..em17, em19..em20); em18 is the merchant (protected, see below)
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
#ifdef RE4_RANDOM_INCLUDE_PROTECTED
    0x18,                   // merchant
#endif
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    // animals / small creatures / traps
    0x21, 0x22,             // village dog, dog
    0x23, 0x24, 0x25,       // crow, snake (box ambusher), parasite
    0x26, 0x27, 0x28, 0x29, // large stationary biter, fish, chicken, bats
    0x2A,                   // traps
    // sub-bosses and bosses on their own module
    0x2B, 0x2C, 0x2D, 0x2E, // El Gigante, insect boss, Novistador, crawler
#ifdef RE4_RANDOM_INCLUDE_PROTECTED
    0x2F, 0x30,             // Del Lago, Saddler
#endif
    0x31, 0x32,             // giant Salazar, U-3
    0x34, 0x35, 0x36,       // Mendez forms, Regenerator
    0x38, 0x39,             // multi-enemy boss, Krauser (knife fight)
    0x3A, 0x3B, 0x3C, 0x3D, // gunship, truck, armoured humanoid, support helicopter
    0x3E                    // gallery targets
};
#define RE4_ACTOR_ID_COUNT ((uint32_t)(sizeof(RE4_ACTOR_IDS) / sizeof(RE4_ACTOR_IDS[0])))

// Ids of the Ganado family: they share one class (em10.cpp), so the list entry's `type`
// (weapon / look) is meaningful between them.
static inline int re4_is_ganado_id(uint8_t id)
{
    return id >= 0x10 && id <= 0x20 && id != 0x18;
}

static inline uint32_t re4_entity_mix(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    return x;
}

static inline int re4_actor_pool_index(uint8_t id)
{
    uint32_t i;
    for (i = 0; i < RE4_ACTOR_ID_COUNT; ++i) {
        if (RE4_ACTOR_IDS[i] == id) return (int)i;
    }
    return -1;
}

// Replacement for list entry (room_key, entry_key). `attempt` 0 is the first choice; higher
// attempts give further, equally reproducible choices (used when the first one cannot be loaded).
// An id outside the pool is returned unchanged.
static inline uint8_t re4_random_entity_id(uint8_t original, uint32_t seed,
                                           uint32_t room_key, uint32_t entry_key,
                                           uint32_t attempt)
{
    uint32_t h;
    if (re4_actor_pool_index(original) < 0) return original;
    h = re4_entity_mix(seed * 0x9E3779B1u ^
                       re4_entity_mix(room_key ^ (entry_key * 0x85EBCA6Bu)));
    h = re4_entity_mix(h + attempt * 0x9E3779B9u);
    return RE4_ACTOR_IDS[h % RE4_ACTOR_ID_COUNT];
}

#endif
