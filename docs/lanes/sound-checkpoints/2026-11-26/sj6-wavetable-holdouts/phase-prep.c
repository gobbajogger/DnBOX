/* Transfer approximation derived at omega=0; no audio/phase data. */
float sj6_phase_prep(float x)
{
    const float D=.97079264196f;
    const float C=1.8060494887199716f; /* regenerated decimal below */
    return x*(D+C*x*x);
}
