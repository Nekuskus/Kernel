#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr size_t address_mask = ~(0xFFF | (1ull << 63));

namespace Spark::Vmm {
    /**
     * @brief The virtual memory flags
     * 
     */
    enum VirtualMemoryFlags {
        VMM_PRESENT = 1 << 0,
        VMM_WRITE = 1 << 1,
        VMM_USER = 1 << 2,
        VMM_WT = 1 << 3,
        VMM_NO_CACHE = 1 << 4,
        VMM_DIRTY = 1 << 5,
        VMM_LARGE = 1 << 7,
    };

    /**
     * @brief Page table definition
     * 
     */
    struct PageTable {
        uint64_t ents[512];
    };

    /**
     * @brief Page table entries for 4-level paging definition
     * 
     */
    struct PageTableEntries {
        size_t pml4;
        size_t pdp;
        size_t pd;
        size_t pt;
    };

    /**
     * @brief Converts virtual address to page table entries
     * 
     * @param virt the virtual address
     * @return PageTableEntries 
     */
    PageTableEntries virtual_to_entries(void* virt);

    /**
     * @brief Converts page table entries to a virtual address
     * 
     * @param entries 
     * @return void* 
     */
    void* entries_to_virtual(PageTableEntries entries);

    /**
     * @brief Initializes 4-level paging
     * 
     */
    void init();

    /**
     * @brief Maps a virtual address
     * 
     * @param pml4 
     * @param virt 
     * @param phys 
     * @param count 
     * @param perms 
     * @return true 
     * @return false 
     */
    bool map_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms);

    /**
     * @brief Unmaps a virtual address
     * 
     * @param pml4 
     * @param virt 
     * @param count 
     * @return true 
     * @return false 
     */
    bool unmap_pages(PageTable* pml4, void* virt, size_t count);

    /**
     * @brief Updates a virtual address' permissions
     * 
     * @param pml4 
     * @param virt 
     * @param count 
     * @param perms 
     * @return true 
     * @return false 
     */
    bool update_perms(PageTable* pml4, void* virt, size_t count, int perms);

    /**
     * @brief Maps huge memory pages
     * 
     * @param pml4 
     * @param virt 
     * @param phys 
     * @param count 
     * @param perms 
     * @return true 
     * @return false 
     */
    bool map_huge_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms);

    /**
     * @brief Unmaps huge memory pages
     * 
     * @param pml4 
     * @param virt 
     * @param count 
     * @return true 
     * @return false 
     */
    bool unmap_huge_pages(PageTable* pml4, void* virt, size_t count);

    /**
     * @brief Updates huge memory pages' virtual address' permissions
     * 
     * @param pml4 
     * @param virt 
     * @param count 
     * @param perms 
     * @return true 
     * @return false 
     */
    bool update_huge_perms(PageTable* pml4, void* virt, size_t count, int perms);

    /**
     * @brief Gets physical address from 4-level paging entry
     * 
     * @param pml4 
     * @param virt 
     * @return uintptr_t 
     */
    uintptr_t get_entry(PageTable* pml4, void* virt);

    /**
     * @brief Creates a new address space
     * 
     * @return PageTable* 
     */
    PageTable* new_address_space();

    /**
     * @brief Saves the current context
     * 
     */
    void save_context();

    /**
     * @brief Gives the saved context's pointer
     * 
     * @return PageTable** 
     */
    PageTable** get_ctx_ptr();

    /**
     * @brief Restores the context
     * 
     */
    void restore_context();

    /**
     * @brief Drops the context
     * 
     */
    void drop_context();

    /**
     * @brief Sets the context
     * 
     * @param ctx 
     */
    void set_context(PageTable* ctx);

    /**
     * @brief Get the current context object
     * 
     * @return PageTable* 
     */
    PageTable* get_current_context();
};  // namespace Spark::Vmm