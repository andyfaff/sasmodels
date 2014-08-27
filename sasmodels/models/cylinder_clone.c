real form_volume(real radius, real length);
real Iq(real q, real sld, real solvent_sld, real radius, real length);
real Iqxy(real qx, real qy, real sld, real solvent_sld, real radius, real length, real theta, real phi);

real form_volume(real radius, real length)
{
    return M_PI*radius*radius*length;
}

real Iq(real q,
    real sldCyl,
    real sldSolv,
    real radius,
    real length)
{
    const real h = REAL(0.5)*length;
    real summ = REAL(0.0);
    for (int i=0; i<76 ;i++) {
        //const real zi = ( Gauss76Z[i]*(uplim-lolim) + uplim + lolim )/2.0;
        const real zi = REAL(0.5)*(Gauss76Z[i]*M_PI_2 + M_PI_2);
        summ += Gauss76Wt[i] * CylKernel(q, radius, h, zi);
    }
    //const real form = (uplim-lolim)/2.0*summ;
    const real form = summ * M_PI_4;

    // Multiply by contrast^2, normalize by cylinder volume and convert to cm-1
    // NOTE that for this (Fournet) definition of the integral, one must MULTIPLY by Vcyl
    // The additional volume factor is for polydisperse volume normalization.
    const real s = (sldCyl - sldSolv) * form_volume(radius, length);
    return REAL(1.0e8) * form * s * s;
}

real Iqxy(real qx, real qy,
    real sldCyl,
    real sldSolv,
    real radius,
    real length,
    real cyl_theta,
    real cyl_phi)
{
    real sn, cn; // slots to hold sincos function output

    // Compute angle alpha between q and the cylinder axis
    SINCOS(cyl_theta*M_PI_180, sn, cn);
    // # The following correction factor exists in sasview, but it can't be
    // # right, so we are leaving it out for now.
    const real spherical_integration = fabs(cn)*M_PI_2;
    const real q = sqrt(qx*qx+qy*qy);
    const real cos_val = cn*cos(cyl_phi*M_PI_180)*(qx/q) + sn*(qy/q);
    const real alpha = acos(cos_val);

    // The following is CylKernel() / sin(alpha), but we are doing it in place
    // to avoid sin(alpha)/sin(alpha) for alpha = 0.  It is also a teensy bit
    // faster since we don't mulitply and divide sin(alpha).
    SINCOS(alpha, sn, cn);
    const real besarg = q*radius*sn;
    const real siarg = REAL(0.5)*q*length*cn;
    // lim_{x->0} J1(x)/x = 1/2,   lim_{x->0} sin(x)/x = 1
    const real bj = (besarg == REAL(0.0) ? REAL(0.5) : J1(besarg)/besarg);
    const real si = (siarg == REAL(0.0) ? REAL(1.0) : sin(siarg)/siarg);
    const real form = REAL(4.0)*bj*bj*si*si;

    // Multiply by contrast^2, normalize by cylinder volume and convert to cm-1
    // NOTE that for this (Fournet) definition of the integral, one must MULTIPLY by Vcyl
    // The additional volume factor is for polydisperse volume normalization.
    const real s = (sldCyl - sldSolv) * form_volume(radius, length);
    return REAL(1.0e8) * form * s * s * spherical_integration;
}