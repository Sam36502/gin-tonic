# Bemused - GinTonic Music Editor

## TO-DO:
 - [ ] Fix song queueing/looping (currently not seamless)
 - [ ] Add CLI option to create new file
 - [ ] Decide what to do with .PLAY macro
 - [ ] Implement Running status?

## Song Assembly
I got lazy/fed up with complicated other data formats (like MIDI) and decided
to write my own textual music data format to make converting it to my custom
binary format easier.

### Syntax
Comments are made with semicolons (`;`). They will cause everything following them
to be ignored until the end of the line.

Each line generally contains an instruction or command followed by its arguments
separated by any number of whitespace. E.g.:

	WAIT	1q	; Waits for 1 quarter-note

Arguments can be of the following types:

 - *Pitch:* These arguments should be in the form of a 3-character string which
            contains the Note, whether sharp or flat and the octave number: e.g. `C-4` or `Bb2`

 - *Time:* These arguments should be a number followed by a letter indicating the type which can
           be any of `w`, `h`, `q`, `e`, `s`, standing for Whole, Half, Quarter, Eighth and Sixteenth, respectively.

 - *BPM:* These arguments should be a number followed by 'BPM'. They'll be parsed and converted into ms per 16th note.
          If the parser is expecting a BPM, but no 'BPM' follows the number, the direct number will be used instead for the tick-ms.

 - *Number:* Any other argument may be a simple integer. `strtol` input conventions for other bases is accepted.

### GinTonic Music Commands

| Hex    | Name     | Args | Function                                                                                                    |
|--------|----------|------|-------------------------------------------------------------------------------------------------------------|
| `0x00` | `END`    | 0    | Marks the end of the current song data block. Causes the channel to do nothing until all channels are done  |
| `0x01` | `WAIT`   | 1    | Waits the given number of song ticks before proceeding with the channel                                     |
| `0x02` | `NOTE`   | 1    | Sets the current channel frequency to the provided MIDI note value                                          |
| `0x03` | `VOL`    | 1    | Sets the current channel volume to the provided level                                                       |
| `0x04` | `TRIG`   | 0    | Triggers the current channel envelope                                                                       |
| `0x05` | `REL`    | 0    | Releases the current channel envelope                                                                       |
| `0x06` | `TICK`   | 1    | Changes the current song clocks tick length in ms                                                           |
| `0x07` | `INS`    | 2    | Changes which instrument the current channel uses. The arguments are the instrument ID high & low bytes.    |

### Dot Directives
These are special commands that can be included in the assembly, which won't directly translate to
instructions in the final file, but serve to inform the assembler about something.

| Name     | Args | Function                                                                                                                                              |
|----------|------|-------------------------------------------------------------------------------------------------------------------------------------------------------|
| `.CH`    | 1    | Sets which is the current channel being edited. All instructions following this directive will be saved to the given channel                          |
| `.DTICK` | 1    | Sets the default tick-time in ms                                                                                                                      |
| `.DINS`  | 1    | Sets which instrument is the default for the current channel (This instrument will be automatically set without needing to use an instruction.)       |
| `.PLAY`  | 2    | This is a convenience macro to avoid repeating the same sets of instructions. It takes a pitch and time value and plays that note for the given time. |
| `.PLSTAC`| 2    | Like `.PLAY`, but more staccato (hold for half the time given, then release for half the time)                                                        |
| `.PLLEGA`| 2    | Like `.PLAY`, but more legato (hold for the whole time given)                                                                                         |

// TODO: Split into legato/staccato play macros?
Here's how the `.PLAY` macro works:

	; This:
	.PLAY	C#4	2q	; Plays a C-sharp (Octave 4) for 2 quarter notes

	; ... Is equivalent to this:
	NOTE	C#4
	TRIG
	WAIT	2q
