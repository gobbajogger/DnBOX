/* Host/native lab only, shared classifier. No write or fallback alias. */
enum { FDATA_READY=0, FDATA_MISSING=1, FDATA_LEGACY=2, FDATA_MISMATCH=3, FDATA_CORRUPT=4 };
static uint8_t fdata_lab_status(const uint8_t *d, size_t bytes, const uint8_t *expected)
{
    if (!d || bytes < 32u || (d[0] == 255 && d[1] == 255 && d[2] == 255 && d[3] == 255))
        return FDATA_MISSING;
    if (!memcmp(d, "S1PK", 4))
        return FDATA_LEGACY; /* identify/reject only; do not validate, adopt or overwrite */
    if (memcmp(d, "D9RD", 4) || memcmp(d, expected, 32))
        return FDATA_MISMATCH;
    return cold_lab_valid(d, bytes, expected) ? FDATA_READY : FDATA_CORRUPT;
}
