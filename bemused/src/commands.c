//	
//	Handles displaying/editing the song data command stream
//	

#include "global.h"

#define MAX_ROWS 34
#define CHANNEL_WIDTH 100

// `name` must be at least 4 Bytes long
char *note_name(int midi_note_num);

void draw_commands() {
	Music_Song song = g_music->songs[g_curr_song];
	char buf[256];

	for (int c=0; c<song.num_channels && c<3; c++) {
		Song_Channel ch = song.channels[c];

		int base_x = 5 + 100 + 5 + c*(CHANNEL_WIDTH+2);
		int base_y = 5 + TEXT_SPRITE_HEIGHT + TEXT_KERN;

		// Draw Channel Heading
		SDL_SetRenderDrawColor(g_renderer, 0x40, 0x40, 0x40, 0xFF);
		SDL_RenderFillRect(g_renderer, &(SDL_Rect){
			base_x, base_y,
			CHANNEL_WIDTH, TEXT_SPRITE_HEIGHT + TEXT_KERN
		});
		SDL_snprintf(buf, 256, "Channel %i", c);
		Text_Draw(buf, 256, base_x+1, base_y+1);

		// Draw Command List
		int row = 0;
		int new_y = base_y;
		for (int i=0; i<ch.data_len && row < MAX_ROWS; i++) {
			new_y += TEXT_SPRITE_HEIGHT + TEXT_KERN;

			// Draw alternating rows in different colours
			if (row & 0b1) {
				SDL_SetRenderDrawColor(g_renderer, 0x20, 0x20, 0x20, 0xFF);
				SDL_RenderFillRect(g_renderer, &(SDL_Rect){
					base_x, new_y,
					CHANNEL_WIDTH, TEXT_SPRITE_HEIGHT + TEXT_KERN
				});
			}

			// Highlight current row if it's currently being played
			if (i == ch.data_offset) {
				SDL_SetRenderDrawColor(g_renderer, 0x40, 0x20, 0x10, 0xFF);
				SDL_RenderFillRect(g_renderer, &(SDL_Rect){
					base_x, new_y,
					CHANNEL_WIDTH, TEXT_SPRITE_HEIGHT + TEXT_KERN
				});
			}

			// Parse command
			char *cmd_str;
			int cmd_args = 0;
			switch (ch.data[i]) {
				case SONG_END:	cmd_str = "END";	cmd_args = 0; break;
				case SONG_WAIT:	cmd_str = "WAIT";	cmd_args = 1; break;
				case SONG_NOTE:	cmd_str = "NOTE";	cmd_args = 1; break;
				case SONG_VOL:	cmd_str = "VOL";	cmd_args = 1; break;
				case SONG_TRIG:	cmd_str = "TRIG";	cmd_args = 0; break;
				case SONG_REL:	cmd_str = "REL";	cmd_args = 0; break;
				case SONG_TICK:	cmd_str = "TICK";	cmd_args = 1; break;
				case SONG_INS:	cmd_str = "INS";	cmd_args = 1; break; // INS actually has a 2-byte arg, but it's handled specially below
				case SONG_MOD:	cmd_str = "MOD";	cmd_args = 1; break;
				default: {
					cmd_str = "UNKN";
					SDL_snprintf(buf, 256, "0x%02X/%2i", ch.data[i], ch.data[i]);
					Text_Draw(buf, 256, base_x+1 + 5*(TEXT_SPRITE_WIDTH+TEXT_KERN), new_y+1);
				} break;
			}
			int x;

			Text_Draw(cmd_str, 4, base_x+1, new_y+1);
			x = base_x+1 + 5*(TEXT_SPRITE_WIDTH+TEXT_KERN);

			for (int j=0; j<cmd_args; j++) {
				switch (ch.data[i]) {
					case SONG_NOTE:	SDL_snprintf(buf, 256, "%s ", note_name(ch.data[++i])); break;
					case SONG_INS:	SDL_snprintf(buf, 256, "0x%02X%02X ", ch.data[i+1], ch.data[i+2]); i+=2; break;
					default:		SDL_snprintf(buf, 256, "%3i ", ch.data[++i]); break;
				}
				Text_Draw(buf, 256, x, new_y+1);
				x += 4*(TEXT_SPRITE_WIDTH+TEXT_KERN);
			}

			row++;
		}

	}
}

char *note_name(int midi_note_num) {
	static char name[4] = "   ";
	int n = (midi_note_num & 0x7F) - 21;
	switch (n % 12) {
		case  0: name[0] = 'A'; name[1] = '-'; break;
		case  1: name[0] = 'B'; name[1] = 'b'; break; // Maybe just keep it A# instead of Bb
		case  2: name[0] = 'B'; name[1] = '-'; break;
		case  3: name[0] = 'C'; name[1] = '-'; break;
		case  4: name[0] = 'C'; name[1] = '#'; break;
		case  5: name[0] = 'D'; name[1] = '-'; break;
		case  6: name[0] = 'D'; name[1] = '#'; break;
		case  7: name[0] = 'E'; name[1] = '-'; break;
		case  8: name[0] = 'F'; name[1] = '-'; break;
		case  9: name[0] = 'F'; name[1] = '#'; break;
		case 10: name[0] = 'G'; name[1] = '-'; break;
		case 11: name[0] = 'G'; name[1] = '#'; break;
	}

	switch (n / 12) {
		case  0: name[2] = '0'; break;
		case  1: name[2] = '1'; break;
		case  2: name[2] = '2'; break;
		case  3: name[2] = '3'; break;
		case  4: name[2] = '4'; break;
		case  5: name[2] = '5'; break;
		case  6: name[2] = '6'; break;
		case  7: name[2] = '7'; break;
		case  8: name[2] = '8'; break;
		case  9: name[2] = '9'; break;
	}

	return name;
}