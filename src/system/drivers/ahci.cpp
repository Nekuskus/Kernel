#include "ahci.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>

static Ahci::HbaConfiguration controller{};
static Ahci::HbaMemory* memory_reg = nullptr;

void Ahci::wait_idle(uint16_t port) {
    PortRegister mem_port = memory_reg->ports[port];

    PortRegister::CommandAndStatus command{};
    command.raw = mem_port.command_and_status.raw;
    command.bits.start = 0;
    mem_port.command_and_status.raw = command.raw;

    while (true) {
        command.raw = mem_port.command_and_status.raw;

        if (!command.bits.command_list_running)
            break;
    }

    command.raw = mem_port.command_and_status.raw;
    command.bits.fis_receive = 0;
    mem_port.command_and_status.raw = command.raw;

    while (true) {
        command.raw = mem_port.command_and_status.raw;

        if (!command.bits.fis_receive_running)
            break;
    }
}

void Ahci::wait_ready(uint16_t port) {
    PortRegister mem_port = memory_reg->ports[port];

    while (true) {
        PortRegister::TaskFileData poll{};
        poll.raw = mem_port.task_file_data.raw;

        if (!poll.bits.status.bits.busy && !poll.bits.status.bits.drq)
            break;

        asm("pause");
    }
}

bool Ahci::gain_ownership() {
    if (memory_reg->host_control.ahci_version.bits.major >= 1 && memory_reg->host_control.ahci_version.bits.minor) {
        GenericHostControl::ExtendedCapabilities extended_caps{};
        extended_caps.raw = memory_reg->host_control.extended_capabilities.raw;

        if (extended_caps.bits.bios_os_handoff) {
            GenericHostControl::BiosOsHandoffControlAndStatus bohc{};
            bohc.raw = memory_reg->host_control.bios_os_handoff_control_and_status.raw;
            bohc.bits.os_owned_semaphore = true;
            memory_reg->host_control.bios_os_handoff_control_and_status.raw = bohc.raw;

            for (size_t i = 0; i < 100000; i++)
                asm("pause");

            bohc.raw = memory_reg->host_control.bios_os_handoff_control_and_status.raw;

            if (bohc.bits.bios_busy)
                for (size_t i = 0; i < 800000; i++)
                    asm("pause");

            bohc.raw = memory_reg->host_control.bios_os_handoff_control_and_status.raw;

            if (!bohc.bits.os_owned_semaphore && (bohc.bits.bios_owned_semaphore || bohc.bits.bios_busy)) {
                Debug::print("[AHCI] Failed to acquire ownership of controller.\n\r");

                return false;
            }

            bohc.bits.os_ownership_change = false;
            memory_reg->host_control.bios_os_handoff_control_and_status.raw = bohc.raw;

            Debug::print("[AHCI] Acquired ownership of controller.\n\r");
        }
    }
    return true;
}

void Ahci::init() {
    auto devs = Pci::get_devices(0x01, 0x06, 0x01);

    if (!devs.length()) {
        Debug::print("[AHCI] No AHCI compatible controller found.\n\r");

        return;
    }

    controller = { .device = devs[0] };

    char text[2048] = "";

    sprintf(text, "[AHCI] Found AHCI controller with Vendor ID %x, Device ID %x, and AHCI base %x.\n\r", controller.device->vendor_id(), controller.device->device_id(), controller.ahci_base());
    Debug::print(text);

    uint64_t ahci_base = (uint64_t)controller.ahci_base();

    memory_reg = (HbaMemory*)(ahci_base + virtual_physical_base);

    Vmm::map_pages(Vmm::get_ctx_kernel(), memory_reg, (void*)ahci_base, (sizeof(HbaMemory) + 0x1000 - 1) / 0x1000, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

    GenericHostControl::HbaControl ghc{};
    ghc.raw = memory_reg->host_control.global_hba_control.raw;
    ghc.bits.enable_ahci = true;
    memory_reg->host_control.global_hba_control.raw = ghc.raw;

    if (!gain_ownership()) {
        Debug::print("[AHCI] Failed to gain controller ownership.\n\r");

        return;
    }

    Debug::print("[AHCI] Gained controller ownership.\n\r");
}