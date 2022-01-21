;sudo apt install qemu-system-x86
;sudo apt install nasm
bits 16

;ax is the primary accumulator, it is used in input/output and most arithmetic instructions.

start:
        mov ax, 0x07C0  ;  0x07C0 is where we are
        add ax, 0x20    ;  add 0x20 (when shifted 512)
        mov ss, ax      ;  set the stack segment, if we remove this we dont know where the stack is. we could overwrite ourself?, 0x7E0
        mov sp, 0x1000  ;  set the stack pointer, 0x08E0 (relativt stacksegmentet)
        
        mov ax, 0x07C0  ;  set data segment
        mov ds, ax      ;  The starting address of the data segment

        mov si, msg     ;  pointer to the message in SI 
        mov ah, 0x0E    ;  print char BIOS procedure

.next:
        lodsb           ;  load byte at address ds:si(segment:offset) into al. si is then incremented
        cmp al, 0       ;  if the byte is zero
        je .done        ;  jump to done
        int 0x10        ;  invoke the BIOS system call
        jmp .next       ;  loop

.done:
        jmp $           ;  loop forever
    
msg:    db 'Hello', 0   ;  the string we want to print

times 510-($-$$) db 0   ;  fill up to 510 bytes
dw 0xAA55               ; master boot record signature