float fastfloor(float x) {
    if (x > 0) {
        return (int)x;
    } else if (x < 0) {
        return (int)(x-1);
    } else {
        return 0;
    }
}

float fast_abs(float x) {
    return x > 0 ? x : -x;
}

float frac(float x) {
    return x - fastfloor(x);
}

float lerp(float a, float b, float t) {
    return t*b + (1-t)*a;
}
float bilinear(float a, float b, float c, float d, float t1, float t2) {
    return lerp(lerp(a,b,t1),lerp(c,d,t1),t2);
}
float trilinear(float a, float b, float c, float d, float e, float f, float g, float h, float x, float y, float z) {
    return lerp(bilinear(a,b,c,d,x,y), bilinear(e, f, g, h, x, y), z);
}

float u3(float x) {
    return x*x*(3-2*x);
}
float interp3(float a, float b, float t) {
    return lerp(a,b,u3(t));
}
float u5(float x) {
    return x*x*(10-3*x*(5-2*x));
}
float interp5(float a, float b, float t) {
    return lerp(a,b,u5(t));
}
float bilinear3(float a, float b, float c, float d, float t1, float t2) {
    return interp3(interp3(a,b,t1),interp3(c,d,t1),t2);
}
float trilinear3(float a, float b, float c, float d, float e, float f, float g, float h, float x, float y, float z) {
    return interp3(bilinear3(a,b,c,d,x,y), bilinear3(e, f, g, h, x, y), z);
}