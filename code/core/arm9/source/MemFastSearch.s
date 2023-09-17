.text
.arm

// r0: data
// r1: size
// r2: pattern (4 words, each word should be unique)
.global mem_fastSearch16
.type mem_fastSearch16, %function
mem_fastSearch16:
    push {r4-r11,lr}
    add r1, r1, r0
    sub r1, r1, #44 // 32 bytes to load at once + 12 bytes after it
    ldmia r2, {r2-r5}

1:
    ldmia r0!, {r6-r12,lr}
    cmp r6, r2
    cmpne r7, r2
    cmpne r8, r2
    cmpne r9, r2
    cmpne r10, r2
    cmpne r11, r2
    cmpne r12, r2
    cmpne lr, r2
    beq fastSearch16_firstWordMatch
fastSearch16_continueFastSearch:
    cmp r0, r1
    ble 1b

    // only need to handle the last couple of words now
    add r1, #(44 - 16)
2:
    ldr r6, [r0], #4
    cmp r6, r2
    beq fastSearch16_firstWordMatchLast
fastSearch16_continueLastSearch:
    cmp r0, r1
    ble 2b

    mov r0, #0
    pop {r4-r11,pc}

fastSearch16_firstWordMatchLast:
    ldr r6, [r0], #4
    cmp r6, r3
    ldreq r6, [r0], #4
    cmpeq r6, r4
    ldreq r6, [r0], #4
    cmpeq r6, r5
    bne fastSearch16_continueLastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch:
    cmp r6, r2
    beq fastSearch16_firstWordMatch_r6
    cmpne r7, r2
    beq fastSearch16_firstWordMatch_r7
    cmpne r8, r2
    beq fastSearch16_firstWordMatch_r8
    cmpne r9, r2
    beq fastSearch16_firstWordMatch_r9
    cmpne r10, r2
    beq fastSearch16_firstWordMatch_r10
    cmpne r11, r2
    beq fastSearch16_firstWordMatch_r11
    cmpne r12, r2
    beq fastSearch16_firstWordMatch_r12
fastSearch16_firstWordMatch_lr:
    ldr r6, [r0], #4
    cmp r6, r3
    ldreq r6, [r0], #4
    cmpeq r6, r4
    ldreq r6, [r0], #4
    cmpeq r6, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r6:
    sub r0, r0, #16
    cmp r7, r3
    cmpeq r8, r4
    cmpeq r9, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r7:
    sub r0, r0, #12
    cmp r8, r3
    cmpeq r9, r4
    cmpeq r10, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r8:
    sub r0, r0, #8
    cmp r9, r3
    cmpeq r10, r4
    cmpeq r11, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r9:
    sub r0, r0, #4
    cmp r10, r3
    cmpeq r11, r4
    cmpeq r12, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r10:
    cmp r11, r3
    cmpeq r12, r4
    cmpeq lr, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r11:
    cmp r12, r3
    cmpeq lr, r4
    ldreq r6, [r0], #4
    cmpeq r6, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}

fastSearch16_firstWordMatch_r12:
    cmp lr, r3
    ldreq r6, [r0], #4
    cmpeq r6, r4
    ldreq r6, [r0], #4
    cmpeq r6, r5
    bne fastSearch16_continueFastSearch
    sub r0, r0, #16
    pop {r4-r11,pc}
