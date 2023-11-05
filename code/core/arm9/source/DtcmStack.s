.section ".dtcm", "aw"

.global dtcmStack
dtcmStack:
    .word 0xDEAD57AC // stack protection word
    .space 1024 - 4 - 32
.global dtcmStackEnd
dtcmStackEnd:

.global dtcmIrqStack
dtcmIrqStack:
    .word 0xDEAD57AC // stack protection word
    .space 128 - 4 + 32
.global dtcmIrqStackEnd
dtcmIrqStackEnd:

.end
