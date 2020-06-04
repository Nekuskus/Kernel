#pragma once

extern "C" void acquire_lock(uint32_t *lock);
extern "C" void release_lock(uint32_t *lock);