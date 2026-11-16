/* SPDX-License-Identifier: GPL-3.0-only */
/* R22 drum voice / SOUND page. Generated paths from
 * from-dot/R22-gallery/views/909-voice-00..10.svg and 808-voice-00..10.svg.
 * Included from ui.c after drum_strip_glyph. */

static const char *const DV_NAME_909[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "CLOSED HAT", "OPEN HAT", "CRASH", "RIDE"};
static const char *const DV_NAME_808[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "COWBELL", "CYMBAL", "OPEN HAT", "CLOSED HAT"};

static const char *dv_voice_name(int t, int v)
{
    if (v < 0 || v >= NDRUM) return "KIT";
    return t == T_808 ? DV_NAME_808[v] : DV_NAME_909[v];
}
static void dv_full_title(char *t, int part, int v)
{
    put_s(put_s(t, part == PART_808 ? "808 " : "909 "), dv_voice_name(part == PART_808 ? T_808 : T_909, v));
}

static const float DV_XY[] = {
    15.400f, 5.400f, 15.250f, 6.539f, 14.811f, 7.600f, 14.111f, 8.511f, 13.200f, 9.211f, 12.139f, 9.650f,
    11.000f, 9.800f, 9.861f, 9.650f, 8.800f, 9.211f, 7.889f, 8.511f, 7.189f, 7.600f, 6.750f, 6.539f,
    6.600f, 5.400f, 6.750f, 4.261f, 7.189f, 3.200f, 7.889f, 2.289f, 8.800f, 1.589f, 9.861f, 1.150f,
    11.000f, 1.000f, 12.139f, 1.150f, 13.200f, 1.589f, 14.111f, 2.289f, 14.811f, 3.200f, 15.250f, 4.261f,
    15.400f, 5.400f, 7.500f, 8.700f, 5.600f, 10.500f, 14.500f, 8.700f, 16.400f, 10.500f, 11.000f, 9.800f,
    11.000f, 11.000f, 10.000f, 10.800f, 12.000f, 10.800f, 18.500f, 3.000f, 18.261f, 3.427f, 17.562f, 3.825f,
    16.450f, 4.167f, 15.000f, 4.429f, 13.312f, 4.594f, 11.500f, 4.650f, 9.688f, 4.594f, 8.000f, 4.429f,
    6.550f, 4.167f, 5.438f, 3.825f, 4.739f, 3.427f, 4.500f, 3.000f, 4.739f, 2.573f, 5.438f, 2.175f,
    6.550f, 1.833f, 8.000f, 1.571f, 9.688f, 1.406f, 11.500f, 1.350f, 13.312f, 1.406f, 15.000f, 1.571f,
    16.450f, 1.833f, 17.562f, 2.175f, 18.261f, 2.573f, 18.500f, 3.000f, 4.500f, 3.000f, 4.500f, 7.700f,
    6.250f, 8.422f, 8.000f, 8.938f, 9.750f, 9.247f, 11.500f, 9.350f, 13.250f, 9.247f, 15.000f, 8.938f,
    16.750f, 8.422f, 18.500f, 7.700f, 18.500f, 3.000f, 5.000f, 6.700f, 6.625f, 7.356f, 8.250f, 7.825f,
    9.875f, 8.106f, 11.500f, 8.200f, 13.125f, 8.106f, 14.750f, 7.825f, 16.375f, 7.356f, 18.000f, 6.700f,
    7.000f, 4.700f, 7.000f, 7.500f, 16.000f, 4.700f, 16.000f, 7.500f, 8.500f, 9.300f, 9.500f, 7.900f,
    11.000f, 9.700f, 12.000f, 8.300f, 13.500f, 9.300f, 14.500f, 7.900f, 17.800f, 2.400f, 17.585f, 2.814f,
    16.956f, 3.200f, 15.955f, 3.531f, 14.650f, 3.786f, 13.131f, 3.945f, 11.500f, 4.000f, 9.869f, 3.945f,
    8.350f, 3.786f, 7.045f, 3.531f, 6.044f, 3.200f, 5.415f, 2.814f, 5.200f, 2.400f, 5.415f, 1.986f,
    6.044f, 1.600f, 7.045f, 1.269f, 8.350f, 1.014f, 9.869f, 0.855f, 11.500f, 0.800f, 13.131f, 0.855f,
    14.650f, 1.014f, 15.955f, 1.269f, 16.956f, 1.600f, 17.585f, 1.986f, 17.800f, 2.400f, 5.200f, 2.400f,
    5.200f, 7.700f, 6.775f, 8.269f, 8.350f, 8.675f, 9.925f, 8.919f, 11.500f, 9.000f, 13.075f, 8.919f,
    14.650f, 8.675f, 16.225f, 8.269f, 17.800f, 7.700f, 17.800f, 2.400f, 5.850f, 4.000f, 5.850f, 6.300f,
    17.150f, 4.000f, 17.150f, 6.300f, 6.300f, 8.000f, 5.500f, 10.800f, 16.700f, 8.000f, 17.500f, 10.800f,
    16.800f, 2.400f, 16.619f, 2.814f, 16.090f, 3.200f, 15.248f, 3.531f, 14.150f, 3.786f, 12.872f, 3.945f,
    11.500f, 4.000f, 10.128f, 3.945f, 8.850f, 3.786f, 7.752f, 3.531f, 6.910f, 3.200f, 6.381f, 2.814f,
    6.200f, 2.400f, 6.381f, 1.986f, 6.910f, 1.600f, 7.752f, 1.269f, 8.850f, 1.014f, 10.128f, 0.855f,
    11.500f, 0.800f, 12.872f, 0.855f, 14.150f, 1.014f, 15.248f, 1.269f, 16.090f, 1.600f, 16.619f, 1.986f,
    16.800f, 2.400f, 6.200f, 2.400f, 6.200f, 6.400f, 7.525f, 6.969f, 8.850f, 7.375f, 10.175f, 7.619f,
    11.500f, 7.700f, 12.825f, 7.619f, 14.150f, 7.375f, 15.475f, 6.969f, 16.800f, 6.400f, 16.800f, 2.400f,
    6.350f, 4.000f, 6.350f, 5.000f, 16.650f, 4.000f, 16.650f, 5.000f, 11.500f, 7.200f, 11.500f, 10.300f,
    16.000f, 2.400f, 15.847f, 2.814f, 15.397f, 3.200f, 14.682f, 3.531f, 13.750f, 3.786f, 12.665f, 3.945f,
    11.500f, 4.000f, 10.335f, 3.945f, 9.250f, 3.786f, 8.318f, 3.531f, 7.603f, 3.200f, 7.153f, 2.814f,
    7.000f, 2.400f, 7.153f, 1.986f, 7.603f, 1.600f, 8.318f, 1.269f, 9.250f, 1.014f, 10.335f, 0.855f,
    11.500f, 0.800f, 12.665f, 0.855f, 13.750f, 1.014f, 14.682f, 1.269f, 15.397f, 1.600f, 15.847f, 1.986f,
    16.000f, 2.400f, 7.000f, 2.400f, 7.000f, 5.800f, 8.125f, 6.369f, 9.250f, 6.775f, 10.375f, 7.019f,
    11.500f, 7.100f, 12.625f, 7.019f, 13.750f, 6.775f, 14.875f, 6.369f, 16.000f, 5.800f, 16.000f, 2.400f,
    6.750f, 4.000f, 6.750f, 4.400f, 16.250f, 4.000f, 16.250f, 4.400f, 11.500f, 7.200f, 11.500f, 10.300f,
    18.000f, 7.000f, 17.761f, 7.544f, 17.062f, 8.050f, 15.950f, 8.485f, 14.500f, 8.819f, 12.812f, 9.028f,
    11.000f, 9.100f, 9.188f, 9.028f, 7.500f, 8.819f, 6.050f, 8.485f, 4.938f, 8.050f, 4.239f, 7.544f,
    4.000f, 7.000f, 4.239f, 6.456f, 4.938f, 5.950f, 6.050f, 5.515f, 7.500f, 5.181f, 9.188f, 4.972f,
    11.000f, 4.900f, 12.812f, 4.972f, 14.500f, 5.181f, 15.950f, 5.515f, 17.062f, 5.950f, 17.761f, 6.456f,
    18.000f, 7.000f, 4.000f, 7.000f, 4.000f, 9.000f, 5.750f, 9.547f, 7.500f, 9.938f, 9.250f, 10.172f,
    11.000f, 10.250f, 12.750f, 10.172f, 14.500f, 9.938f, 16.250f, 9.547f, 18.000f, 9.000f, 18.000f, 7.000f,
    15.500f, 0.800f, 10.300f, 6.900f, 16.800f, 1.400f, 11.600f, 7.500f, 3.000f, 9.000f, 7.700f, 4.400f,
    9.400f, 6.200f, 7.000f, 8.700f, 12.000f, 8.200f, 15.800f, 3.400f, 18.000f, 5.200f, 14.200f, 10.000f,
    9.200f, 11.000f, 3.000f, 9.000f, 9.100f, 3.800f, 11.300f, 1.200f, 13.000f, 2.800f, 11.000f, 5.500f,
    9.100f, 3.800f, 18.500f, 1.300f, 20.500f, 0.300f, 20.000f, 4.000f, 22.000f, 4.400f, 3.500f, 5.400f,
    5.500f, 4.503f, 7.500f, 3.863f, 9.500f, 3.478f, 11.500f, 3.350f, 13.500f, 3.478f, 15.500f, 3.863f,
    17.500f, 4.503f, 19.500f, 5.400f, 17.500f, 5.969f, 15.500f, 6.375f, 13.500f, 6.619f, 11.500f, 6.700f,
    9.500f, 6.619f, 7.500f, 6.375f, 5.500f, 5.969f, 3.500f, 5.400f, 3.500f, 5.400f, 3.800f, 6.500f,
    5.725f, 7.047f, 7.650f, 7.438f, 9.575f, 7.672f, 11.500f, 7.750f, 13.425f, 7.672f, 15.350f, 7.438f,
    17.275f, 7.047f, 19.200f, 6.500f, 11.500f, 0.200f, 11.500f, 2.400f, 10.500f, 2.500f, 12.500f, 2.500f,
    10.300f, 8.100f, 12.700f, 8.100f, 11.500f, 8.200f, 11.500f, 11.600f, 3.500f, 3.600f, 5.500f, 2.725f,
    7.500f, 2.100f, 9.500f, 1.725f, 11.500f, 1.600f, 13.500f, 1.725f, 15.500f, 2.100f, 17.500f, 2.725f,
    19.500f, 3.600f, 17.500f, 4.191f, 15.500f, 4.612f, 13.500f, 4.866f, 11.500f, 4.950f, 9.500f, 4.866f,
    7.500f, 4.612f, 5.500f, 4.191f, 3.500f, 3.600f, 3.500f, 3.600f, 3.500f, 8.300f, 5.500f, 7.709f,
    7.500f, 7.287f, 9.500f, 7.034f, 11.500f, 6.950f, 13.500f, 7.034f, 15.500f, 7.287f, 17.500f, 7.709f,
    19.500f, 8.300f, 17.500f, 8.956f, 15.500f, 9.425f, 13.500f, 9.706f, 11.500f, 9.800f, 9.500f, 9.706f,
    7.500f, 9.425f, 5.500f, 8.956f, 3.500f, 8.300f, 3.500f, 8.300f, 11.500f, 0.100f, 11.500f, 0.900f,
    10.500f, 1.100f, 12.500f, 1.100f, 11.500f, 5.100f, 11.500f, 6.700f, 10.300f, 10.000f, 12.700f, 10.000f,
    11.500f, 10.100f, 11.500f, 11.700f, 2.000f, 5.200f, 9.000f, 3.300f, 9.500f, 2.789f, 10.000f, 2.406f,
    10.500f, 2.152f, 11.000f, 2.025f, 11.500f, 2.027f, 12.000f, 2.156f, 12.500f, 2.414f, 13.000f, 2.800f,
    21.000f, 3.700f, 19.062f, 4.533f, 17.000f, 5.181f, 14.812f, 5.645f, 12.500f, 5.925f, 10.062f, 6.020f,
    7.500f, 5.931f, 4.812f, 5.658f, 2.000f, 5.200f, 2.000f, 5.200f, 11.500f, 4.400f, 11.500f, 10.500f,
    8.400f, 10.700f, 14.500f, 10.700f, 18.000f, 0.600f, 19.200f, 1.700f, 2.500f, 5.600f, 3.725f, 5.314f,
    4.850f, 5.006f, 5.875f, 4.677f, 6.800f, 4.325f, 7.625f, 3.952f, 8.350f, 3.556f, 8.975f, 3.139f,
    9.500f, 2.700f, 10.000f, 2.394f, 10.500f, 2.175f, 11.000f, 2.044f, 11.500f, 2.000f, 12.000f, 2.044f,
    12.500f, 2.175f, 13.000f, 2.394f, 13.500f, 2.700f, 14.047f, 3.139f, 14.688f, 3.556f, 15.422f, 3.952f,
    16.250f, 4.325f, 17.172f, 4.677f, 18.188f, 5.006f, 19.297f, 5.314f, 20.500f, 5.600f, 18.250f, 6.300f,
    16.000f, 6.800f, 13.750f, 7.100f, 11.500f, 7.200f, 9.250f, 7.100f, 7.000f, 6.800f, 4.750f, 6.300f,
    2.500f, 5.600f, 2.500f, 5.600f, 11.500f, 2.100f, 11.500f, 10.700f, 8.500f, 10.700f, 14.500f, 10.700f,
    15.400f, 5.400f, 15.250f, 6.539f, 14.811f, 7.600f, 14.111f, 8.511f, 13.200f, 9.211f, 12.139f, 9.650f,
    11.000f, 9.800f, 9.861f, 9.650f, 8.800f, 9.211f, 7.889f, 8.511f, 7.189f, 7.600f, 6.750f, 6.539f,
    6.600f, 5.400f, 6.750f, 4.261f, 7.189f, 3.200f, 7.889f, 2.289f, 8.800f, 1.589f, 9.861f, 1.150f,
    11.000f, 1.000f, 12.139f, 1.150f, 13.200f, 1.589f, 14.111f, 2.289f, 14.811f, 3.200f, 15.250f, 4.261f,
    15.400f, 5.400f, 7.500f, 8.700f, 5.600f, 10.500f, 14.500f, 8.700f, 16.400f, 10.500f, 11.000f, 9.800f,
    11.000f, 11.000f, 10.000f, 10.800f, 12.000f, 10.800f, 18.500f, 3.000f, 18.261f, 3.427f, 17.562f, 3.825f,
    16.450f, 4.167f, 15.000f, 4.429f, 13.312f, 4.594f, 11.500f, 4.650f, 9.688f, 4.594f, 8.000f, 4.429f,
    6.550f, 4.167f, 5.438f, 3.825f, 4.739f, 3.427f, 4.500f, 3.000f, 4.739f, 2.573f, 5.438f, 2.175f,
    6.550f, 1.833f, 8.000f, 1.571f, 9.688f, 1.406f, 11.500f, 1.350f, 13.312f, 1.406f, 15.000f, 1.571f,
    16.450f, 1.833f, 17.562f, 2.175f, 18.261f, 2.573f, 18.500f, 3.000f, 4.500f, 3.000f, 4.500f, 7.700f,
    6.250f, 8.422f, 8.000f, 8.938f, 9.750f, 9.247f, 11.500f, 9.350f, 13.250f, 9.247f, 15.000f, 8.938f,
    16.750f, 8.422f, 18.500f, 7.700f, 18.500f, 3.000f, 5.000f, 6.700f, 6.625f, 7.356f, 8.250f, 7.825f,
    9.875f, 8.106f, 11.500f, 8.200f, 13.125f, 8.106f, 14.750f, 7.825f, 16.375f, 7.356f, 18.000f, 6.700f,
    7.000f, 4.700f, 7.000f, 7.500f, 16.000f, 4.700f, 16.000f, 7.500f, 8.500f, 9.300f, 9.500f, 7.900f,
    11.000f, 9.700f, 12.000f, 8.300f, 13.500f, 9.300f, 14.500f, 7.900f, 17.800f, 2.400f, 17.585f, 2.814f,
    16.956f, 3.200f, 15.955f, 3.531f, 14.650f, 3.786f, 13.131f, 3.945f, 11.500f, 4.000f, 9.869f, 3.945f,
    8.350f, 3.786f, 7.045f, 3.531f, 6.044f, 3.200f, 5.415f, 2.814f, 5.200f, 2.400f, 5.415f, 1.986f,
    6.044f, 1.600f, 7.045f, 1.269f, 8.350f, 1.014f, 9.869f, 0.855f, 11.500f, 0.800f, 13.131f, 0.855f,
    14.650f, 1.014f, 15.955f, 1.269f, 16.956f, 1.600f, 17.585f, 1.986f, 17.800f, 2.400f, 5.200f, 2.400f,
    5.200f, 7.700f, 6.775f, 8.269f, 8.350f, 8.675f, 9.925f, 8.919f, 11.500f, 9.000f, 13.075f, 8.919f,
    14.650f, 8.675f, 16.225f, 8.269f, 17.800f, 7.700f, 17.800f, 2.400f, 5.850f, 4.000f, 5.850f, 6.300f,
    17.150f, 4.000f, 17.150f, 6.300f, 6.300f, 8.000f, 5.500f, 10.800f, 16.700f, 8.000f, 17.500f, 10.800f,
    16.800f, 2.400f, 16.619f, 2.814f, 16.090f, 3.200f, 15.248f, 3.531f, 14.150f, 3.786f, 12.872f, 3.945f,
    11.500f, 4.000f, 10.128f, 3.945f, 8.850f, 3.786f, 7.752f, 3.531f, 6.910f, 3.200f, 6.381f, 2.814f,
    6.200f, 2.400f, 6.381f, 1.986f, 6.910f, 1.600f, 7.752f, 1.269f, 8.850f, 1.014f, 10.128f, 0.855f,
    11.500f, 0.800f, 12.872f, 0.855f, 14.150f, 1.014f, 15.248f, 1.269f, 16.090f, 1.600f, 16.619f, 1.986f,
    16.800f, 2.400f, 6.200f, 2.400f, 6.200f, 6.400f, 7.525f, 6.969f, 8.850f, 7.375f, 10.175f, 7.619f,
    11.500f, 7.700f, 12.825f, 7.619f, 14.150f, 7.375f, 15.475f, 6.969f, 16.800f, 6.400f, 16.800f, 2.400f,
    6.350f, 4.000f, 6.350f, 5.000f, 16.650f, 4.000f, 16.650f, 5.000f, 11.500f, 7.200f, 11.500f, 10.300f,
    16.000f, 2.400f, 15.847f, 2.814f, 15.397f, 3.200f, 14.682f, 3.531f, 13.750f, 3.786f, 12.665f, 3.945f,
    11.500f, 4.000f, 10.335f, 3.945f, 9.250f, 3.786f, 8.318f, 3.531f, 7.603f, 3.200f, 7.153f, 2.814f,
    7.000f, 2.400f, 7.153f, 1.986f, 7.603f, 1.600f, 8.318f, 1.269f, 9.250f, 1.014f, 10.335f, 0.855f,
    11.500f, 0.800f, 12.665f, 0.855f, 13.750f, 1.014f, 14.682f, 1.269f, 15.397f, 1.600f, 15.847f, 1.986f,
    16.000f, 2.400f, 7.000f, 2.400f, 7.000f, 5.800f, 8.125f, 6.369f, 9.250f, 6.775f, 10.375f, 7.019f,
    11.500f, 7.100f, 12.625f, 7.019f, 13.750f, 6.775f, 14.875f, 6.369f, 16.000f, 5.800f, 16.000f, 2.400f,
    6.750f, 4.000f, 6.750f, 4.400f, 16.250f, 4.000f, 16.250f, 4.400f, 11.500f, 7.200f, 11.500f, 10.300f,
    18.000f, 7.000f, 17.761f, 7.544f, 17.062f, 8.050f, 15.950f, 8.485f, 14.500f, 8.819f, 12.812f, 9.028f,
    11.000f, 9.100f, 9.188f, 9.028f, 7.500f, 8.819f, 6.050f, 8.485f, 4.938f, 8.050f, 4.239f, 7.544f,
    4.000f, 7.000f, 4.239f, 6.456f, 4.938f, 5.950f, 6.050f, 5.515f, 7.500f, 5.181f, 9.188f, 4.972f,
    11.000f, 4.900f, 12.812f, 4.972f, 14.500f, 5.181f, 15.950f, 5.515f, 17.062f, 5.950f, 17.761f, 6.456f,
    18.000f, 7.000f, 4.000f, 7.000f, 4.000f, 9.000f, 5.750f, 9.547f, 7.500f, 9.938f, 9.250f, 10.172f,
    11.000f, 10.250f, 12.750f, 10.172f, 14.500f, 9.938f, 16.250f, 9.547f, 18.000f, 9.000f, 18.000f, 7.000f,
    15.500f, 0.800f, 10.300f, 6.900f, 16.800f, 1.400f, 11.600f, 7.500f, 3.000f, 9.000f, 7.700f, 4.400f,
    9.400f, 6.200f, 7.000f, 8.700f, 12.000f, 8.200f, 15.800f, 3.400f, 18.000f, 5.200f, 14.200f, 10.000f,
    9.200f, 11.000f, 3.000f, 9.000f, 9.100f, 3.800f, 11.300f, 1.200f, 13.000f, 2.800f, 11.000f, 5.500f,
    9.100f, 3.800f, 18.500f, 1.300f, 20.500f, 0.300f, 20.000f, 4.000f, 22.000f, 4.400f, 8.400f, 1.000f,
    14.500f, 1.000f, 18.500f, 9.300f, 16.750f, 9.891f, 15.000f, 10.312f, 13.250f, 10.566f, 11.500f, 10.650f,
    9.750f, 10.566f, 8.000f, 10.312f, 6.250f, 9.891f, 4.500f, 9.300f, 8.400f, 1.000f, 7.600f, 2.500f,
    15.100f, 2.500f, 5.400f, 9.300f, 6.925f, 8.797f, 8.450f, 8.438f, 9.975f, 8.222f, 11.500f, 8.150f,
    13.025f, 8.222f, 14.550f, 8.438f, 16.075f, 8.797f, 17.600f, 9.300f, 2.000f, 5.200f, 9.000f, 3.300f,
    9.500f, 2.789f, 10.000f, 2.406f, 10.500f, 2.152f, 11.000f, 2.025f, 11.500f, 2.027f, 12.000f, 2.156f,
    12.500f, 2.414f, 13.000f, 2.800f, 21.000f, 3.700f, 19.062f, 4.533f, 17.000f, 5.181f, 14.812f, 5.645f,
    12.500f, 5.925f, 10.062f, 6.020f, 7.500f, 5.931f, 4.812f, 5.658f, 2.000f, 5.200f, 2.000f, 5.200f,
    11.500f, 4.400f, 11.500f, 10.500f, 8.400f, 10.700f, 14.500f, 10.700f, 18.000f, 0.600f, 19.200f, 1.700f,
    3.500f, 3.600f, 5.500f, 2.725f, 7.500f, 2.100f, 9.500f, 1.725f, 11.500f, 1.600f, 13.500f, 1.725f,
    15.500f, 2.100f, 17.500f, 2.725f, 19.500f, 3.600f, 17.500f, 4.191f, 15.500f, 4.612f, 13.500f, 4.866f,
    11.500f, 4.950f, 9.500f, 4.866f, 7.500f, 4.612f, 5.500f, 4.191f, 3.500f, 3.600f, 3.500f, 3.600f,
    3.500f, 8.300f, 5.500f, 7.709f, 7.500f, 7.287f, 9.500f, 7.034f, 11.500f, 6.950f, 13.500f, 7.034f,
    15.500f, 7.287f, 17.500f, 7.709f, 19.500f, 8.300f, 17.500f, 8.956f, 15.500f, 9.425f, 13.500f, 9.706f,
    11.500f, 9.800f, 9.500f, 9.706f, 7.500f, 9.425f, 5.500f, 8.956f, 3.500f, 8.300f, 3.500f, 8.300f,
    11.500f, 0.100f, 11.500f, 0.900f, 10.500f, 1.100f, 12.500f, 1.100f, 11.500f, 5.100f, 11.500f, 6.700f,
    10.300f, 10.000f, 12.700f, 10.000f, 11.500f, 10.100f, 11.500f, 11.700f, 3.500f, 5.400f, 5.500f, 4.503f,
    7.500f, 3.863f, 9.500f, 3.478f, 11.500f, 3.350f, 13.500f, 3.478f, 15.500f, 3.863f, 17.500f, 4.503f,
    19.500f, 5.400f, 17.500f, 5.969f, 15.500f, 6.375f, 13.500f, 6.619f, 11.500f, 6.700f, 9.500f, 6.619f,
    7.500f, 6.375f, 5.500f, 5.969f, 3.500f, 5.400f, 3.500f, 5.400f, 3.800f, 6.500f, 5.725f, 7.047f,
    7.650f, 7.438f, 9.575f, 7.672f, 11.500f, 7.750f, 13.425f, 7.672f, 15.350f, 7.438f, 17.275f, 7.047f,
    19.200f, 6.500f, 11.500f, 0.200f, 11.500f, 2.400f, 10.500f, 2.500f, 12.500f, 2.500f, 10.300f, 8.100f,
    12.700f, 8.100f, 11.500f, 8.200f, 11.500f, 11.600f,
};
typedef struct { uint16_t off; uint8_t n; } dv_poly_t;
static const dv_poly_t DV_POLY[] = {
    {0, 25},
    {25, 2},
    {27, 2},
    {29, 2},
    {31, 2},
    {33, 25},
    {58, 11},
    {69, 9},
    {78, 2},
    {80, 2},
    {82, 2},
    {84, 2},
    {86, 2},
    {88, 25},
    {113, 11},
    {124, 2},
    {126, 2},
    {128, 2},
    {130, 2},
    {132, 25},
    {157, 11},
    {168, 2},
    {170, 2},
    {172, 2},
    {174, 25},
    {199, 11},
    {210, 2},
    {212, 2},
    {214, 2},
    {216, 25},
    {241, 11},
    {252, 2},
    {254, 2},
    {256, 10},
    {266, 5},
    {271, 2},
    {273, 2},
    {275, 18},
    {293, 9},
    {302, 2},
    {304, 2},
    {306, 2},
    {308, 2},
    {310, 18},
    {328, 18},
    {346, 2},
    {348, 2},
    {350, 2},
    {352, 2},
    {354, 2},
    {356, 20},
    {376, 2},
    {378, 2},
    {380, 2},
    {382, 34},
    {416, 2},
    {418, 2},
    {420, 25},
    {445, 2},
    {447, 2},
    {449, 2},
    {451, 2},
    {453, 25},
    {478, 11},
    {489, 9},
    {498, 2},
    {500, 2},
    {502, 2},
    {504, 2},
    {506, 2},
    {508, 25},
    {533, 11},
    {544, 2},
    {546, 2},
    {548, 2},
    {550, 2},
    {552, 25},
    {577, 11},
    {588, 2},
    {590, 2},
    {592, 2},
    {594, 25},
    {619, 11},
    {630, 2},
    {632, 2},
    {634, 2},
    {636, 25},
    {661, 11},
    {672, 2},
    {674, 2},
    {676, 10},
    {686, 5},
    {691, 2},
    {693, 2},
    {695, 12},
    {707, 2},
    {709, 9},
    {718, 20},
    {738, 2},
    {740, 2},
    {742, 2},
    {744, 18},
    {762, 18},
    {780, 2},
    {782, 2},
    {784, 2},
    {786, 2},
    {788, 2},
    {790, 18},
    {808, 9},
    {817, 2},
    {819, 2},
    {821, 2},
    {823, 2},
};
typedef struct { uint8_t first, npoly; } dv_body_t;
static const dv_body_t DV_BODY[2][NDRUM] = {
    {
        {0, 5},
        {5, 8},
        {13, 6},
        {19, 5},
        {24, 5},
        {29, 4},
        {33, 4},
        {37, 6},
        {43, 7},
        {50, 4},
        {54, 3},
    },
    {
        {57, 5},
        {62, 8},
        {70, 6},
        {76, 5},
        {81, 5},
        {86, 4},
        {90, 4},
        {94, 3},
        {97, 4},
        {101, 7},
        {108, 6},
    },
};

/* R22 layout (main-area y, origin at the first row under the 20 px header). */
#define DV_MAIN(y_screen) ((y_screen) - HDR_H)
static int dv_band_h(int band) { return band == 2 ? KNOB_H : BAND_H; }

static void dv_header_battery(int x, int y)
{
    int lvl=plat_battery();uint16_t ink=lvl==0?SOUND_AMBER:SOUND_GRAY;
    /* Reserve the same material on messages/pickup/modal/ordinary headers. */
    rbox(x-2,y-3,22,14,R22_RADIUS,SOUND_FACE);
    if(lvl==4){ /* USB host power does not report the battery's charge. */
        cv_rect(x+4,y+2,7,5,ink);cv_rect(x+5,y,1,3,ink);cv_rect(x+9,y,1,3,ink);
        cv_rect(x+7,y+7,1,2,ink);cv_rect(x+5,y+8,5,1,ink);return;
    }
    cv_rect(x,y,14,1,ink);cv_rect(x,y+7,14,1,ink);
    cv_rect(x,y,1,8,ink);cv_rect(x+13,y,1,8,ink);cv_rect(x+14,y+2,2,4,ink);
    if(lvl<0){ /* Not sampled is distinct from measured empty. */
        for(int i=0;i<3;i++)cv_rect(x+3+i*3,y+4,1,1,ink);return;
    }
    int bars=lvl>3?3:lvl;
    for(int i=0;i<3;i++)cv_rect(x+2+i*4,y+2,3,4,i<bars?ink:SOUND_LINE);
}

static UI_DRAW void dv_body_pictogram(int t, int v, int x, int y, float scale, float env, uint16_t col)
{
    const dv_body_t *b;
    float s, cx = x + 11.0f * 3.0f, cy = y + 6.0f * 3.0f;
    int p, i, n;
    uint16_t ink;
    if (t != T_808) t = T_909;
    if (v < 0 || v >= NDRUM) return;
    b = &DV_BODY[t == T_808][v];
    s = 3.0f * scale * (1.0f + 0.06f * env);
    ink = env > 0.02f ? blend(col, C_WHITE, (int)(env * 6.0f + 0.5f)) : col;
    for (p = 0; p < b->npoly; p++) {
        const dv_poly_t *pl = &DV_POLY[b->first + p];
        float xy[128];
        n = pl->n;
        if (n > 64) n = 64;
        for (i = 0; i < n; i++) {
            float px = DV_XY[2 * (pl->off + i)];
            float py = DV_XY[2 * (pl->off + i) + 1];
            xy[2*i]     = (float)x + (px - 11.0f) * s + 11.0f * 3.0f;
            xy[2*i + 1] = (float)y + (py - 5.4f) * s + 5.4f * 3.0f;
            (void)cx; (void)cy;
        }
        for(int j=1;j<n;j++) cv_stroke_aa(xy[2*j-2],xy[2*j-1],xy[2*j],xy[2*j+1],3.6f*scale,ink);
    }
}

/* TUNE fork (R22 path M9 0V6Q9 11 16 11Q23 11 23 6V0M16 11V17) that bends with tune. */
static UI_DRAW void dv_tune_fork(float ox, float oy, float sx, float sy, float tune01, uint16_t col)
{
    float mid = 16.0f, half = 7.0f * (0.62f + 0.76f * tune01); /* 14-wide at default ~0.27 */
    float drop = 11.0f + 2.0f * (tune01 - 0.27f);
    float tip = 17.0f + 1.5f * (tune01 - 0.27f);
    float L = mid - half, R = mid + half;
    float xy[40];
    int n = 0;
    #define P(X,Y) do { xy[2*n]=(ox)+(X)*sx; xy[2*n+1]=(oy)+(Y)*sy; n++; } while(0)
    P(L,0);P(L,6);
    for(int i=1;i<=8;i++) {
        float t=(float)i/8.f,mt=1.f-t;
        P(mt*mt*L+2*mt*t*L+t*t*mid,mt*mt*6+2*mt*t*drop+t*t*drop);
    }
    for(int i=1;i<=8;i++) {
        float t=(float)i/8.f,mt=1.f-t;
        P(mt*mt*mid+2*mt*t*R+t*t*R,mt*mt*drop+2*mt*t*drop+t*t*6);
    }
    P(R,0);
    cv_poly_aa(xy, n, col);
    n = 0;
    P(mid, drop); P(mid, tip);
    cv_poly_aa(xy, n, col);
    #undef P
}

/* Live AA glyph in an R22 slot. Box origin is the path origin (unit 0..32, 0..17). */
static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)
{
    int role = drum_strip_role(r);
    int val = (int)(ev + (ev >= 0 ? 0.5f : -0.5f));
    const x0x_param_t *d = pref_desc(r);
    float q = d && d->max ? ev / (float)d->max : 0;
    float e = 0;
    uint32_t age = 0xffffffffu;
    int live = drum_live(r, &age, &e);
    int i, n;
    float xy[80];
    if (q < 0) q = 0;
    if (q > 1) q = 1;
    if (d && name_is(d,"Pitch")) {
        /* Pitch is the 909 BD multiplier, not Tune's sweep time constant.
         * Show the actual pitched carrier's cycles over a fixed 40ms window. */
        float tau=dg_si(r,0,matrix_value(PR(R_ENG,T_909,0,0)));
        float m=.01f*dg_si(r,4,matrix_value(PR(R_ENG,T_909,0,4)));
        float pm=dg_si(r,5,val),hz=engine_bd909_hz(tau,m,pm,100.f);
        n=0;
        for(i=0;i<=32;i++) {
            float t=(float)i/32.f;
            xy[2*n]=ox+(1.f+30.f*t)*sx;
            xy[2*n+1]=oy+(6.f-5.f*fm_sinf(6.28318531f*hz*.04f*t))*sy;n++;
        }
        cv_poly_aa(xy,n,col);return;
    }
    if (role == DG_TUNE || (d && name_is(d, "Tune"))) {
        dv_tune_fork(ox, oy, sx, sy, q, col);
        return;
    }
    if (role == DG_ATTACK) {
        /* Normalized rising-edge symbol: x encodes attack AMOUNT, not time.
         * The parameter and its percentage retain the engine's click amount.
         * Zero is vertical; larger amounts lengthen the visible rising ramp. */
        float end=1.f+30.f*q;
        n=0;
        xy[2*n]=ox+sx;xy[2*n+1]=oy+12.f*sy;n++;
        xy[2*n]=ox+end*sx;xy[2*n+1]=oy;n++;
        xy[2*n]=ox+31.f*sx;xy[2*n+1]=oy;n++;
        cv_poly_aa(xy, n, live && age < 30u ? C_WHITE : col);
        return;
    }
    if (role == DG_DECAY || role == DG_NTAIL) {
        float ms = 0, reach;
        int uu = engine_param_si(r.a, r.b, r.c, val, &ms);
        if (uu == X0X_U_S) ms *= 1000.0f;
        if (uu != X0X_U_MS && uu != X0X_U_S) ms = q * 4000.0f;
        if (role == DG_NTAIL) ms = 24.0f + 0.7f * ms;
        if (ms < 0) ms = 0;
        if (ms > 4000) ms = 4000;
        reach = 8.0f + 23.0f * fm_sqrtf(ms / 4000.0f); /* 8..31 */
        n = 0;
        for (i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f;
            float px,py;
            if(r.a==T_909 && (uu==X0X_U_MS || uu==X0X_U_S)) {
                /* The production 909 exponential reaches 1e-5 at Decay ms.
                 * A common 2s axis makes changing decay visibly meaningful. */
                px=1.f+30.f*t;
                py=12.f*(1.f-fm_exp2f(-16.60964047f*(2000.f*t)/(ms>0?ms:1.f)));
            } else {
                /* Untimed 808 controls remain raw controls, not invented ms. */
                px=1.f+t*reach;
                py=12.f*(1.f-fm_exp2f(-t*(2.2f+3.f*(1.f-q))));
                if(t*reach>reach-.01f)py=12.f;
            }
            xy[2*n] = ox + px * sx; xy[2*n+1] = oy + py * sy; n++;
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_BDSWEEP) { /* P.DPTH / PITCH: kick pitch drop, 0..100 ms */
        int c0 = 0, c4 = 4, c5 = 5;
        float tau = dg_si(r, c0, r.c == c0 ? val : matrix_value(PR(R_ENG, T_909, 0, c0)));
        float m = .01f * dg_si(r, c4, r.c == c4 ? val : matrix_value(PR(R_ENG, T_909, 0, c4)));
        float pm = dg_si(r, c5, r.c == c5 ? val : matrix_value(PR(R_ENG, T_909, 0, c5)));
        n = 0;
        for (i = 0; i <= 24; i++) {
            float tms = (float)i * 100.0f / 24.0f;
            float hz = engine_bd909_hz(tau, m, pm, tms);
            float yv = fm_log2f(hz / 40.0f) / 3.32f;
            if (yv < 0) yv = 0;
            if (yv > 1) yv = 1;
            xy[2*n] = ox + (31.0f * (float)i / 24.0f) * sx;
            xy[2*n+1] = oy + (12.0f * (1.0f - yv)) * sy;
            n++;
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_LPF) {
        /* Actual one-pole 808 BD tone response; logarithmic frequency axis.
         * bd_trigger sets c=1-exp(-2*pi*fc/sr), fc=300*20^(pot/127). */
        float hz=300.f*fm_exp2f(q*4.32192809f);
        float c=1.f-fm_exp2f(-6.28318531f*hz/44100.f*1.44269504f),a=1.f-c;
        n=0;
        for(i=0;i<=24;i++) {
            float t=(float)i/24.f,freq=100.f*fm_exp2f(t*7.78463485f);
            float den=1.f+a*a-2.f*a*fm_cosf(6.28318531f*freq/44100.f);
            float mag=den>0?c/fm_sqrtf(den):1.f;
            if(mag>1)mag=1;
            if(mag<0)mag=0;
            xy[2*n]=ox+31.f*t*sx;xy[2*n+1]=oy+(1.f+11.f*(1.f-mag))*sy;n++;
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_ACCENT) {
        /* four ticks, height follows accent */
        float hmax = 4.0f + 8.0f * q;
        for (i = 0; i < 4; i++) {
            float hx = 2.0f + i * 8.0f;
            float hh = hmax * (0.25f + 0.25f * (float)i);
            cv_line_aa(ox + hx * sx, oy + 12 * sy, ox + hx * sx, oy + (12.0f - hh) * sy, col);
        }
        return;
    }
    if (role == DG_SNAPPY || role == DG_SHELLS) {
        /* a live burst / two shells, scaled into the slot */
        float k = live ? (0.35f + 0.65f * e) : 1.0f;
        if (role == DG_SHELLS) {
            float tp = ev, ang = (tp <= 64 ? ev / 128.0f : 0.5f + (ev - 64.0f) * (0.5f / 63.0f)) * 1.5708f;
            float g1 = fm_cosf(ang) * 12.0f * k, g2 = fm_sinf(ang) * 12.0f * k;
            cv_line_aa(ox + 7 * sx, oy + 12 * sy, ox + 11 * sx, oy + (12 - g1) * sy, col);
            cv_line_aa(ox + 11 * sx, oy + (12 - g1) * sy, ox + 15 * sx, oy + 12 * sy, col);
            cv_line_aa(ox + 17 * sx, oy + 12 * sy, ox + 21 * sx, oy + (12 - g2) * sy, col);
            cv_line_aa(ox + 21 * sx, oy + (12 - g2) * sy, ox + 25 * sx, oy + 12 * sy, col);
        } else {
            n = 0;
            for (i = 0; i <= 16; i++) {
                float t = (float)i / 16.0f;
                float envp = fm_exp2f(-t * 3.5f);
                float h = q * k * envp * 12.0f;
                xy[2*n] = ox + (1 + t * 30) * sx;
                xy[2*n+1] = oy + (12.0f - h) * sy;
                n++;
            }
            cv_poly_aa(xy, n, col);
        }
        return;
    }
    /* fallback: a value-scaled arc */
    n = 0;
    for (i = 0; i <= 12; i++) {
        float t = (float)i / 12.0f;
        xy[2*n] = ox + (1 + t * 30) * sx;
        xy[2*n+1] = oy + (12.0f - 12.0f * q * fm_sinf(t * 3.1416f)) * sy;
        n++;
    }
    cv_poly_aa(xy, n, col);
}

static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)
{
    char num[16], unit[8], t[24];
    const felucca_font_t *f = large ? &FONT_S : &FONT_T;
    pref_value_of(r, ev_i, num, unit);
    t[0] = 0;
    put_s(put_s(t, num), unit);
    text_c(cx, y, f, t, col);
}

/* Format helpers already exist; text_c used above. */

static UI_DRAW void draw_drum_voice(int band)
{
    int off = band * BAND_H, bh = dv_band_h(band);
    int t = ui.part == PART_808 ? T_808 : T_909;
    int v = ui.sel[ui.part];
    int page = cur_page();
    int n = pg.n;
    int p, y, active;
    uint16_t col = part_col();
    uint16_t ink = col, gray = SOUND_GRAY, dimc = SOUND_DIM;
    uint32_t age = 0;
    float env = 0;
    int live;
    const dpart_t *dp;
    int len, s;
    int body_y, yb;
    cv_rect(0, 0, 240, bh, SOUND_BASE);

    /* body surface + banks: R22 y=24 (screen) => main y=4, height grows with bank count. */
    yb = DV_MAIN(24);
    /* Keep the lower voice/step panel compact. Spare space belongs to the
     * active controls, not an empty box between the pictogram and steps. */
    {
        int by = yb - off;
        int body_h;
        int used = 0;
        int ah;
        for (p = 0; p < n; p++) {
            int cells = 0, k;
            for (k = 0; k < 4; k++) if (pg.r[p][k].kind != R_NONE) cells++;
            ah = (p == page) ? (cells==1?56:72) : (cells>1?42:32);
            used += ah;
        }
        body_h = used + 4;
        if (by + body_h > 0 && by < bh)
            rbox(3, by, 234, body_h, 9, SOUND_FACE);
        /* banks */
        y = yb + 2;
        for (p = 0; p < n; p++) {
            int cells = 0, k, cx0;
            for (k = 0; k < 4; k++) if (pg.r[p][k].kind != R_NONE) cells++;
            if (cells < 1) cells = 1;
            active = (p == page);
            ah = active ? (cells==1?56:72) : (cells>1?42:32);
            if (active) {
                int ay = y - off;
                if (ay + ah > 0 && ay < bh)
                    rbox(6, ay, 228, ah, 9, RGB(36,48,54));
            }
            /* labels */
            {
                                int ly = y + 2 - off;
                char pgind[8], a[16], b[16];
                const x0x_param_t *pd0;
                a[0] = b[0] = 0;
                if (pg.r[p][0].kind == R_ENG && pg.r[p][0].b == NDRUM) {
                    put_s(a, "KIT"); put_s(b, pref_name(pg.r[p][0]));
                } else {
                    put_s(a, "SOUND");
                    pd0 = pref_desc(pg.r[p][0]);
                    /* Multi-control rows already label each cell; a second
                     * first-control summary would overprint that caption. */
                    if (cells==1 && pd0 && pd0->name) {
                        const char *nm = pd0->name;
                        if (name_is(pd0, "Pitch"))put_s(b,"PITCH");
                        else if(name_is(pd0,"P.Dpth"))put_s(b,"P.DEPTH");
                        else {
                            int i; char *d = b;
                            for (i = 0; nm[i] && i < 12; i++) {
                                char c = nm[i];
                                if (c >= 'a' && c <= 'z') c = (char)(c - 32);
                                *d++ = c;
                            }
                            *d = 0;
                        }
                    }
                }
                if (ly + 12 > 0 && ly < bh) {
                    cv_text(11, ly, &FONT_T, a, gray);
                    put_s(put_i(put_s(put_i(pgind, p + 1), "/"), pg.n), "");
                    text_r(232, ly, &FONT_T, pgind, gray);
                }
                /* The second line may enter this strip after the first line
                 * has left it. Test its own bounds, not the first line's. */
                if (b[0] && ly + 24 > 0 && ly + 12 < bh)
                    cv_text(11, ly + 12, &FONT_T, b, gray);
            }
            /* glyphs + values */
            cx0 = 11;
            for (k = 0; k < 4; k++) {
                pref_t r = pg.r[p][k];
                int ev_i, vx, gy, gx;
                float ev;
                if (r.kind == R_NONE) continue;
                ev_i = matrix_value(r);
                ev = ui_ease_float(r,ev_i);
                /* Only the pictogram eases; numeric state is committed now. */
                if (active && cells >= 2) {
                    vx=6+(228*(2*k+1))/(2*cells);
                    gx=vx-22;
                    gy=y+28;
                    {
                        char caption[16];const x0x_param_t *d=pref_desc(r);int j=0;
                        if(d&&d->name)for(;d->name[j]&&j<14;j++)caption[j]=d->name[j]>='a'&&d->name[j]<='z'?d->name[j]-32:d->name[j];
                        caption[j]=0;
                        if(d&&name_is(d,"P.Dpth"))put_s(caption,"P.DEPTH");
                        text_c(vx,y+14-off,&FONT_T,caption,gray);
                    }
                    float glyph_sy=1.50f+(float)(ah-72)*.025f;
                    if (gy-off < bh && (float)(gy-off)+18.f*glyph_sy+2.f > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy-off),1.375f,glyph_sy,ink);
                    if (y + ah - 20 - off < bh && y + ah - 20 - off + 16 > 0)
                        dv_value_text(r, ev_i, vx, y + ah - 20 - off, SOUND_WHITE, 1);
                } else if (active) {
                    /* single-control active bank (e.g. rimshot Tune): centered large glyph */
                    gx = 92; gy = y + 25;
                    if (gy - off < bh)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.8f, 1.8f, ink);
                    dv_value_text(r, ev_i, 214, y + 35 - off, SOUND_WHITE, 1);
                } else if(cells>1) {
                    /* Each real control retains its own labeled compact cell.
                     * A shared centre/right position would overprint siblings. */
                    gx=6+(228*(2*k+1))/(2*cells)-22;gy=y+29;
                    const x0x_param_t *d=pref_desc(r);char caption[16];int j=0;
                    if(d&&d->name)for(;d->name[j]&&j<14;j++)caption[j]=d->name[j]>='a'&&d->name[j]<='z'?d->name[j]-32:d->name[j];
                    caption[j]=0;
                    if(d&&name_is(d,"P.Dpth"))put_s(caption,"P.DEPTH");
                    text_c(gx+22,y+14-off,&FONT_T,caption,gray);
                    if(gy-off<bh&&gy+17-off>0)dv_glyph(r,ev,(float)gx,(float)(gy-off),.62f,.76f,gray);
                    dv_value_text(r,ev_i,gx+40,y+29-off,SOUND_WHITE,0);
                } else {
                    /* A single-control summary has one glyph and one value. */
                    gx = 98; gy = y + 5;
                    if (gy - off < bh && gy + 20 - off > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.3f, 1.5f, gray);
                    dv_value_text(r, ev_i, 214, y + 15 - off, SOUND_WHITE, 1);
                }
                (void)cx0;
            }
            y += ah;
        }
        body_y = yb + body_h + 4; /* gap before the lower panel */
    }

    /* lower panel: body pictogram, voice name, P1/16, 16-step strip */
    live = engine_drum_view(ui.part, v, &age, &env);
    if (!live) env = 0;
    {
        int py = body_y - off;
        int ph = 237 - HDR_H - body_y; /* to screen y=237 */
        int pat_i, slen;
        if (ph < 40) ph = 40;
        if (py + ph > 0 && py < bh) {
            rbox(3, py, 234, ph, 9, SOUND_FACE);
            /* Real voice identity and actual16 steps share the remaining
             * context surface. Rows reflow; no activity or data is invented. */
            int cols=ph>=88?4:8, rows=16/cols;
            int left=cols==4?124:118, pitch=(228-left)/cols;
            int step_top=body_y+19, step_h=(ph-24)/rows;
            float body_scale=ph<64?.65f:1.f;
            int body_height=(int)(47.f*body_scale);
            dv_body_pictogram(t,v,10,body_y+(ph-body_height)/2-off,body_scale,env,col);
            cv_text(11,body_y+3-off,&FONT_T,dv_voice_name(t,v),col);
            pat_i = seq.ppat[ui.part];
            slen = cur_pat()->drum[ui.part].len;
            if (slen < 1) slen = 16;
            {
                char rhs[16];
                put_s(put_i(put_s(put_i(put_s(rhs, "P"), pat_i + 1), " / "), slen > 16 ? 16 : slen), "");
                text_r(226,body_y+3-off,&FONT_T,rhs,gray);
                char id[16];put_i(put_s(id,t==T_808?"808 / ":"909 / "),v+1);
                cv_text(11,body_y+ph-14-off,&FONT_T,id,gray);
            }
            dp = &cur_pat()->drum[ui.part];
            len = dp->len;
            for (s = 0; s < 16; s++) {
                int sx = left+(s%cols)*pitch;
                int sy=step_top+(s/cols)*step_h-off;
                int hit = s < NSTEPS && s < len && sm_get(dp->hit[v], s);
                int curstep = seq.playing && (eng_step[TRK_DRUM + ui.part] % (len ? len : 16)) == s;
                uint16_t sc = hit ? col : SOUND_LINE;
                if (curstep) sc = C_WHITE;
                cv_rect(sx, sy, 11, 4, sc);
                {
                    char nn[4];put_i(nn, s + 1);
                    text_c(sx+5,sy+6,&FONT_T,nn,gray);
                }
            }
        }
    }
}


