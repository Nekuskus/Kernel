#pragma once
#include <stdint.h>

#include <system/drivers/pci.hpp>

namespace Ahci {
    struct HbaConfiguration {
        Pci::Device *device;

        uint32_t bar0() {
            return Pci::read(device->bus, device->slot, device->function, 16, 4);
        }

        uint32_t bar1() {
            return Pci::read(device->bus, device->slot, device->function, 20, 4);
        }

        uint32_t bar2() {
            return Pci::read(device->bus, device->slot, device->function, 24, 4);
        }

        uint32_t bar3() {
            return Pci::read(device->bus, device->slot, device->function, 28, 4);
        }

        uint32_t bar4() {
            return Pci::read(device->bus, device->slot, device->function, 32, 4);
        }

        uint32_t ahci_base() {
            return Pci::read(device->bus, device->slot, device->function, 36, 4);
        }

        void ahci_base(uint32_t value) {
            Pci::write(device->bus, device->slot, device->function, 36, value, 4);
        }

        uint16_t sub_system_id() {
            return Pci::read(device->bus, device->slot, device->function, 40, 2);
        }

        uint16_t sub_system_vendor_id() {
            return Pci::read(device->bus, device->slot, device->function, 42, 2);
        }

        uint32_t eprom() {
            return Pci::read(device->bus, device->slot, device->function, 44, 4);
        }

        void eprom(uint32_t value) {
            Pci::write(device->bus, device->slot, device->function, 44, value, 4);
        }

        uint8_t min_grant() {
            return Pci::read(device->bus, device->slot, device->function, 62, 1);
        }

        uint8_t max_latency() {
            return Pci::read(device->bus, device->slot, device->function, 63, 1);
        }
    };

    struct [[gnu::packed]] GenericHostControl {
        union HbaCapabilities {
            struct {
                uint32_t port_count : 4;
                uint32_t enclosure_management : 1;
                uint32_t command_completion_coalescing : 1;
                uint32_t command_slot_count : 4;
                uint32_t partial_state : 1;
                uint32_t slumber_state : 1;
                uint32_t pio_multiple_drq_block : 1;
                uint32_t fis_based_switching : 1;
                uint32_t port_multiplier : 1;
                uint32_t ahci_mode_only : 1;
                uint32_t reserved : 1;
                uint32_t interface_speed_support : 4;
                uint32_t command_list_override : 1;
                uint32_t activity_led : 1;
                uint32_t aggressive_link_power_management : 1;
                uint32_t staggered_spinup : 1;
                uint32_t mechanical_presence_switch : 1;
                uint32_t snotification : 1;
                uint32_t native_command_queuing : 1;
                uint32_t x64bit_addr : 1;
            } bits;
            uint32_t raw;
        } capabilities;
        union HbaControl {
            struct {
                uint32_t hba_reset : 1;
                uint32_t interrupt_enable : 1;
                uint32_t msi_revert_to_single_message : 1;
                uint32_t reserved : 28;
                uint32_t enable_ahci : 1;
            } bits;
            uint32_t raw;
        } global_hba_control;
        uint32_t interrupt_status;
        uint32_t ports_implemented;
        union AhciVersion {
            struct {
                uint32_t minor : 2;
                uint32_t major : 2;
            } bits;
            uint32_t raw;
        } ahci_version;
        union CommandCompletionCoalescingControl {
            struct {
                uint32_t timeout_value : 16;
                uint32_t command_completions : 8;
                uint32_t interrupt : 4;
                uint32_t reserved : 2;
                uint32_t enable : 1;
            } bits;
            uint32_t raw;
        } command_completion_coalescing_control;
        uint32_t command_completion_coalescing_ports;
        union EnclosureManagementLocation {
            struct {
                uint32_t buffer_size : 16;
                uint32_t offset : 16;
            } bits;
            uint32_t raw;
        } enclosure_management_location;
        union EnclosureManagementControl {
            struct {
                uint32_t reserved : 4;
                uint32_t port_multiplier : 1;
                uint32_t activity_led_hardware_driven : 1;
                uint32_t transmit_only : 1;
                uint32_t single_message_buffer : 1;
                uint32_t reserved2 : 4;
                uint32_t sgpio_messages : 1;
                uint32_t ses2_messages : 1;
                uint32_t saf_te_messages : 1;
                uint32_t led_message_type : 1;
                uint32_t reserved3 : 6;
                uint32_t reset : 1;
                uint32_t transmit_message : 1;
                uint32_t reserved4 : 8;
                uint32_t message_received : 1;
            } bits;
            uint32_t raw;
        } enclosure_management_control;
        union ExtendedCapabilities {
            struct {
                uint32_t bios_os_handoff : 1;
                uint32_t nvmhci_present : 1;
                uint32_t auto_partial_to_slumber_transitions : 1;
                uint32_t supports_device_sleep : 1;
                uint32_t supports_aggressive_device_sleep_management : 1;
                uint32_t devsleep_entrance_from_slumber_only : 1;
                uint32_t reserved : 26;
            } bits;
            uint32_t raw;
        } extended_capabilities;
        union BiosOsHandoffControlAndStatus {
            struct {
                uint32_t bios_owned_semaphore : 1;
                uint32_t os_owned_semaphore : 1;
                uint32_t smi_on_os_ownership_change : 1;
                uint32_t os_ownership_change : 1;
                uint32_t bios_busy : 1;
                uint32_t reserved : 26;
            } bits;
            uint32_t raw;
        } bios_os_handoff_control_and_status;
    };

    struct [[gnu::packed]] PortRegister {
        union CommandListBase {
            struct {
                uint32_t reserved : 10;
                uint32_t base : 22;
            } bits;
            uint32_t raw;
        } command_list_base;
        uint32_t command_list_base_upper;
        union FisBase {
            struct {
                uint32_t reserved : 24;
                uint32_t base : 20;
            } bits;
            uint32_t raw;
        } fis_base;
        uint32_t fis_base_upper;
        union InterruptStatus {
            struct {
                uint32_t device_to_host_register_fis : 1;
                uint32_t pio_setup_fis : 1;
                uint32_t dma_setup_fis : 1;
                uint32_t set_device_bits : 1;
                uint32_t unknown_fis : 1;
                uint32_t descriptor_processed : 1;
                uint32_t port_connect_change : 1;
                uint32_t device_mechanical_presence : 1;
                uint32_t reserved : 14;
                uint32_t phyrdy_change : 1;
                uint32_t incorrect_port_multiplier : 1;
                uint32_t overflow : 1;
                uint32_t reserved2 : 1;
                uint32_t interface_non_fatal_error : 1;
                uint32_t interface_fatal_error : 1;
                uint32_t host_bus_data_error : 1;
                uint32_t host_bus_fatal_error : 1;
                uint32_t task_file_error : 1;
                uint32_t cold_port_detect : 1;
            } bits;
            uint32_t raw;
        } interrupt_status;
        union InterruptEnable {
            struct {
                uint32_t device_to_host_register_fis : 1;
                uint32_t pio_setup_fis : 1;
                uint32_t dma_setup_fis : 1;
                uint32_t set_device_bits : 1;
                uint32_t unknown_fis : 1;
                uint32_t descriptor_processed : 1;
                uint32_t port_connect_change : 1;
                uint32_t device_mechanical_presence : 1;
                uint32_t reserved : 14;
                uint32_t phyrdy_change : 1;
                uint32_t incorrect_port_multiplier : 1;
                uint32_t overflow : 1;
                uint32_t reserved2 : 1;
                uint32_t interface_non_fatal_error : 1;
                uint32_t interface_fatal_error : 1;
                uint32_t host_bus_data_error : 1;
                uint32_t host_bus_fatal_error : 1;
                uint32_t task_file_error : 1;
                uint32_t cold_port_detect : 1;
            } bits;
            uint32_t raw;
        } interrupt_enable;
        union CommandAndStatus {
            struct {
                uint32_t start : 1;
                uint32_t spin_up_device : 1;
                uint32_t power_on_device : 1;
                uint32_t command_list_override : 1;
                uint32_t fis_receive : 1;
                uint32_t reserved : 3;
                uint32_t current_command_slot : 5;
                uint32_t mechanical_presence_switch_state : 1;
                uint32_t fis_receive_running : 1;
                uint32_t command_list_running : 1;
                uint32_t cold_presence_state : 1;
                uint32_t port_multiplier_attached : 1;
                uint32_t hot_plug_capable_port : 1;
                uint32_t mechanical_presence_switch_attached_to_port : 1;
                uint32_t cold_presence_detection : 1;
                uint32_t external_sata_port : 1;
                uint32_t fis_based_switching_capable_port : 1;
                uint32_t auto_partial_to_slumber_transitions : 1;
                uint32_t device_is_atapi : 1;
                uint32_t drive_led_on_atapi : 1;
                uint32_t aggressive_link_power_management : 1;
                uint32_t aggressive_slumber_partial : 1;
                uint32_t interface_communication_control : 4;
            } bits;
            uint32_t raw;
        } command_and_status;
        union TaskFileData {
            struct {
                union {
                    struct {
                        uint8_t error : 1;
                        uint8_t command_specific : 2;
                        uint8_t drq : 1;
                        uint8_t command_specific2 : 3;
                        uint8_t busy : 1;
                    } bits;
                    uint8_t raw;
                } status;
                uint32_t error : 8;
                uint32_t reserved : 16;
            } bits;
            uint32_t raw;
        } task_file_data;
        union Signature {
            struct {
                uint32_t lba_high_register : 8;
                uint32_t lba_mid_register : 8;
                uint32_t lba_low_register : 8;
                uint32_t sector_count_register : 8;
            } bits;
            uint32_t raw;
        } signature;
        union SataStatus {
            struct {
                uint32_t device_detection : 4;
                uint32_t current_interface_speed : 4;
                uint32_t interface_power_management : 4;
                uint32_t reserved : 20;
            } bits;
            uint32_t raw;
        } sata_status;
        union SataControl {
            struct {
                uint32_t device_detection_init : 4;
                uint32_t speed_allowed : 4;
                uint32_t interface_power_management_transitions_allowed : 4;
                uint32_t select_power_managment : 4;
                uint32_t port_multiplier_port : 4;
                uint32_t reserved : 12;
            } bits;
            uint32_t raw;
        } sata_control;
        union SataError {
            struct {
                union Error {
                    struct {
                        uint16_t recovered_data_integrity : 1;
                        uint16_t recovered_communications : 1;
                        uint16_t reserved : 6;
                        uint16_t transient_data_integrity : 1;
                        uint16_t persistent_communication_or_data_integrity : 1;
                        uint16_t protocol : 1;
                        uint16_t internal_error : 1;
                        uint16_t reserved2 : 4;
                    } bits;
                    uint16_t raw;
                } error;
            } bits;
            uint32_t raw;
        } sata_error;
        uint32_t sata_active;
        uint32_t command_issue;
        union SNotification {
            struct {
                uint32_t pm_notify : 16;
                uint32_t reserved : 16;
            } bits;
            uint32_t raw;
        } snotification;
        union FisBasedSwitchingControl {
            struct {
                uint32_t enable : 1;
                uint32_t device_error_clear : 1;
                uint32_t single_device_error : 1;
                uint32_t reserved : 4;
                uint32_t device_to_issue : 4;
                uint32_t active_device_optimization : 4;
                uint32_t device_with_error : 4;
                uint32_t reserved2 : 2;
            } bits;
            uint32_t raw;
        } fis_based_switching_control;
        union DeviceSleep {
            struct {
                uint32_t aggressive_device_sleep : 1;
                uint32_t device_sleep_present : 1;
                uint32_t device_sleep_timeout : 8;
                uint32_t min_device_sleep_assertion_time : 4;
                uint32_t device_sleep_idle_timeout : 10;
                uint32_t dito_multiplier : 4;
                uint32_t reserved : 3;
            } bits;
            uint32_t raw;
        } device_sleep;
        uint32_t vendor_specific[4];
    };

    struct [[gnu::packed]] HbaMemory {
        GenericHostControl host_control;
        uint64_t reserved;
        uint32_t vendor_specific[3];
        PortRegister ports[32];
    };

    void init();
    bool gain_ownership();
    void wait_idle(uint16_t port);
    void wait_ready(uint16_t port);
    void *read(uint16_t port, size_t sectors);
    void write(uint16_t port, size_t sectors, void *data);
}  // namespace Ahci