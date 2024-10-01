;
;	Comments are marked with semi-colons
;	and last until the end of a line
;

.DTICK	180BPM		; .DTICK sets the ms per tick for the whole song

;	
;	Bass (Lower)
;	

.CH		0x00		; .CH sets which channel the following data should use

.DINS	0x0000

.PLAY	D-2	3q	; Intro
.PLAY	E-2	3q
.PLAY	F-2	3q
.PLAY	E-2	3q

.PLAY	D-2	3q
.PLAY	E-2	3q
.PLAY	F-2	3q
.PLAY	E-2	3q

.PLAY	D-2	3q	; Melody A
.PLAY	E-2	3q
.PLAY	F-2	3q
.PLAY	E-2	3q

.PLAY	Bb2	3q	; Melody B
.PLAY	F-2	3q
.PLAY	Bb2	3q
.PLAY	A-2	3q

.PLAY	D-2	3q	; Melody A
.PLAY	E-2	3q
.PLAY	F-2	3q
.PLAY	E-2	3q

.PLAY	Bb2	3q	; Melody C
.PLAY	A-2	3q

.PLAY	D-2	3q
.PLAY	D-2	3q

REL
END

;	
;	Bass (Upper)
;	

.CH		0x01		; .CH sets which channel the following data should use

.DINS	0x0001
VOL		127

.PLAY	D-3	1q	; Intro
.PLAY	F-3 1q
.PLAY	F-3 1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	F-3	1q
.PLAY	A-3	1q
.PLAY	A-3	1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	D-3	1q
.PLAY	F-3 1q
.PLAY	F-3 1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	F-3	1q
.PLAY	A-3	1q
.PLAY	A-3	1q

.PLAY	E-3	1q
.PLAY	G-3	2q
	
.PLAY	D-3	1q	; Melody A
.PLAY	F-3 1q
.PLAY	F-3 1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	F-3	1q
.PLAY	A-3	1q
.PLAY	A-3	1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	Bb2	1q	; Melody B
.PLAY	D-3 1q
.PLAY	D-3 1q

.PLAY	F-2	1q
.PLAY	C-3	2q

.PLAY	Bb2	1q
.PLAY	D-3 1q
.PLAY	D-3 1q

.PLAY	A-2	1q
.PLAY	C#3	1q
.PLAY	C#3	1q

.PLAY	D-3	1q	; Melody A
.PLAY	F-3 1q
.PLAY	F-3 1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	F-3	1q
.PLAY	A-3	1q
.PLAY	A-3	1q

.PLAY	E-3	1q
.PLAY	G-3	2q

.PLAY	Bb2	1q	; Melody C
.PLAY	D-3 1q
.PLAY	D-3 1q

.PLAY	A-2	1q
.PLAY	C#3	1q
.PLAY	C#3	1q

.PLAY	D-3	3q

REL
END

;	
;	Melody
;	

.CH		0x02
.DINS	0x0002

WAIT	12q		; TODO: Include automatic handling to chain waits if value is too large
WAIT	12q

.PLAY	D-4	1e	; Melody A
.PLAY	F-4	1e
.PLAY	D-5	1h

.PLAY	D-4	1e
.PLAY	F-4	1e
.PLAY	D-5	1h

.PLAY	E-5 1q
WAIT		1e
.PLAY	F-5 1e
.PLAY	E-5 1e
.PLAY	F-5 1e

.PLAY	E-5	1e
.PLAY	C-5	1e
.PLAY	A-4	1h

.PLAY	A-4 1q	; Melody B
.PLAY	D-4 1q
.PLAY	F-4 1e
.PLAY	G-4 1e

.PLAY	A-4 3q

.PLAY	A-4 1q
.PLAY	D-4 1q
.PLAY	F-4 1e
.PLAY	G-4 1e

.PLAY	E-4 3q

.PLAY	D-4	1e	; Melody A
.PLAY	F-4	1e
.PLAY	D-5	1h

.PLAY	D-4	1e
.PLAY	F-4	1e
.PLAY	D-5	1h

.PLAY	E-5 1q
WAIT		1e
.PLAY	F-5 1e
.PLAY	E-5 1e
.PLAY	F-5 1e

.PLAY	E-5	1e
.PLAY	C-5	1e
.PLAY	A-4	1h

.PLAY	A-4	1q	; Melody C
.PLAY	D-4	1q
.PLAY	F-4	1e
.PLAY	G-4	1e

.PLAY	A-4	1e
.PLAY	G-4	1e
.PLAY	F-4	1e
.PLAY	E-4	1e
.PLAY	D-4	1e
.PLAY	C#4	1e

.PLAY	D-4	3q

REL
END