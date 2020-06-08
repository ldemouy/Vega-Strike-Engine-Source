namespace GFXMatrices
{
    extern Matrix model, view;
    extern float projection[16];
    extern float invprojection[16];
    extern Matrix rotview;
} // namespace GFXMatrices
void MultFloatMatrix(float *ans, const float *a, const Matrix &b);
