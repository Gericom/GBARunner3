#pragma once

.macro arm_func name
    .arm
    .global \name
    .type \name, %function
    \name:
.endm

.macro thumb_func name
    .thumb
    .global \name
    .type \name, %function
    \name:
.endm

@ .macro generate macroName, count, arg=0
@     \macroName \arg
@ .if \arg<(\count-1)
@     generate \macroName, \count, %(\arg+1)
@ .endif
@ .endm

.macro generate macroName, count, start=0
    .set value\@, \start
    .rept (\count - \start)
        \macroName %(value\@)
        .set value\@, value\@+1
    .endr
.endm

.macro generateBackwards macroName, count, start=0
    .set value\@, \start+\count-1
    .rept (\count - \start)
        \macroName %(value\@)
        .set value\@, value\@-1
    .endr
.endm
