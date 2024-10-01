;
;	Simple test song that makes a few clicks
;	to test seamless looping
;

.DTICK 120BPM

.CH		0x00
.DINS	0x0000

NOTE	A-5
TRIG
WAIT	1q

NOTE	A-4
TRIG
WAIT	1q

NOTE	A-4
TRIG
WAIT	1q

NOTE	A-4
TRIG
WAIT	1q

REL
END
