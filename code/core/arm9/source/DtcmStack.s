.section ".dtcm", "aw"

.global dtcmStack
dtcmStack:
    .space 512
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
