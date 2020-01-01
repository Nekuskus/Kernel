#include <hardware/devices/vbe.hpp>
#include <hardware/idt.hpp>
#include <hardware/port.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>

Firework::FireworkKernel::Idt::Entry idt_entries[256] = {};
Firework::FireworkKernel::Idt::InterruptHandler interrupt_handlers[256] = {};
Firework::FireworkKernel::Idt::Pointer idt_pointer = {};

struct {
    const char* mnemonic;
    const char* message;
} exceptions[] = {
    { .mnemonic = "DE", .message = "Division By Zero" },
    { .mnemonic = "DB", .message = "Debug" },
    { .mnemonic = "NMI", .message = "Non Maskable Interrupt" },
    { .mnemonic = "BP", .message = "Breakpoint" },
    { .mnemonic = "OF", .message = "Overflow" },
    { .mnemonic = "BR", .message = "Out of Bounds" },
    { .mnemonic = "UD", .message = "Invalid Opcode" },
    { .mnemonic = "NM", .message = "No Coprocessor" },
    { .mnemonic = "DF", .message = "Double Fault" },
    { .mnemonic = "-", .message = "Coprocessor Segment Overrun" },
    { .mnemonic = "TS", .message = "Invalid TSS" },
    { .mnemonic = "NP", .message = "Segment Not Present" },
    { .mnemonic = "SS", .message = "Stack Fault" },
    { .mnemonic = "GP", .message = "General Protection Fault" },
    { .mnemonic = "PF", .message = "Page Fault" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "MF", .message = "x87 Floating-Point Exception" },
    { .mnemonic = "AC", .message = "Alignment Check" },
    { .mnemonic = "MC", .message = "Machine Check" },
    { .mnemonic = "XM/XF", .message = "SIMD Floating-Point Exception" },
    { .mnemonic = "VE", .message = "Virtualization Exception" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "SX", .message = "Security Exception" },
    { .mnemonic = "-", .message = "Reserved" },
};

extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void isr32();
extern "C" void isr33();
extern "C" void isr34();
extern "C" void isr35();
extern "C" void isr36();
extern "C" void isr37();
extern "C" void isr38();
extern "C" void isr39();
extern "C" void isr40();
extern "C" void isr41();
extern "C" void isr42();
extern "C" void isr43();
extern "C" void isr44();
extern "C" void isr45();
extern "C" void isr46();
extern "C" void isr47();
extern "C" void isr48();
extern "C" void isr49();
extern "C" void isr50();
extern "C" void isr51();
extern "C" void isr52();
extern "C" void isr53();
extern "C" void isr54();
extern "C" void isr55();
extern "C" void isr56();
extern "C" void isr57();
extern "C" void isr58();
extern "C" void isr59();
extern "C" void isr60();
extern "C" void isr61();
extern "C" void isr62();
extern "C" void isr63();
extern "C" void isr64();
extern "C" void isr65();
extern "C" void isr66();
extern "C" void isr67();
extern "C" void isr68();
extern "C" void isr69();
extern "C" void isr70();
extern "C" void isr71();
extern "C" void isr72();
extern "C" void isr73();
extern "C" void isr74();
extern "C" void isr75();
extern "C" void isr76();
extern "C" void isr76();
extern "C" void isr77();
extern "C" void isr78();
extern "C" void isr79();
extern "C" void isr80();
extern "C" void isr81();
extern "C" void isr82();
extern "C" void isr83();
extern "C" void isr84();
extern "C" void isr85();
extern "C" void isr86();
extern "C" void isr87();
extern "C" void isr88();
extern "C" void isr89();
extern "C" void isr90();
extern "C" void isr91();
extern "C" void isr92();
extern "C" void isr93();
extern "C" void isr94();
extern "C" void isr95();
extern "C" void isr96();
extern "C" void isr97();
extern "C" void isr98();
extern "C" void isr99();
extern "C" void isr100();
extern "C" void isr101();
extern "C" void isr102();
extern "C" void isr103();
extern "C" void isr104();
extern "C" void isr105();
extern "C" void isr106();
extern "C" void isr107();
extern "C" void isr108();
extern "C" void isr109();
extern "C" void isr110();
extern "C" void isr111();
extern "C" void isr112();
extern "C" void isr113();
extern "C" void isr114();
extern "C" void isr115();
extern "C" void isr116();
extern "C" void isr117();
extern "C" void isr118();
extern "C" void isr119();
extern "C" void isr120();
extern "C" void isr121();
extern "C" void isr122();
extern "C" void isr123();
extern "C" void isr124();
extern "C" void isr125();
extern "C" void isr126();
extern "C" void isr127();
extern "C" void isr128();
extern "C" void isr129();
extern "C" void isr130();
extern "C" void isr131();
extern "C" void isr132();
extern "C" void isr133();
extern "C" void isr134();
extern "C" void isr135();
extern "C" void isr136();
extern "C" void isr137();
extern "C" void isr138();
extern "C" void isr139();
extern "C" void isr140();
extern "C" void isr141();
extern "C" void isr142();
extern "C" void isr143();
extern "C" void isr144();
extern "C" void isr145();
extern "C" void isr146();
extern "C" void isr147();
extern "C" void isr148();
extern "C" void isr149();
extern "C" void isr150();
extern "C" void isr151();
extern "C" void isr152();
extern "C" void isr153();
extern "C" void isr154();
extern "C" void isr155();
extern "C" void isr156();
extern "C" void isr157();
extern "C" void isr158();
extern "C" void isr159();
extern "C" void isr160();
extern "C" void isr161();
extern "C" void isr162();
extern "C" void isr163();
extern "C" void isr164();
extern "C" void isr165();
extern "C" void isr166();
extern "C" void isr167();
extern "C" void isr168();
extern "C" void isr169();
extern "C" void isr170();
extern "C" void isr171();
extern "C" void isr172();
extern "C" void isr173();
extern "C" void isr174();
extern "C" void isr175();
extern "C" void isr176();
extern "C" void isr177();
extern "C" void isr178();
extern "C" void isr179();
extern "C" void isr180();
extern "C" void isr181();
extern "C" void isr182();
extern "C" void isr183();
extern "C" void isr184();
extern "C" void isr185();
extern "C" void isr186();
extern "C" void isr187();
extern "C" void isr188();
extern "C" void isr189();
extern "C" void isr190();
extern "C" void isr191();
extern "C" void isr192();
extern "C" void isr193();
extern "C" void isr194();
extern "C" void isr195();
extern "C" void isr196();
extern "C" void isr197();
extern "C" void isr198();
extern "C" void isr199();
extern "C" void isr200();
extern "C" void isr201();
extern "C" void isr202();
extern "C" void isr203();
extern "C" void isr204();
extern "C" void isr205();
extern "C" void isr206();
extern "C" void isr207();
extern "C" void isr208();
extern "C" void isr209();
extern "C" void isr210();
extern "C" void isr211();
extern "C" void isr212();
extern "C" void isr213();
extern "C" void isr214();
extern "C" void isr215();
extern "C" void isr216();
extern "C" void isr217();
extern "C" void isr218();
extern "C" void isr219();
extern "C" void isr220();
extern "C" void isr221();
extern "C" void isr222();
extern "C" void isr223();
extern "C" void isr224();
extern "C" void isr225();
extern "C" void isr226();
extern "C" void isr227();
extern "C" void isr228();
extern "C" void isr229();
extern "C" void isr230();
extern "C" void isr231();
extern "C" void isr232();
extern "C" void isr233();
extern "C" void isr234();
extern "C" void isr235();
extern "C" void isr236();
extern "C" void isr237();
extern "C" void isr238();
extern "C" void isr239();
extern "C" void isr240();
extern "C" void isr241();
extern "C" void isr242();
extern "C" void isr243();
extern "C" void isr244();
extern "C" void isr245();
extern "C" void isr246();
extern "C" void isr247();
extern "C" void isr248();
extern "C" void isr249();
extern "C" void isr250();
extern "C" void isr251();
extern "C" void isr252();
extern "C" void isr253();
extern "C" void isr254();
extern "C" void isr255();

extern "C" void isr_handler(Firework::FireworkKernel::Idt::InterruptRegisters* registers) {
    uint8_t n = registers->int_num & 0xFF;
    Firework::FireworkKernel::Idt::InterruptHandler& handler = interrupt_handlers[n];

    if (n < 32) {
        char text[8192] = "";

        sprintf(text, "Received Exception #%s (%s),\n\rCPU registers: RIP: %x, RSP: %x\n\r    RAX: %x, RBX: %x, RCX: %x, RDX : %x\n\r    RSI: %x, RDI: %x, RSP: %x, RBP: %x\n\r    R8: %x, R9: %x, R10: %x, R11: %x\n\r    R12: %x, R12: %x, R13: %x, R14: %x\n\r    R15: %x", exceptions[n].mnemonic, exceptions[n].message, registers->rip, registers->rsp, registers->rax, registers->rbx, registers->rcx, registers->rbx, registers->rsi, registers->rdi, registers->rsp, registers->rbp, registers->r8, registers->r9, registers->r10, registers->r11, registers->r12, registers->r13, registers->r13, registers->r14, registers->r15);
        Firework::FireworkKernel::Terminal::write_line(text, 0xFFFFFF, 0xe50000);
    }

    if (handler.handler)
        interrupt_handlers[n].handler(registers);

    //if (handler.is_irq)

    if (!handler.should_iret)
        while (1)
            ;
}

void Firework::FireworkKernel::Idt::register_interrupt_handler(uint16_t n, idt_function function, bool is_irq, bool should_iret) {
    interrupt_handlers[n].handler = function;
    interrupt_handlers[n].is_irq = is_irq;
    interrupt_handlers[n].should_iret = should_iret;
}

void set_entry(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t ist) {
    uint16_t flags = 0;
    flags |= ist & 0x7;
    flags |= 1ULL << 9;
    flags |= 1ULL << 10;
    flags |= 1ULL << 11;
    flags |= 1ULL << 15;

    idt_entries[vec].offset_low = function & 0xFFFF;
    idt_entries[vec].selector = selector;
    idt_entries[vec].flags = flags;
    idt_entries[vec].offset_mid = (function >> 16) & 0xFFFF;
    idt_entries[vec].offset_high = (function >> 32) & 0xFFFFFFFF;
    idt_entries[vec].reserved = 0;
}

void set_entry(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t ist, uint8_t dpl) {
    uint16_t flags = 0;
    flags |= ist & 0x7;
    flags |= 1ULL << 9;
    flags |= 1ULL << 10;
    flags |= 1ULL << 11;
    flags |= (dpl & 0x3) << 13;
    flags |= 1ULL << 15;

    idt_entries[vec].offset_low = function & 0xFFFF;
    idt_entries[vec].selector = selector;
    idt_entries[vec].flags = flags;
    idt_entries[vec].offset_mid = (function >> 16) & 0xFFFF;
    idt_entries[vec].offset_high = (function >> 32) & 0xFFFFFFFF;
    idt_entries[vec].reserved = 0;
}

void Firework::FireworkKernel::Idt::init() {
    Port::outb(0x20, 17);
    Port::outb(0xA0, 17);
    Port::outb(0x21, 32);
    Port::outb(0xA1, 40);
    Port::outb(0x21, 1);
    Port::outb(0xA1, 1);
    Port::outb(0x21, 4);
    Port::outb(0xA1, 2);
    Port::outb(0x21, 255);
    Port::outb(0xA1, 255);

    set_entry(0, (uintptr_t)isr0, 0x08, 0);
    set_entry(1, (uintptr_t)isr1, 0x08, 0);
    set_entry(2, (uintptr_t)isr2, 0x08, 0);
    set_entry(3, (uintptr_t)isr3, 0x08, 0);
    set_entry(4, (uintptr_t)isr4, 0x08, 0);
    set_entry(5, (uintptr_t)isr5, 0x08, 0);
    set_entry(6, (uintptr_t)isr6, 0x08, 0);
    set_entry(7, (uintptr_t)isr7, 0x08, 0);
    set_entry(8, (uintptr_t)isr8, 0x08, 0);
    set_entry(9, (uintptr_t)isr9, 0x08, 0);
    set_entry(10, (uintptr_t)isr10, 0x08, 0);
    set_entry(11, (uintptr_t)isr11, 0x08, 0);
    set_entry(12, (uintptr_t)isr12, 0x08, 0);
    set_entry(13, (uintptr_t)isr13, 0x08, 0);
    set_entry(14, (uintptr_t)isr14, 0x08, 0);
    set_entry(15, (uintptr_t)isr15, 0x08, 0);
    set_entry(16, (uintptr_t)isr16, 0x08, 0);
    set_entry(17, (uintptr_t)isr17, 0x08, 0);
    set_entry(18, (uintptr_t)isr18, 0x08, 0);
    set_entry(19, (uintptr_t)isr19, 0x08, 0);
    set_entry(20, (uintptr_t)isr20, 0x08, 0);
    set_entry(21, (uintptr_t)isr21, 0x08, 0);
    set_entry(22, (uintptr_t)isr22, 0x08, 0);
    set_entry(23, (uintptr_t)isr23, 0x08, 0);
    set_entry(24, (uintptr_t)isr24, 0x08, 0);
    set_entry(25, (uintptr_t)isr25, 0x08, 0);
    set_entry(26, (uintptr_t)isr26, 0x08, 0);
    set_entry(27, (uintptr_t)isr27, 0x08, 0);
    set_entry(28, (uintptr_t)isr28, 0x08, 0);
    set_entry(29, (uintptr_t)isr29, 0x08, 0);
    set_entry(30, (uintptr_t)isr30, 0x08, 0);
    set_entry(31, (uintptr_t)isr31, 0x08, 0);
    set_entry(32, (uintptr_t)isr32, 0x08, 0);
    set_entry(33, (uintptr_t)isr33, 0x08, 0);
    set_entry(34, (uintptr_t)isr34, 0x08, 0);
    set_entry(35, (uintptr_t)isr35, 0x08, 0);
    set_entry(36, (uintptr_t)isr36, 0x08, 0);
    set_entry(37, (uintptr_t)isr37, 0x08, 0);
    set_entry(38, (uintptr_t)isr38, 0x08, 0);
    set_entry(39, (uintptr_t)isr39, 0x08, 0);
    set_entry(40, (uintptr_t)isr40, 0x08, 0);
    set_entry(41, (uintptr_t)isr41, 0x08, 0);
    set_entry(42, (uintptr_t)isr42, 0x08, 0);
    set_entry(43, (uintptr_t)isr43, 0x08, 0);
    set_entry(44, (uintptr_t)isr44, 0x08, 0);
    set_entry(45, (uintptr_t)isr45, 0x08, 0);
    set_entry(46, (uintptr_t)isr46, 0x08, 0);
    set_entry(47, (uintptr_t)isr47, 0x08, 0);
    set_entry(48, (uintptr_t)isr48, 0x08, 0);
    set_entry(49, (uintptr_t)isr49, 0x08, 0);
    set_entry(50, (uintptr_t)isr50, 0x08, 0);
    set_entry(51, (uintptr_t)isr51, 0x08, 0);
    set_entry(52, (uintptr_t)isr52, 0x08, 0);
    set_entry(53, (uintptr_t)isr53, 0x08, 0);
    set_entry(54, (uintptr_t)isr54, 0x08, 0);
    set_entry(55, (uintptr_t)isr55, 0x08, 0);
    set_entry(56, (uintptr_t)isr56, 0x08, 0);
    set_entry(57, (uintptr_t)isr57, 0x08, 0);
    set_entry(58, (uintptr_t)isr58, 0x08, 0);
    set_entry(59, (uintptr_t)isr59, 0x08, 0);
    set_entry(60, (uintptr_t)isr60, 0x08, 0);
    set_entry(61, (uintptr_t)isr61, 0x08, 0);
    set_entry(62, (uintptr_t)isr62, 0x08, 0);
    set_entry(63, (uintptr_t)isr63, 0x08, 0);
    set_entry(64, (uintptr_t)isr64, 0x08, 0);
    set_entry(65, (uintptr_t)isr65, 0x08, 0);
    set_entry(66, (uintptr_t)isr66, 0x08, 0);
    set_entry(67, (uintptr_t)isr67, 0x08, 0);
    set_entry(68, (uintptr_t)isr68, 0x08, 0);
    set_entry(69, (uintptr_t)isr69, 0x08, 0);
    set_entry(70, (uintptr_t)isr70, 0x08, 0);
    set_entry(71, (uintptr_t)isr71, 0x08, 0);
    set_entry(72, (uintptr_t)isr72, 0x08, 0);
    set_entry(73, (uintptr_t)isr73, 0x08, 0);
    set_entry(74, (uintptr_t)isr74, 0x08, 0);
    set_entry(75, (uintptr_t)isr75, 0x08, 0);
    set_entry(76, (uintptr_t)isr76, 0x08, 0);
    set_entry(77, (uintptr_t)isr77, 0x08, 0);
    set_entry(78, (uintptr_t)isr78, 0x08, 0);
    set_entry(79, (uintptr_t)isr79, 0x08, 0);
    set_entry(80, (uintptr_t)isr80, 0x08, 0);
    set_entry(81, (uintptr_t)isr81, 0x08, 0);
    set_entry(82, (uintptr_t)isr82, 0x08, 0);
    set_entry(83, (uintptr_t)isr83, 0x08, 0);
    set_entry(84, (uintptr_t)isr84, 0x08, 0);
    set_entry(85, (uintptr_t)isr85, 0x08, 0);
    set_entry(86, (uintptr_t)isr86, 0x08, 0);
    set_entry(87, (uintptr_t)isr87, 0x08, 0);
    set_entry(88, (uintptr_t)isr88, 0x08, 0);
    set_entry(89, (uintptr_t)isr89, 0x08, 0);
    set_entry(90, (uintptr_t)isr90, 0x08, 0);
    set_entry(91, (uintptr_t)isr91, 0x08, 0);
    set_entry(92, (uintptr_t)isr92, 0x08, 0);
    set_entry(93, (uintptr_t)isr93, 0x08, 0);
    set_entry(94, (uintptr_t)isr94, 0x08, 0);
    set_entry(95, (uintptr_t)isr95, 0x08, 0);
    set_entry(96, (uintptr_t)isr96, 0x08, 0);
    set_entry(97, (uintptr_t)isr97, 0x08, 0);
    set_entry(98, (uintptr_t)isr98, 0x08, 0);
    set_entry(99, (uintptr_t)isr99, 0x08, 0);
    set_entry(100, (uintptr_t)isr100, 0x08, 0);
    set_entry(101, (uintptr_t)isr101, 0x08, 0);
    set_entry(102, (uintptr_t)isr102, 0x08, 0);
    set_entry(103, (uintptr_t)isr103, 0x08, 0);
    set_entry(104, (uintptr_t)isr104, 0x08, 0);
    set_entry(105, (uintptr_t)isr105, 0x08, 0);
    set_entry(106, (uintptr_t)isr106, 0x08, 0);
    set_entry(107, (uintptr_t)isr107, 0x08, 0);
    set_entry(108, (uintptr_t)isr108, 0x08, 0);
    set_entry(109, (uintptr_t)isr109, 0x08, 0);
    set_entry(110, (uintptr_t)isr110, 0x08, 0);
    set_entry(111, (uintptr_t)isr111, 0x08, 0);
    set_entry(112, (uintptr_t)isr112, 0x08, 0);
    set_entry(113, (uintptr_t)isr113, 0x08, 0);
    set_entry(114, (uintptr_t)isr114, 0x08, 0);
    set_entry(115, (uintptr_t)isr115, 0x08, 0);
    set_entry(116, (uintptr_t)isr116, 0x08, 0);
    set_entry(117, (uintptr_t)isr117, 0x08, 0);
    set_entry(118, (uintptr_t)isr118, 0x08, 0);
    set_entry(119, (uintptr_t)isr119, 0x08, 0);
    set_entry(120, (uintptr_t)isr120, 0x08, 0);
    set_entry(121, (uintptr_t)isr121, 0x08, 0);
    set_entry(122, (uintptr_t)isr122, 0x08, 0);
    set_entry(123, (uintptr_t)isr123, 0x08, 0);
    set_entry(124, (uintptr_t)isr124, 0x08, 0);
    set_entry(125, (uintptr_t)isr125, 0x08, 0);
    set_entry(126, (uintptr_t)isr126, 0x08, 0);
    set_entry(127, (uintptr_t)isr127, 0x08, 0);
    set_entry(128, (uintptr_t)isr128, 0x08, 0);
    set_entry(129, (uintptr_t)isr129, 0x08, 0);
    set_entry(130, (uintptr_t)isr130, 0x08, 0);
    set_entry(131, (uintptr_t)isr131, 0x08, 0);
    set_entry(132, (uintptr_t)isr132, 0x08, 0);
    set_entry(133, (uintptr_t)isr133, 0x08, 0);
    set_entry(134, (uintptr_t)isr134, 0x08, 0);
    set_entry(135, (uintptr_t)isr135, 0x08, 0);
    set_entry(136, (uintptr_t)isr136, 0x08, 0);
    set_entry(137, (uintptr_t)isr137, 0x08, 0);
    set_entry(138, (uintptr_t)isr138, 0x08, 0);
    set_entry(139, (uintptr_t)isr139, 0x08, 0);
    set_entry(140, (uintptr_t)isr140, 0x08, 0);
    set_entry(141, (uintptr_t)isr141, 0x08, 0);
    set_entry(142, (uintptr_t)isr142, 0x08, 0);
    set_entry(143, (uintptr_t)isr143, 0x08, 0);
    set_entry(144, (uintptr_t)isr144, 0x08, 0);
    set_entry(145, (uintptr_t)isr145, 0x08, 0);
    set_entry(146, (uintptr_t)isr146, 0x08, 0);
    set_entry(147, (uintptr_t)isr147, 0x08, 0);
    set_entry(148, (uintptr_t)isr148, 0x08, 0);
    set_entry(149, (uintptr_t)isr149, 0x08, 0);
    set_entry(150, (uintptr_t)isr150, 0x08, 0);
    set_entry(151, (uintptr_t)isr151, 0x08, 0);
    set_entry(152, (uintptr_t)isr152, 0x08, 0);
    set_entry(153, (uintptr_t)isr153, 0x08, 0);
    set_entry(154, (uintptr_t)isr154, 0x08, 0);
    set_entry(155, (uintptr_t)isr155, 0x08, 0);
    set_entry(156, (uintptr_t)isr156, 0x08, 0);
    set_entry(157, (uintptr_t)isr157, 0x08, 0);
    set_entry(158, (uintptr_t)isr158, 0x08, 0);
    set_entry(159, (uintptr_t)isr159, 0x08, 0);
    set_entry(160, (uintptr_t)isr160, 0x08, 0);
    set_entry(161, (uintptr_t)isr161, 0x08, 0);
    set_entry(162, (uintptr_t)isr162, 0x08, 0);
    set_entry(163, (uintptr_t)isr163, 0x08, 0);
    set_entry(164, (uintptr_t)isr164, 0x08, 0);
    set_entry(165, (uintptr_t)isr165, 0x08, 0);
    set_entry(166, (uintptr_t)isr166, 0x08, 0);
    set_entry(167, (uintptr_t)isr167, 0x08, 0);
    set_entry(168, (uintptr_t)isr168, 0x08, 0);
    set_entry(169, (uintptr_t)isr169, 0x08, 0);
    set_entry(170, (uintptr_t)isr170, 0x08, 0);
    set_entry(171, (uintptr_t)isr171, 0x08, 0);
    set_entry(172, (uintptr_t)isr172, 0x08, 0);
    set_entry(173, (uintptr_t)isr173, 0x08, 0);
    set_entry(174, (uintptr_t)isr174, 0x08, 0);
    set_entry(175, (uintptr_t)isr175, 0x08, 0);
    set_entry(176, (uintptr_t)isr176, 0x08, 0);
    set_entry(177, (uintptr_t)isr177, 0x08, 0);
    set_entry(178, (uintptr_t)isr178, 0x08, 0);
    set_entry(179, (uintptr_t)isr179, 0x08, 0);
    set_entry(180, (uintptr_t)isr180, 0x08, 0);
    set_entry(181, (uintptr_t)isr181, 0x08, 0);
    set_entry(182, (uintptr_t)isr182, 0x08, 0);
    set_entry(183, (uintptr_t)isr183, 0x08, 0);
    set_entry(184, (uintptr_t)isr184, 0x08, 0);
    set_entry(185, (uintptr_t)isr185, 0x08, 0);
    set_entry(186, (uintptr_t)isr186, 0x08, 0);
    set_entry(187, (uintptr_t)isr187, 0x08, 0);
    set_entry(188, (uintptr_t)isr188, 0x08, 0);
    set_entry(189, (uintptr_t)isr189, 0x08, 0);
    set_entry(190, (uintptr_t)isr190, 0x08, 0);
    set_entry(191, (uintptr_t)isr191, 0x08, 0);
    set_entry(192, (uintptr_t)isr192, 0x08, 0);
    set_entry(193, (uintptr_t)isr193, 0x08, 0);
    set_entry(194, (uintptr_t)isr194, 0x08, 0);
    set_entry(195, (uintptr_t)isr195, 0x08, 0);
    set_entry(196, (uintptr_t)isr196, 0x08, 0);
    set_entry(197, (uintptr_t)isr197, 0x08, 0);
    set_entry(198, (uintptr_t)isr198, 0x08, 0);
    set_entry(199, (uintptr_t)isr199, 0x08, 0);
    set_entry(200, (uintptr_t)isr200, 0x08, 0);
    set_entry(201, (uintptr_t)isr201, 0x08, 0);
    set_entry(202, (uintptr_t)isr202, 0x08, 0);
    set_entry(203, (uintptr_t)isr203, 0x08, 0);
    set_entry(204, (uintptr_t)isr204, 0x08, 0);
    set_entry(205, (uintptr_t)isr205, 0x08, 0);
    set_entry(206, (uintptr_t)isr206, 0x08, 0);
    set_entry(207, (uintptr_t)isr207, 0x08, 0);
    set_entry(208, (uintptr_t)isr208, 0x08, 0);
    set_entry(209, (uintptr_t)isr209, 0x08, 0);
    set_entry(210, (uintptr_t)isr210, 0x08, 0);
    set_entry(211, (uintptr_t)isr211, 0x08, 0);
    set_entry(212, (uintptr_t)isr212, 0x08, 0);
    set_entry(213, (uintptr_t)isr213, 0x08, 0);
    set_entry(214, (uintptr_t)isr214, 0x08, 0);
    set_entry(215, (uintptr_t)isr215, 0x08, 0);
    set_entry(216, (uintptr_t)isr216, 0x08, 0);
    set_entry(217, (uintptr_t)isr217, 0x08, 0);
    set_entry(218, (uintptr_t)isr218, 0x08, 0);
    set_entry(219, (uintptr_t)isr219, 0x08, 0);
    set_entry(220, (uintptr_t)isr220, 0x08, 0);
    set_entry(221, (uintptr_t)isr221, 0x08, 0);
    set_entry(222, (uintptr_t)isr222, 0x08, 0);
    set_entry(223, (uintptr_t)isr223, 0x08, 0);
    set_entry(224, (uintptr_t)isr224, 0x08, 0);
    set_entry(225, (uintptr_t)isr225, 0x08, 0);
    set_entry(226, (uintptr_t)isr226, 0x08, 0);
    set_entry(227, (uintptr_t)isr227, 0x08, 0);
    set_entry(228, (uintptr_t)isr228, 0x08, 0);
    set_entry(229, (uintptr_t)isr229, 0x08, 0);
    set_entry(230, (uintptr_t)isr230, 0x08, 0);
    set_entry(231, (uintptr_t)isr231, 0x08, 0);
    set_entry(232, (uintptr_t)isr232, 0x08, 0);
    set_entry(233, (uintptr_t)isr233, 0x08, 0);
    set_entry(234, (uintptr_t)isr234, 0x08, 0);
    set_entry(235, (uintptr_t)isr235, 0x08, 0);
    set_entry(236, (uintptr_t)isr236, 0x08, 0);
    set_entry(237, (uintptr_t)isr237, 0x08, 0);
    set_entry(238, (uintptr_t)isr238, 0x08, 0);
    set_entry(239, (uintptr_t)isr239, 0x08, 0);
    set_entry(240, (uintptr_t)isr240, 0x08, 0);
    set_entry(241, (uintptr_t)isr241, 0x08, 0);
    set_entry(242, (uintptr_t)isr242, 0x08, 0);
    set_entry(243, (uintptr_t)isr243, 0x08, 0);
    set_entry(244, (uintptr_t)isr244, 0x08, 0);
    set_entry(245, (uintptr_t)isr245, 0x08, 0);
    set_entry(246, (uintptr_t)isr246, 0x08, 0);
    set_entry(247, (uintptr_t)isr247, 0x08, 0);
    set_entry(248, (uintptr_t)isr248, 0x08, 0);
    set_entry(249, (uintptr_t)isr249, 0x08, 0, 3);
    set_entry(250, (uintptr_t)isr250, 0x08, 0);
    set_entry(251, (uintptr_t)isr251, 0x08, 0);
    set_entry(252, (uintptr_t)isr252, 0x08, 0);
    set_entry(253, (uintptr_t)isr253, 0x08, 0);
    set_entry(254, (uintptr_t)isr254, 0x08, 0);
    set_entry(255, (uintptr_t)isr255, 0x08, 0);

    idt_pointer = { .limit = 256 * sizeof(Entry) - 1, .base = (uint64_t)&idt_entries };
    asm volatile("lidt %0" ::"m"(idt_pointer));
    asm volatile("sti");
}