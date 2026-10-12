#include <kos.h>
#include <dc/sound/sound.h>
#include "sndwav.h"
#include "aica_dsp.h"

/* s_sound.c */
#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "sounds.h"

int activ = 0;
wav_stream_hnd_t cur_hnd = SND_STREAM_INVALID;

/*
 * Reverb. Doom 64 starts WESS with libultra's big room (S_Init in Doom64-RE: reverb_id
 * WESS_REVERB_BIGROOM at a 22050 Hz output rate), and its S_StartSound sends a sound to it at
 * effect mix 16 when the origin is in an MS_REVERB sector, 32 in an MS_REVERBHEAVY one, else 0.
 * The big room runs on the AICA's DSP beside KOS's stock driver (aica_dsp_gen.c, aica_dsp_kos.c,
 * shared with the quake64-decomp port), and each sound's channel is split as libultra splits a
 * voice: the dry share in the channel's volume, the wet share, relative to it, in the channel's
 * DSP send (the send comes after the channel's volume). The music stream stays dry.
 *
 * The send also carries the difference between the two pan laws. libultra pans a voice with
 * equal power (a centred one plays at -3 dB on each side), its effect takes the two wet sides
 * mixed at 0.707 each, and the effect returns to both sides at unity. The AICA plays the near
 * side at full level whatever the pan (a centred channel at 0 dB on both sides) and sends the
 * channel to the DSP unpanned. So on the near side the N64's effect stood
 * 0.707 (eqpower[pan] + eqpower[127 - pan]) / max(eqpower[pan], eqpower[127 - pan]) above
 * libultra's wet share relative to the dry: +3 dB centred, -3 dB panned hard.
 */
#define REVERB_OUTPUT_RATE 22050 /* Doom 64's wess_config.outputsamplerate */
#define REVERB_MS 40 /* libultra's "ms" in its effect tables: 40 samples at any rate */
#define REVERB_RETURN 15 /* the DSP's output level (EFSDL, 3 dB steps, 15 = 0 dB): libultra adds the effect at unity */

#define REVERB_NONE 0
#define REVERB_LIGHT 1 /* MS_REVERB: effect mix 16 */
#define REVERB_HEAVY 2 /* MS_REVERBHEAVY: effect mix 32 */

/* libultra's equal-power table (env.c): the pan law, and the dry and wet shares of an effect
   mix (eqpower[fxMix], eqpower[127 - fxMix]), Q15 */
static const short eqpower[128] = {
	32767, 32764, 32757, 32744, 32727, 32704, 32677, 32644, 32607, 32564, 32517, 32464,
	32407, 32344, 32277, 32205, 32127, 32045, 31958, 31866, 31770, 31668, 31561, 31450,
	31334, 31213, 31087, 30957, 30822, 30682, 30537, 30388, 30234, 30075, 29912, 29744,
	29572, 29395, 29214, 29028, 28838, 28643, 28444, 28241, 28033, 27821, 27605, 27385,
	27160, 26931, 26698, 26461, 26220, 25975, 25726, 25473, 25216, 24956, 24691, 24423,
	24151, 23875, 23596, 23313, 23026, 22736, 22442, 22145, 21845, 21541, 21234, 20924,
	20610, 20294, 19974, 19651, 19325, 18997, 18665, 18331, 17993, 17653, 17310, 16965,
	16617, 16266, 15913, 15558, 15200, 14840, 14477, 14113, 13746, 13377, 13006, 12633,
	12258, 11881, 11503, 11122, 10740, 10357, 9971, 9584, 9196, 8806, 8415, 8023,
	7630, 7235, 6839, 6442, 6044, 5646, 5246, 4845, 4444, 4042, 3640, 3237,
	2833, 2429, 2025, 1620, 1216, 810, 405, 0
};
static const int reverb_mix[3] = { 0, 16, 32 }; /* REVERB_NONE, REVERB_LIGHT, REVERB_HEAVY */
/* the channel's DSP send for each reverb at each pan (KOS's 0-255, the N64's twice over),
   0 (off) to 15 (0 dB); all 0 if the DSP is not up */
static unsigned char reverb_send[3][256];
static int reverb_up = 0;
static int channel_send[64]; /* each channel's DSP send as last written (snd_init zeroes them) */

static void S_InitReverb(void)
{
	/* libultra's big room (drvrnew.c BIGROOM_PARAMS), in samples at the synthesizer's rate */
	static const int bigroom[2 + 4 * 8] = {
		/* sections   length */
		4, 100 * REVERB_MS,
		/* input           output           fbcoef  ffcoef   gain    chorus rate, depth   filter coef */
		0,                 66 * REVERB_MS,  9830,   -9830,   0,      0, 0,                0,
		22 * REVERB_MS,    54 * REVERB_MS,  3276,   -3276,   0x3fff, 0, 0,                0,
		66 * REVERB_MS,    91 * REVERB_MS,  3276,   -3276,   0x3fff, 0, 0,                0,
		0,                 94 * REVERB_MS,  8000,   0,       0,      0, 0,                0x5000
	};
	static aica_dsp_image_t img;
	aica_fx_desc_t fx;
	int i, pan;

	memset(&fx, 0, sizeof(fx));
	fx.rate = REVERB_OUTPUT_RATE;
	fx.sections = bigroom[0];
	fx.length = bigroom[1];
	for (i = 0; i < fx.sections; i++) {
		const int *s = &bigroom[2 + 8 * i];
		fx.section[i].input = s[0];
		fx.section[i].output = s[1];
		fx.section[i].fbcoef = (int16_t)s[2];
		fx.section[i].ffcoef = (int16_t)s[3];
		fx.section[i].gain = (int16_t)s[4];
		fx.section[i].chorus = (uint8_t)(s[5] != 0);
		fx.section[i].lp_fc = (int16_t)s[7];
	}

	if (!aica_dsp_build_libultra_fx(&fx, &img)) {
		dbgio_printf("S_InitReverb: the big room does not fit the DSP\n");
		return;
	}

	if (!aica_dsp_load(&img, REVERB_RETURN))
		return;

	for (i = REVERB_LIGHT; i <= REVERB_HEAVY; i++) {
		int mix = reverb_mix[i];
		double wet = (double)eqpower[127 - mix] / eqpower[mix];

		for (pan = 0; pan < 256; pan++) {
			int left = eqpower[pan >> 1];
			int right = eqpower[127 - (pan >> 1)];
			int near = left > right ? left : right;
			double send = wet * 0.70709 * (left + right) / near;

			reverb_send[i][pan] = aica_dsp_level((int32_t)(send * 32768.0 + 0.5));
		}
	}

	reverb_up = 1;
}

static void S_WriteChannelSend(int chn, int level)
{
	if (chn < 0 || chn >= 64 || channel_send[chn] == level)
		return;

	aica_dsp_send(chn, level);
	channel_send[chn] = level;
}

/* The reverb a sound from origin gets from its sector. Doom 64 read
   origin->subsector->sector->flags for every origin, the sectors' sound origins
   (degenmobj_t, cast to mobj_t *) included: there the two structs shared their first fields.
   Here mobj_t keeps old_x, old_y, old_z before its subsector, so a sector's sound origin is
   told apart by its address and read through its own subsec. */
int S_SoundReverb(mobj_t *origin)
{
	subsector_t *ss;
	int flags;

	if (!origin)
		return REVERB_NONE;

	if ((uintptr_t)origin >= (uintptr_t)sectors &&
		(uintptr_t)origin < (uintptr_t)&sectors[numsectors])
		ss = ((degenmobj_t *)origin)->subsec;
	else
		ss = origin->subsector;

	if (!ss)
		return REVERB_NONE;

	flags = ss->sector->flags;

	if (flags & MS_REVERB)
		return REVERB_LIGHT;
	else if (flags & MS_REVERBHEAVY)
		return REVERB_HEAVY;

	return REVERB_NONE;
}

/* a channel's volume with libultra's dry share for that reverb */
int S_ReverbDryVolume(int vol, int reverb)
{
	if (!reverb_up || reverb == REVERB_NONE)
		return vol;

	return (vol * eqpower[reverb_mix[reverb]]) >> 15;
}

/* a channel's DSP send for that reverb at that pan (0-255; written whatever the reverb, so a
   channel a reverberant sound used last does not keep its send) */
void S_SetChannelReverb(int chn, int reverb, int pan)
{
	if (pan < 0)
		pan = 0;
	else if (pan > 255)
		pan = 255;

	S_WriteChannelSend(chn, reverb_send[reverb][pan]);
}

/* The music is dry. KOS's snd_stream_alloc takes the two lowest free channels
   (snd_sfx_chn_alloc), and a sound may have left its send on either: find them the same way
   and clear their sends before the stream takes them. */
static void S_DryStreamChannels(void)
{
	int chn[2];
	int i;

	for (i = 0; i < 2; i++)
		chn[i] = snd_sfx_chn_alloc();

	for (i = 0; i < 2; i++) {
		if (chn[i] >= 0) {
			snd_sfx_chn_free(chn[i]);
			S_WriteChannelSend(chn[i], 0);
		}
	}
}

void S_RemoveOrigin(mobj_t *origin)
{
	(void)origin;
}

void S_ResetSound(void)
{
}

void S_UpdateSounds(void)
{
}

extern void W_DrawLoadScreen(char *what, int current, int total);

sfxhnd_t sounds[NUMSFX];

#define stringed(sfxname) #sfxname

void *sndptr;

#define setsfx(sn)											\
	sprintf(fnbuf, "%s/sfx/%s.wav", fnpre, stringed(sn));	\
	fs_load(fnbuf, &sndptr);								\
	sounds[sn] = snd_sfx_load_buf((char *)sndptr);			\
	if (sndptr) free(sndptr);								\
	W_DrawLoadScreen("Sounds", sn, NUMSFX - 24)

void init_all_sounds(void)
{
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
}

void S_Init(void)
{
	int wi_rv = wav_init();
	if (!wi_rv)
		dbgio_printf("could not wav_init\n");

	/* after wav_init (snd_stream_init runs snd_init) and before the sounds take sound RAM */
	S_InitReverb();

	init_all_sounds();

	cur_hnd = SND_STREAM_INVALID;
	S_SetSoundVolume(menu_settings.SfxVolume);
}

float soundscale = 1.0f;

void S_SetSoundVolume(int volume)
{
	soundscale = (float)volume / 100.0f;
	if (plasma_loop_channel != -1) {
		P_StopElectricLoop();
		P_StartElectricLoop();
	}
}

void S_SetMusicVolume(int volume)
{
	int sleeps = 0;

	if (cur_hnd == SND_STREAM_INVALID) {
		dbgio_printf("setmusvol invalid handle\n");
		return;
	}

	while (!wav_is_playing() && sleeps < 100) {
		sleeps++;
		thd_sleep(50);
	}

	if (sleeps < 100)
		wav_volume((volume * 255)/100);
	else
		dbgio_printf("timed out on wavisplaying\n");
}

int music_sequence;
char itname[256];

extern int from_menu;

void S_StartMusic(int mus_seq)
{
	if (disabledrawing) {
		music_sequence = 0;
		activ = 0;
		return;
	}

	music_sequence = mus_seq;

	char *name = NULL;
	switch (mus_seq) {
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

	default:
		I_Error("unknown sequence %d\n", mus_seq);
		music_sequence = 0;
		activ = 0;
		return;
	}

	int looping = 1;

	if ((!from_menu) && (gamemap > 40) && !((mus_seq >= 113) && (mus_seq <= 116))) {
		sprintf(itname, "%s/mus/e1m%d.adpcm", fnpre, gamemap-40);
	} else {
		sprintf(itname, "%s/mus/%s.adpcm", fnpre, name);
		if ((mus_seq == 115) || (mus_seq == 114)) {
			looping = 0;
		}
	}

	S_DryStreamChannels();
	cur_hnd = wav_create(itname, looping);

	if (cur_hnd == SND_STREAM_INVALID) {
#if RANGECHECK
		dbgio_printf("Could not create wav %s\n", itname);
#endif
		music_sequence = 0;
		activ = 0;
		return;
	}

	wav_play();

	S_SetMusicVolume(menu_settings.MusVolume);

	activ = 1;
}

void S_StopMusic(void)
{
	music_sequence = 0;
	if (cur_hnd != SND_STREAM_INVALID) {
		wav_destroy();
 		cur_hnd = SND_STREAM_INVALID;
 	}
}

void S_PauseSound(void)
{
}

void S_ResumeSound(void)
{
}

void S_StopSound(mobj_t *origin, int seqnum)
{
(void)origin;
(void)seqnum;
}

void S_StopAll(void)
{
	snd_sfx_stop_all();

	S_StopMusic();
}

#define SND_INACTIVE 0
#define SND_PLAYING 1

int S_SoundStatus(int seqnum)
{
	(void)seqnum;
	return activ;
}

int S_StartSound(mobj_t *origin, int sound_id)
{
	int vol;
	int pan;
	int reverb;
	int chn;

#if RANGECHECK
	if (sound_id < 0 || sound_id > sfx_rectsit)
		I_Error("invalid sound_id %d\n", sound_id);
#endif

	if (disabledrawing == false) {
		if (origin && (origin != cameratarget)) {
			if (!S_AdjustSoundParams(cameratarget, origin, &vol,  &pan))
				return -1;
		} else {
			vol = 124;
			pan = 128;
		}

		reverb = S_SoundReverb(origin);
		vol = S_ReverbDryVolume((int)((float)vol * soundscale), reverb);

		chn = snd_sfx_play(sounds[sound_id], vol, pan);
		S_SetChannelReverb(chn, reverb, pan);
		return chn;
	}
	return -1;
}

#define S_CLIPPING_DIST (1700)
#define S_MAX_DIST (127 * S_CLIPPING_DIST)
#define S_CLOSE_DIST (200)
#define S_ATTENUATOR (S_CLIPPING_DIST - S_CLOSE_DIST)
#define S_STEREO_SWING (96)

int S_AdjustSoundParams(mobj_t *listener, mobj_t *origin, int *vol, int *pan)
{
	fixed_t approx_dist;
	angle_t angle;
	int tmpvol;
	int tmppan;

	approx_dist = P_AproxDistance(listener->x - origin->x, listener->y - origin->y);
	approx_dist >>= FRACBITS;

	if (approx_dist > S_CLIPPING_DIST)
		return 0;

	tmppan = 128;

	if ((listener->x != origin->x) || (listener->y != origin->y)) {
		/* angle of source to listener */
		angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);

		if (angle <= listener->angle)
			angle += 0xffffffff;

		angle -= listener->angle;

		/* stereo separation */
		tmppan -= ((finesine[angle >> ANGLETOFINESHIFT] * S_STEREO_SWING) >> FRACBITS);
	}

	/* volume calculation */
	if (approx_dist < S_CLOSE_DIST) {
		tmpvol = 124; // all 124 used to be 127
	} else {
		/* distance effect */
		approx_dist = -approx_dist; /* set neg */
		tmpvol = (((approx_dist << 7) - approx_dist) + S_MAX_DIST) / S_ATTENUATOR;
	}

	if (tmpvol > 124)
		tmpvol = 124;

	*vol = tmpvol;
	*pan = tmppan;
	return (tmpvol > 0);
}
