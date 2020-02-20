#include "ahci.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/terminal.hpp>

Ahci::HbaConfiguration controller{};
Ahci::HbaMemory* memory_reg = nullptr;

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
                Terminal::write_line("[AHCI] Failed to acquire ownership of controller.", 0xFFFFFF, 0xe50000);
                Debug::print("[AHCI] Failed to acquire ownership of controller.\n");
                return false;
            }

            bohc.bits.os_ownership_change = false;
            memory_reg->host_control.bios_os_handoff_control_and_status.raw = bohc.raw;
            Terminal::write_line("[AHCI] Acquired ownership of controller.", 0xFFFFFF);
            Debug::print("[AHCI] Acquired ownership of controller.\n");
        }
    }
    return true;
}

void Ahci::init() {
    auto devs = Pci::get_devices(0x01, 0x06, 0x01);
    controller = { .device = devs[0] };

    if (!controller.device) {
        Terminal::write_line("[AHCI] No AHCI compatible controller found.\r\n", 0xFFFFFF);

        return;
    }

    char text[2048] = "";

    sprintf(text, "[AHCI] Found AHCI controller with Vendor ID %x, Device ID %x, and AHCI base %x.\r\n", controller.device->vendor_id(), controller.device->device_id(), controller.ahci_base());
    Terminal::write(text, 0xFFFFFF, 0x008000);
    Debug::print(text);

    memory_reg = (HbaMemory*)((uint64_t)controller.ahci_base() + virtual_physical_base);

    Vmm::map_pages(Vmm::get_ctx_kernel(), memory_reg, (void*)(uint64_t)controller.ahci_base(), (sizeof(HbaMemory) + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

    GenericHostControl::HbaControl ghc{};
    ghc.raw = memory_reg->host_control.global_hba_control.raw;
    ghc.bits.enable_ahci = true;
    memory_reg->host_control.global_hba_control.raw = ghc.raw;

    if (!gain_ownership()) {
        Terminal::write_line("[AHCI] Failed to initialize controller.", 0xFFFFFF, 0xe50000);
        Debug::print("[AHCI] Failed to initialize controller.\n");

        return;
    }

    Terminal::write_line("[AHCI] Successfully initialized controller.", 0xFFFFFF, 0x008000);
    Debug::print("[AHCI] Successfully initialized controller.\n");
}