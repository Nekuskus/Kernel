global acquire_lock
global release_lock

acquire_lock:
    lock bts dword [rdi], 0
    jc .spin_with_pause
    ret

.spin_with_pause:
    pause
    test dword [rdi], 1
    jnz .spin_with_pause
    jmp acquire_lock

release_lock:
    mov dword [rdi], 0
    ret