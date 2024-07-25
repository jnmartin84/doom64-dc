#include <kos.h>
#include <dc/sound/sound.h>
#include <oggvorbis/sndoggvorbis.h>

/* s_sound.c */
#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "sounds.h"

#define SYS_FRAMES_PER_SEC 30

int activ = 0;

void S_RemoveOrigin(mobj_t* origin) {}
void S_ResetSound(void) {}
void S_UpdateSounds(void) {}
void W_DrawLoadScreen(char *what, int current, int total);
sfxhnd_t sounds[NUMSFX+1];

extern const char *fnpre;

#define fullsfxname(sn) STORAGE_PREFIX"/sfx/"sn".wav"
#define stringed(sfxname) #sfxname
#define setsfx(sn) sounds[sn] = snd_sfx_load(fullsfxname(stringed(sn))); W_DrawLoadScreen("Sounds", sn, NUMSFX-24)

void init_all_sounds(void) {
	snd_init();
	sounds[0] = 0;
	dbglog_set_level(DBG_INFO);
	setsfx(sfx_punch);
	setsfx(sfx_spawn);
	setsfx(sfx_explode);
	setsfx(sfx_implod);
	setsfx(sfx_pistol);
	setsfx(sfx_shotgun);
	setsfx(sfx_plasma);
	setsfx(sfx_bfg);
	setsfx(sfx_sawup);
	setsfx(sfx_sawidle);
	setsfx(sfx_saw1);
	setsfx(sfx_saw2);
	setsfx(sfx_missile);
	setsfx(sfx_bfgexp);
	setsfx(sfx_pstart);
	setsfx(sfx_pstop);
	setsfx(sfx_doorup);
	setsfx(sfx_doordown);
	setsfx(sfx_secmove);
	setsfx(sfx_switch1);
	setsfx(sfx_switch2);
	setsfx(sfx_itemup);
	setsfx(sfx_sgcock);
	setsfx(sfx_oof);
	setsfx(sfx_telept);
	setsfx(sfx_noway);
	setsfx(sfx_sht2fire);
	setsfx(sfx_sht2load1);
	setsfx(sfx_sht2load2);
	setsfx(sfx_plrpain);
	setsfx(sfx_plrdie);
	setsfx(sfx_slop);
	setsfx(sfx_possit1);
	setsfx(sfx_possit2);
	setsfx(sfx_possit3);
	setsfx(sfx_posdie1);
	setsfx(sfx_posdie2);
	setsfx(sfx_posdie3);
	setsfx(sfx_posact);
	setsfx(sfx_dbpain1);
	setsfx(sfx_dbpain2);
	setsfx(sfx_dbact);
	setsfx(sfx_scratch);
	setsfx(sfx_impsit1);
	setsfx(sfx_impsit2);
	setsfx(sfx_impdth1);
	setsfx(sfx_impdth2);
	setsfx(sfx_impact);
	setsfx(sfx_sargsit);
	setsfx(sfx_sargatk);
	setsfx(sfx_sargdie);
	setsfx(sfx_bos1sit);
	setsfx(sfx_bos1die);
	setsfx(sfx_headsit);
	setsfx(sfx_headdie);
	setsfx(sfx_skullatk);
	setsfx(sfx_bos2sit);
	setsfx(sfx_bos2die);
	setsfx(sfx_pesit);
	setsfx(sfx_pepain);
	setsfx(sfx_pedie);
	setsfx(sfx_bspisit);
	setsfx(sfx_bspidie);
	setsfx(sfx_bspilift);
	setsfx(sfx_bspistomp);
	setsfx(sfx_fattatk);
	setsfx(sfx_fattsit);
	setsfx(sfx_fatthit);
	setsfx(sfx_fattdie);
	setsfx(sfx_bdmissile);
	setsfx(sfx_skelact);
	setsfx(sfx_tracer);
	setsfx(sfx_dart);
	setsfx(sfx_dartshoot);
	setsfx(sfx_cybsit);
	setsfx(sfx_cybdth);
	setsfx(sfx_cybhoof);
	setsfx(sfx_metal);
	setsfx(sfx_door2up);
	setsfx(sfx_door2dwn);
	setsfx(sfx_powerup);
	setsfx(sfx_laser);
	setsfx(sfx_electric);
	setsfx(sfx_thndrlow);
	setsfx(sfx_thndrhigh);
	setsfx(sfx_quake);
	setsfx(sfx_darthit);
	setsfx(sfx_rectact);
	setsfx(sfx_rectatk);
	setsfx(sfx_rectdie);
	setsfx(sfx_rectpain);
	setsfx(sfx_rectsit);
	sounds[NUMSFX] = snd_sfx_load(STORAGE_PREFIX"/sfx/sfx_electric_loop.wav");
}

extern int SfxVolume;
extern int MusVolume;

void S_Init(void) // 80029590
{
	init_all_sounds();

	snd_stream_init();
	sndoggvorbis_init();

	S_SetSoundVolume(SfxVolume);
	S_SetMusicVolume(MusVolume);
}

float soundscale = 1.0f;

void S_SetSoundVolume(int volume)
{
	soundscale = (float)volume / 100.0f;
}

void S_SetMusicVolume(int volume)
{
	sndoggvorbis_volume(volume * 255 / 100);
}

int music_sequence;
char itname[256];

void S_StartMusic(int mus_seq)
{
	if (disabledrawing == false) {
		music_sequence = mus_seq;

		char *name;
		switch(mus_seq) {
			case 96:
			name = "musamb04";
			break;

			case 97:
			name = "musamb05";
			break;

			case 105:
			name = "musamb13";
			break;

			case 104:
			name = "musamb12";
			break;

			case 101:
			name = "musamb09";
			break;

			case 107:
			name = "musamb15";
			break;

			case 108:
			name = "musamb16";
			break;

			case 110:
			name = "musamb18";
			break;

			case 95:
			name = "musamb03";
			break;

			case 98:
			name = "musamb06";
			break;

			case 99:
			name = "musamb07";
			break;

			case 102:
			name = "musamb10";
			break;

			case 93:
			name = "musamb01";
			break;

			case 106:
			name = "musamb14";
			break;

			case 111:
			name = "musamb19";
			break;

			case 103:
			name = "musamb11";
			break;

			case 94:
			name = "musamb02";
			break;

			case 100:
			name = "musamb08";
			break;

			case 112:
			name = "musamb20";
			break;

			case 109:
			name = "musamb17";
			break;

			case 113:
			name = "musfinal";
			break;

			case 114:
			name = "musdone";
			break;

			case 115:
			name = "musintro";
			break;

			case 116:
			name = "mustitle";
			break;
			
			/*
			 * DON'T LET THIS HAPPEN
			 * -Wmaybe-uninitialized
			 */
			default:
			name = "";
			break;
		}

		sprintf(itname, STORAGE_PREFIX"/ogg/%s.ogg", name);

		int looping = 1;
		if(mus_seq == 115 || mus_seq == 114) {
			looping = 0;
		}

		sndoggvorbis_start(itname, looping);
		S_SetMusicVolume(MusVolume);
		activ = 1;
	} else {
		activ = 0;
	}
}

void S_StopMusic(void) // 80029878
{
	music_sequence = 0;
	sndoggvorbis_stop();
}

void S_PauseSound(void) // 800298A4
{
}

void S_ResumeSound(void) // 800298C8
{
}

void S_StopSound(mobj_t *origin,int seqnum) // 800298E8
{
//    if (!origin)
//        wess_seq_stop(seqnum);
//    else
//        wess_seq_stoptype((int)origin);
}

void S_StopAll(void) // 8002991C
{
	snd_sfx_stop_all();
	S_StopMusic();
}

#define SND_INACTIVE 0
#define SND_PLAYING 1

int seqs[256] = {0};

int S_SoundStatus(int seqnum) // 8002993C
{
//    if (wess_seq_status(seqnum) == SEQUENCE_PLAYING)
  //      return SND_PLAYING;
	//else
	//    return SND_INACTIVE;
	return activ;
}

int S_StartSound(mobj_t *origin, int sound_id) // 80029970
{
	int vol;
	int pan;
#if 0
	int flags;
	TriggerPlayAttr attr;
#endif

	if (disabledrawing == false) {
		if (origin && (origin != cameratarget)) {
			if (!S_AdjustSoundParams(cameratarget, origin, &vol, &pan)) {
				return -1;
			}
		} else {
			vol = 127;
			pan = 64;
		}

#if 0
		attr.mask = (TRIGGER_VOLUME | TRIGGER_PAN | TRIGGER_REVERB);
		attr.volume = (char)vol;
		attr.pan = (char)pan;

		attr.reverb = 0;

		if (origin) {
			flags = origin->subsector->sector->flags;

			if (flags & MS_REVERB) {
				attr.reverb = 16;
			} else if (flags & MS_REVERBHEAVY) {
				attr.reverb = 32;
			}
		}

		wess_seq_trigger_type_special(sound_id, (unsigned long)origin, &attr);
#endif

		return snd_sfx_play(sounds[sound_id], (int)((float)(vol * 2.0)*soundscale), pan*2);
	}
	return -1;
}

#define S_CLIPPING_DIST     (1700)
#define S_MAX_DIST          (127 * S_CLIPPING_DIST)
#define S_CLOSE_DIST        (200)
#define S_ATTENUATOR        (S_CLIPPING_DIST - S_CLOSE_DIST)
#define S_STEREO_SWING      (96)
// -1 to 1 * 96 

// (128 +- 96) / 2
// 224 / 2 == 112
// 32 / 2 == 16

// 64 is midpoint

int S_AdjustSoundParams(mobj_t *listener, mobj_t *origin, int* vol, int* pan) // 80029A48
{
	fixed_t approx_dist;
	angle_t angle;
	int tmpvol;

	approx_dist = P_AproxDistance(listener->x - origin->x, listener->y - origin->y);
	approx_dist >>= FRACBITS;

	if (approx_dist > S_CLIPPING_DIST) {
		return 0;
	}

	if (listener->x != origin->x || listener->y != origin->y) {
		/* angle of source to listener */
		angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);

		if (angle <= listener->angle) {
			angle += 0xffffffff;
		}
		angle -= listener->angle;

		/* stereo separation */
		*pan = (128 - ((finesine[angle >> ANGLETOFINESHIFT] * S_STEREO_SWING) >> FRACBITS)) >> 1;
	} else {
		*pan = 64;
	}

	/* volume calculation */
	if (approx_dist < S_CLOSE_DIST) {
		tmpvol = 127;
	} else {
		/* distance effect */
		approx_dist = -approx_dist; /* set neg */
		tmpvol = (((approx_dist << 7) - approx_dist) + S_MAX_DIST) / S_ATTENUATOR;
	}

	tmpvol = (tmpvol * 11) / 10;
	if (tmpvol > 127) {
		tmpvol = 127;
	}
	*vol = tmpvol;
	return (tmpvol > 0);
}
