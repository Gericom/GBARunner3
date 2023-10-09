.section ".dtcm", "aw"

.global dtcmStack
dtcmStack:
    .word 0xDEAD57AC // stack protection word
    .space 1024 - 4
.global dtcmStackEnd
dtcmStackEnd:

.global dtcmIrqStack
dtcmIrqStack:
    .space 128
.global dtcmIrqStackEnd
dtcmIrqStackEnd:

.global dtcmSndStack
dtcmSndStack:
    .space 128
.global dtcmSndStackEnd
dtcmSndStackEnd:

.end
