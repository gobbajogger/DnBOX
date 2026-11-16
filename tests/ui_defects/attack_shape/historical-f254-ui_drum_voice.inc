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

/* Xiaolin Wu, coverage 0..16 into px_blend. Shared with lead/glyph-aa-ease
 * (anim/_src/ctx_aa.c, anim/renderer-aa-ease.js). */
static int dv_ipart(float x) { return (int)fm_floorf(x); }
static float dv_fpart(float x) { return x - fm_floorf(x); }
static float dv_rfpart(float x) { return 1.0f - dv_fpart(x); }
static void cv_line_aa(float x0, float y0, float x1, float y1, uint16_t col)
{
    int steep = fm_fabsf(y1 - y0) > fm_fabsf(x1 - x0);
    float dx, dy, grad, xend, yend, xgap, intery;
    int xpxl1, ypxl1, xpxl2, ypxl2, x;
    if (steep) { float t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
    if (x0 > x1) { float t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    dx = x1 - x0; dy = y1 - y0;
    grad = (dx == 0.0f) ? 1.0f : dy / dx;
    xend = fm_floorf(x0 + 0.5f);
    yend = y0 + grad * (xend - x0);
    xgap = dv_rfpart(x0 + 0.5f);
    xpxl1 = (int)xend; ypxl1 = dv_ipart(yend);
    if (steep) {
        px_blend(ypxl1,     xpxl1, col, (uint32_t)(dv_rfpart(yend) * xgap * 16.0f + 0.5f));
        px_blend(ypxl1 + 1, xpxl1, col, (uint32_t)(dv_fpart(yend)  * xgap * 16.0f + 0.5f));
    } else {
        px_blend(xpxl1, ypxl1,     col, (uint32_t)(dv_rfpart(yend) * xgap * 16.0f + 0.5f));
        px_blend(xpxl1, ypxl1 + 1, col, (uint32_t)(dv_fpart(yend)  * xgap * 16.0f + 0.5f));
    }
    intery = yend + grad;
    xend = fm_floorf(x1 + 0.5f);
    yend = y1 + grad * (xend - x1);
    xgap = dv_fpart(x1 + 0.5f);
    xpxl2 = (int)xend; ypxl2 = dv_ipart(yend);
    if (steep) {
        px_blend(ypxl2,     xpxl2, col, (uint32_t)(dv_rfpart(yend) * xgap * 16.0f + 0.5f));
        px_blend(ypxl2 + 1, xpxl2, col, (uint32_t)(dv_fpart(yend)  * xgap * 16.0f + 0.5f));
    } else {
        px_blend(xpxl2, ypxl2,     col, (uint32_t)(dv_rfpart(yend) * xgap * 16.0f + 0.5f));
        px_blend(xpxl2, ypxl2 + 1, col, (uint32_t)(dv_fpart(yend)  * xgap * 16.0f + 0.5f));
    }
    for (x = xpxl1 + 1; x < xpxl2; x++) {
        if (steep) {
            px_blend(dv_ipart(intery),     x, col, (uint32_t)(dv_rfpart(intery) * 16.0f + 0.5f));
            px_blend(dv_ipart(intery) + 1, x, col, (uint32_t)(dv_fpart(intery)  * 16.0f + 0.5f));
        } else {
            px_blend(x, dv_ipart(intery),     col, (uint32_t)(dv_rfpart(intery) * 16.0f + 0.5f));
            px_blend(x, dv_ipart(intery) + 1, col, (uint32_t)(dv_fpart(intery)  * 16.0f + 0.5f));
        }
        intery += grad;
    }
}
static void cv_poly_aa(const float *xy, int n, uint16_t col)
{
    int i;
    for (i = 1; i < n; i++)
        cv_line_aa(xy[2*(i-1)], xy[2*(i-1)+1], xy[2*i], xy[2*i+1], col);
}

/* Settle: |y-target|/|step| <= 0.5 % and painted error < 0.5 px, within
 * 180 ms at 60 fps, critically damped, no overshoot. ω = 5/0.12 = 41.667
 * (0.47 % at t=180 ms). See anim/ANIM-SPEC.md. */
#define UI_EASE_OMEGA (5.0f / 0.12f)
#define UI_EASE_SLOTS 8
typedef struct { float y, vel; int tgt; uint32_t key; uint8_t on; } ui_ease_t;
static ui_ease_t dv_ease[UI_EASE_SLOTS];
static uint32_t dv_ease_ms;
static void dv_ease_tick(void)
{
    uint32_t now = plat_ms();
    float dt;
    int i;
    if (!dv_ease_ms) { dv_ease_ms = now; return; }
    dt = (float)(now - dv_ease_ms) / 1000.0f;
    if (dt <= 0.0f) return;
    if (dt > 0.050f) dt = 0.050f;
    dv_ease_ms = now;
    for (i = 0; i < UI_EASE_SLOTS; i++) {
        ui_ease_t *e = &dv_ease[i];
        float A, B, ex;
        if (!e->on) continue;
        A = e->y - (float)e->tgt;
        B = e->vel + UI_EASE_OMEGA * A;
        ex = fm_exp2f(-UI_EASE_OMEGA * dt * 1.44269504f); /* exp(-w dt) */
        e->y = (float)e->tgt + (A + B * dt) * ex;
        e->vel = (B - UI_EASE_OMEGA * (A + B * dt)) * ex;
    }
}
static float dv_eased(uint32_t key, int tgt)
{
    int i, free = -1;
    for (i = 0; i < UI_EASE_SLOTS; i++) {
        if (dv_ease[i].on && dv_ease[i].key == key) {
            dv_ease[i].tgt = tgt;
            return dv_ease[i].y;
        }
        if (!dv_ease[i].on && free < 0) free = i;
    }
    if (free < 0) free = 0;
    dv_ease[free].on = 1;
    dv_ease[free].key = key;
    dv_ease[free].tgt = tgt;
    dv_ease[free].y = (float)tgt;
    dv_ease[free].vel = 0;
    return (float)tgt;
}

/* R22 layout (main-area y, origin at the first row under the 20 px header). */
#define DV_MAIN(y_screen) ((y_screen) - HDR_H)
static int dv_band_h(int band) { return band == 2 ? KNOB_H : BAND_H; }

static void dv_header_battery(int x, int y)
{
    int lvl = plat_battery();
    int bars, i;
    uint16_t ink = SOUND_GRAY;
    if (lvl < 0) lvl = 5;
    bars = lvl > 5 ? 5 : lvl;
    if (bars < 0) bars = 0;
    /* r20 battery: 14 x 8 outline, three cells. */
    rbox(x, y, 14, 8, 1, SOUND_FACE);
    cv_rect(x, y, 14, 1, ink);
    cv_rect(x, y+7, 14, 1, ink);
    cv_rect(x, y, 1, 8, ink);
    cv_rect(x+13, y, 1, 8, ink);
    cv_rect(x+14, y+2, 2, 4, ink);
    for (i = 0; i < 3; i++) {
        uint16_t c = (i < (bars * 3 + 4) / 5) ? ink : SOUND_LINE;
        cv_rect(x+2+i*4, y+2, 3, 4, c);
    }
}

static void dv_body_pictogram(int t, int v, int x, int y, float scale, float env, uint16_t col)
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
        cv_poly_aa(xy, n, ink);
    }
}

/* TUNE fork (R22 path M9 0V6Q9 11 16 11Q23 11 23 6V0M16 11V17) that bends with tune. */
static void dv_tune_fork(float ox, float oy, float sx, float sy, float tune01, uint16_t col)
{
    float mid = 16.0f, half = 7.0f * (0.62f + 0.76f * tune01); /* 14-wide at default ~0.27 */
    float drop = 11.0f + 2.0f * (tune01 - 0.27f);
    float tip = 17.0f + 1.5f * (tune01 - 0.27f);
    float L = mid - half, R = mid + half;
    float xy[32];
    int n = 0;
    #define P(X,Y) do { xy[2*n]=(ox)+(X)*sx; xy[2*n+1]=(oy)+(Y)*sy; n++; } while(0)
    P(L, 0); P(L, 6);
    P(L, drop); P(mid, drop); P(R, drop);
    P(R, 6); P(R, 0);
    cv_poly_aa(xy, n, col);
    n = 0;
    P(mid, drop); P(mid, tip);
    cv_poly_aa(xy, n, col);
    #undef P
}

/* Live AA glyph in an R22 slot. Box origin is the path origin (unit 0..32, 0..17). */
static void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)
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
    if (q < 0) q = 0; if (q > 1) q = 1;
    if (role == DG_TUNE || (d && (name_is(d, "Tune") || name_is(d, "Pitch")))) {
        dv_tune_fork(ox, oy, sx, sy, q, col);
        return;
    }
    if (role == DG_ATTACK) {
        /* stamp M1 12C15 12 24 9 31 0: rise steepens / starts later as attack grows */
        float x1 = 1.0f, y0 = 12.0f;
        float c1x = 8.0f + 10.0f * q, c1y = 12.0f;
        float c2x = 18.0f + 6.0f * q, c2y = 9.0f - 6.0f * q;
        float x2 = 31.0f, y2 = 0.0f;
        n = 0;
        for (i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f, mt = 1.0f - t;
            float px = mt*mt*mt*x1 + 3*mt*mt*t*c1x + 3*mt*t*t*c2x + t*t*t*x2;
            float py = mt*mt*mt*y0 + 3*mt*mt*t*c1y + 3*mt*t*t*c2y + t*t*t*y2;
            xy[2*n] = ox + px * sx; xy[2*n+1] = oy + py * sy; n++;
        }
        cv_poly_aa(xy, n, live && age < 30u ? C_WHITE : col);
        return;
    }
    if (role == DG_DECAY || role == DG_NTAIL) {
        float ms = 0, u, reach;
        int uu = engine_param_si(r.a, r.b, r.c, val, &ms);
        if (uu == X0X_U_S) ms *= 1000.0f;
        if (uu != X0X_U_MS && uu != X0X_U_S) ms = q * 4000.0f;
        if (role == DG_NTAIL) ms = 24.0f + 0.7f * ms;
        if (ms < 0) ms = 0; if (ms > 4000) ms = 4000;
        reach = 8.0f + 23.0f * fm_sqrtf(ms / 4000.0f); /* 8..31 */
        n = 0;
        for (i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f;
            float px = 1.0f + t * reach;
            /* exp-like fall from 0 to 12 */
            float py = 12.0f * (1.0f - fm_exp2f(-t * (2.2f + 3.0f * (1.0f - q))));
            if (t * reach > reach - 0.01f) py = 12.0f;
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
            if (yv < 0) yv = 0; if (yv > 1) yv = 1;
            xy[2*n] = ox + (31.0f * (float)i / 24.0f) * sx;
            xy[2*n+1] = oy + (12.0f * (1.0f - yv)) * sy;
            n++;
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_LPF) {
        float hz = 300.0f * fm_exp2f(q * 4.3219f);
        float p = fm_log2f(hz / 100.0f) / fm_log2f(100.0f);
        if (p < 0) p = 0; if (p > 1) p = 1;
        float knee = 31.0f * p;
        n = 0;
        xy[2*n] = ox + 0 * sx; xy[2*n+1] = oy + 2 * sy; n++;
        xy[2*n] = ox + knee * sx; xy[2*n+1] = oy + 2 * sy; n++;
        xy[2*n] = ox + (knee + 12.0f > 31 ? 31 : knee + 12.0f) * sx;
        xy[2*n+1] = oy + 12 * sy; n++;
        if (knee + 12.0f < 31) {
            xy[2*n] = ox + 31 * sx; xy[2*n+1] = oy + 12 * sy; n++;
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

static void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)
{
    char num[16], unit[8], t[24];
    const felucca_font_t *f = large ? &FONT_B : &FONT_T;
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
    dv_ease_tick();
    cv_rect(0, 0, 240, bh, SOUND_BASE);

    /* body surface + banks: R22 y=24 (screen) => main y=4, height grows with bank count. */
    yb = DV_MAIN(24);
    /* active field 62 if the page has 4 cells, else 34; inactive 36. */
    {
        int by = yb - off;
        int body_h;
        int used = 0;
        int ah;
        for (p = 0; p < n; p++) {
            int cells = 0, k;
            for (k = 0; k < 4; k++) if (pg.r[p][k].kind != R_NONE) cells++;
            ah = (p == page) ? (cells >= 3 ? 62 : 34) : 36;
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
            ah = active ? (cells >= 3 ? 62 : 34) : 36;
            if (active) {
                int ay = y - off;
                if (ay + ah > 0 && ay < bh)
                    rbox(6, ay, 228, ah, 4, SOUND_SELECT);
            }
            /* labels */
            {
                const char *g = pg.title[p];
                int ly = y + 2 - off;
                char pgind[8], a[16], b[16];
                const x0x_param_t *pd0;
                a[0] = b[0] = 0;
                if (pg.r[p][0].kind == R_ENG && pg.r[p][0].b == NDRUM) {
                    put_s(a, "KIT"); put_s(b, "ACCENT");
                } else {
                    put_s(a, "SOUND");
                    pd0 = pref_desc(pg.r[p][0]);
                    if (!active && p > 0 && pd0 && pd0->name) {
                        const char *nm = pd0->name;
                        if (name_is(pd0, "Pitch") || name_is(pd0, "P.Dpth"))
                            put_s(b, "PITCH");
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
                    if (b[0]) cv_text(11, ly + 12, &FONT_T, b, gray);
                    put_s(put_i(put_s(put_i(pgind, p + 1), "/"), pg.n), "");
                    text_r(232, ly, &FONT_T, pgind, gray);
                }
            }
            /* glyphs + values */
            cx0 = 11;
            for (k = 0; k < 4; k++) {
                pref_t r = pg.r[p][k];
                int ev_i, vx, gy, gx;
                float ev;
                uint32_t key;
                if (r.kind == R_NONE) continue;
                ev_i = matrix_value(r);
                key = ((uint32_t)r.kind << 24) | ((uint32_t)r.a << 16) | ((uint32_t)r.b << 8) | (uint32_t)r.c;
                ev = dv_eased(key, ev_i);
                ev_i = (int)(ev + (ev >= 0 ? 0.5f : -0.5f));
                if (active && cells >= 3) {
                    gx = 11 + k * 58;
                    gy = y + 12;
                    vx = gx + 22;
                    if (gy - off < bh && gy + 30 - off > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.375f, 1.50f, ink);
                    if (y + 52 - off < bh && y + 52 - off + 16 > 0)
                        dv_value_text(r, ev_i, vx, y + 52 - off, ink, 1);
                } else if (active) {
                    /* single-control active bank (e.g. rimshot Tune): centered large glyph */
                    gx = 104; gy = y + 4;
                    if (gy - off < bh)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.2f, 1.3f, ink);
                    dv_value_text(r, ev_i, 220, y + 16 - off, ink, 1);
                } else {
                    /* compact summary: glyph in the middle, value under 2/3 */
                    gx = 104; gy = y + 8;
                    if (gy - off < bh && gy + 20 - off > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 0.95f, 1.05f, dimc);
                    dv_value_text(r, ev_i, 220, y + 16 - off, gray, 0);
                }
                (void)cx0;
            }
            y += ah;
        }
        body_y = yb + body_h + 4; /* gap before the lower panel */
        if (body_y < DV_MAIN(168) && n >= 3) body_y = DV_MAIN(168);
        if (body_y < DV_MAIN(132) && n == 2) body_y = DV_MAIN(132);
        if (body_y < DV_MAIN(104) && n <= 1) body_y = DV_MAIN(104);
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
            dv_body_pictogram(t, v, 10, body_y + 5 - off, 1.0f, env, col);
            cv_text(88, body_y + 18 - off, &FONT_T, dv_voice_name(t, v), col);
            pat_i = seq.ppat[ui.part];
            slen = cur_pat()->drum[ui.part].len;
            if (slen < 1) slen = 16;
            {
                char rhs[16];
                put_s(put_i(put_s(put_i(put_s(rhs, "P"), pat_i + 1), " / "), slen > 16 ? 16 : slen), "");
                text_r(234 - 8, body_y + 18 - off, &FONT_T, rhs, gray);
            }
            dp = &cur_pat()->drum[ui.part];
            len = dp->len;
            for (s = 0; s < 16; s++) {
                int sx = 11 + s * 14;
                int sy = body_y + ph - 26 - off;
                int hit = s < NSTEPS && s < len && sm_get(dp->hit[v], s);
                int curstep = seq.playing && (eng_step[TRK_DRUM + ui.part] % (len ? len : 16)) == s;
                uint16_t sc = hit ? col : SOUND_LINE;
                if (curstep) sc = C_WHITE;
                cv_rect(sx, sy, 11, 4, sc);
                {
                    char nn[4];
                    put_i(nn, s + 1);
                    text_c(sx + 5, sy + 10, &FONT_T, nn, gray);
                }
            }
        }
    }
}


