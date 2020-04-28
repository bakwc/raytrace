

#define LINMATH_H_DEFINE_VEC(n) \
typedef float vec##n[n]; \
static inline void vec##n##_add(vec##n r, vec##n const a, vec##n const b) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = a[i] + b[i]; \
} \
static inline void vec##n##_sub(vec##n r, vec##n const a, vec##n const b) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = a[i] - b[i]; \
} \
static inline void vec##n##_scale(vec##n r, vec##n const v, float const s) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = v[i] * s; \
} \
static inline void vec##n##_set(vec##n r, vec##n const v) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = v[i]; \
} \
static inline float vec##n##_mul_inner(vec##n const a, vec##n const b) \
{ \
	float p = 0.; \
	int i; \
	for(i=0; i<n; ++i) \
		p += b[i]*a[i]; \
	return p; \
} \
static inline float vec##n##_len(vec##n const v) \
{ \
	return (float) sqrt(vec##n##_mul_inner(v,v)); \
} \
static inline void vec##n##_norm(vec##n r, vec##n const v) \
{ \
	float k = 1.f / vec##n##_len(v); \
	vec##n##_scale(r, v, k); \
}

LINMATH_H_DEFINE_VEC(2)
LINMATH_H_DEFINE_VEC(3)
LINMATH_H_DEFINE_VEC(4)

static inline void vec3_mul_cross(vec3 r, vec3 const a, vec3 const b)
{
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
}

static inline void vec3_reflect(vec3 r, vec3 const v, vec3 const n)
{
    float p  = 2.f*vec3_mul_inner(v, n);
    int i;
    for(i=0;i<3;++i)
        r[i] = v[i] - p*n[i];
}

static inline void vec3_reflect2(vec3 r, vec3 const v, vec3 const n)
{
    float p  = 2.f*vec3_mul_inner(v, n);
    int i;
    for(i=0;i<3;++i)
        r[i] = p*n[i] - v[i];
    vec3_norm(r, r);
}

static inline void vec4_mul_cross(vec4 r, vec4 a, vec4 b)
{
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
    r[3] = 1.f;
}

static inline void vec4_reflect(vec4 r, vec4 v, vec4 n)
{
    float p  = 2.f*vec4_mul_inner(v, n);
    int i;
    for(i=0;i<4;++i)
        r[i] = v[i] - p*n[i];
}

typedef vec4 mat4x4[4];
static inline void mat4x4_identity(mat4x4 M)
{
    int i, j;
    for(i=0; i<4; ++i)
        for(j=0; j<4; ++j)
            M[i][j] = i==j ? 1.f : 0.f;
}
static inline void mat4x4_dup(mat4x4 M, mat4x4 N)
{
    int i, j;
    for(i=0; i<4; ++i)
        for(j=0; j<4; ++j)
            M[i][j] = N[i][j];
}
static inline void mat4x4_row(vec4 r, mat4x4 M, int i)
{
    int k;
    for(k=0; k<4; ++k)
        r[k] = M[k][i];
}
static inline void mat4x4_col(vec4 r, mat4x4 M, int i)
{
    int k;
    for(k=0; k<4; ++k)
        r[k] = M[i][k];
}
static inline void mat4x4_transpose(mat4x4 M, mat4x4 N)
{
    int i, j;
    for(j=0; j<4; ++j)
        for(i=0; i<4; ++i)
            M[i][j] = N[j][i];
}
static inline void mat4x4_add(mat4x4 M, mat4x4 a, mat4x4 b)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_add(M[i], a[i], b[i]);
}
static inline void mat4x4_sub(mat4x4 M, mat4x4 a, mat4x4 b)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_sub(M[i], a[i], b[i]);
}
static inline void mat4x4_scale(mat4x4 M, mat4x4 a, float k)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_scale(M[i], a[i], k);
}
static inline void mat4x4_scale_aniso(mat4x4 M, mat4x4 a, float x, float y, float z)
{
    int i;
    vec4_scale(M[0], a[0], x);
    vec4_scale(M[1], a[1], y);
    vec4_scale(M[2], a[2], z);
    for(i = 0; i < 4; ++i) {
        M[3][i] = a[3][i];
    }
}
static inline void mat4x4_mul(mat4x4 M, mat4x4 a, mat4x4 b)
{
    mat4x4 temp;
    int k, r, c;
    for(c=0; c<4; ++c) for(r=0; r<4; ++r) {
            temp[c][r] = 0.f;
            for(k=0; k<4; ++k)
                temp[c][r] += a[k][r] * b[c][k];
        }
    mat4x4_dup(M, temp);
}
static inline void mat4x4_mul_vec4(vec4 r, mat4x4 M, vec4 v)
{
    int i, j;
    for(j=0; j<4; ++j) {
        r[j] = 0.f;
        for(i=0; i<4; ++i)
            r[j] += M[i][j] * v[i];
    }
}
static inline void mat4x4_translate(mat4x4 T, float x, float y, float z)
{
    mat4x4_identity(T);
    T[3][0] = x;
    T[3][1] = y;
    T[3][2] = z;
}
static inline void mat4x4_translate_in_place(mat4x4 M, float x, float y, float z)
{
    vec4 t = {x, y, z, 0};
    vec4 r;
    int i;
    for (i = 0; i < 4; ++i) {
        mat4x4_row(r, M, i);
        M[3][i] += vec4_mul_inner(r, t);
    }
}
static inline void mat4x4_from_vec3_mul_outer(mat4x4 M, vec3 a, vec3 b)
{
    int i, j;
    for(i=0; i<4; ++i) for(j=0; j<4; ++j)
            M[i][j] = i<3 && j<3 ? a[i] * b[j] : 0.f;
}
static inline void mat4x4_rotate(mat4x4 R, mat4x4 M, float x, float y, float z, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    vec3 u = {x, y, z};

    if(vec3_len(u) > 1e-4) {
        mat4x4 T, C, S = {{0}};

        vec3_norm(u, u);
        mat4x4_from_vec3_mul_outer(T, u, u);

        S[1][2] =  u[0];
        S[2][1] = -u[0];
        S[2][0] =  u[1];
        S[0][2] = -u[1];
        S[0][1] =  u[2];
        S[1][0] = -u[2];

        mat4x4_scale(S, S, s);

        mat4x4_identity(C);
        mat4x4_sub(C, C, T);

        mat4x4_scale(C, C, c);

        mat4x4_add(T, T, C);
        mat4x4_add(T, T, S);

        T[3][3] = 1.;
        mat4x4_mul(R, M, T);
    } else {
        mat4x4_dup(R, M);
    }
}
static inline void mat4x4_rotate_X(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
            {1.f, 0.f, 0.f, 0.f},
            {0.f,   c,   s, 0.f},
            {0.f,  -s,   c, 0.f},
            {0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_rotate_Y(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
            {   c, 0.f,   s, 0.f},
            { 0.f, 1.f, 0.f, 0.f},
            {  -s, 0.f,   c, 0.f},
            { 0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_rotate_Z(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
            {   c,   s, 0.f, 0.f},
            {  -s,   c, 0.f, 0.f},
            { 0.f, 0.f, 1.f, 0.f},
            { 0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_invert(mat4x4 T, mat4x4 M)
{
    float idet;
    float s[6];
    float c[6];
    s[0] = M[0][0]*M[1][1] - M[1][0]*M[0][1];
    s[1] = M[0][0]*M[1][2] - M[1][0]*M[0][2];
    s[2] = M[0][0]*M[1][3] - M[1][0]*M[0][3];
    s[3] = M[0][1]*M[1][2] - M[1][1]*M[0][2];
    s[4] = M[0][1]*M[1][3] - M[1][1]*M[0][3];
    s[5] = M[0][2]*M[1][3] - M[1][2]*M[0][3];

    c[0] = M[2][0]*M[3][1] - M[3][0]*M[2][1];
    c[1] = M[2][0]*M[3][2] - M[3][0]*M[2][2];
    c[2] = M[2][0]*M[3][3] - M[3][0]*M[2][3];
    c[3] = M[2][1]*M[3][2] - M[3][1]*M[2][2];
    c[4] = M[2][1]*M[3][3] - M[3][1]*M[2][3];
    c[5] = M[2][2]*M[3][3] - M[3][2]*M[2][3];

    /* Assumes it is invertible */
    idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

    T[0][0] = ( M[1][1] * c[5] - M[1][2] * c[4] + M[1][3] * c[3]) * idet;
    T[0][1] = (-M[0][1] * c[5] + M[0][2] * c[4] - M[0][3] * c[3]) * idet;
    T[0][2] = ( M[3][1] * s[5] - M[3][2] * s[4] + M[3][3] * s[3]) * idet;
    T[0][3] = (-M[2][1] * s[5] + M[2][2] * s[4] - M[2][3] * s[3]) * idet;

    T[1][0] = (-M[1][0] * c[5] + M[1][2] * c[2] - M[1][3] * c[1]) * idet;
    T[1][1] = ( M[0][0] * c[5] - M[0][2] * c[2] + M[0][3] * c[1]) * idet;
    T[1][2] = (-M[3][0] * s[5] + M[3][2] * s[2] - M[3][3] * s[1]) * idet;
    T[1][3] = ( M[2][0] * s[5] - M[2][2] * s[2] + M[2][3] * s[1]) * idet;

    T[2][0] = ( M[1][0] * c[4] - M[1][1] * c[2] + M[1][3] * c[0]) * idet;
    T[2][1] = (-M[0][0] * c[4] + M[0][1] * c[2] - M[0][3] * c[0]) * idet;
    T[2][2] = ( M[3][0] * s[4] - M[3][1] * s[2] + M[3][3] * s[0]) * idet;
    T[2][3] = (-M[2][0] * s[4] + M[2][1] * s[2] - M[2][3] * s[0]) * idet;

    T[3][0] = (-M[1][0] * c[3] + M[1][1] * c[1] - M[1][2] * c[0]) * idet;
    T[3][1] = ( M[0][0] * c[3] - M[0][1] * c[1] + M[0][2] * c[0]) * idet;
    T[3][2] = (-M[3][0] * s[3] + M[3][1] * s[1] - M[3][2] * s[0]) * idet;
    T[3][3] = ( M[2][0] * s[3] - M[2][1] * s[1] + M[2][2] * s[0]) * idet;
}
static inline void mat4x4_orthonormalize(mat4x4 R, mat4x4 M)
{
    float s = 1.;
    vec3 h;

    mat4x4_dup(R, M);
    vec3_norm(R[2], R[2]);

    s = vec3_mul_inner(R[1], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[1], R[1], h);
    vec3_norm(R[2], R[2]);

    s = vec3_mul_inner(R[1], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[1], R[1], h);
    vec3_norm(R[1], R[1]);

    s = vec3_mul_inner(R[0], R[1]);
    vec3_scale(h, R[1], s);
    vec3_sub(R[0], R[0], h);
    vec3_norm(R[0], R[0]);
}

static inline void mat4x4_frustum(mat4x4 M, float l, float r, float b, float t, float n, float f)
{
    M[0][0] = 2.f*n/(r-l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f*n/(t-b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][0] = (r+l)/(r-l);
    M[2][1] = (t+b)/(t-b);
    M[2][2] = -(f+n)/(f-n);
    M[2][3] = -1.f;

    M[3][2] = -2.f*(f*n)/(f-n);
    M[3][0] = M[3][1] = M[3][3] = 0.f;
}
static inline void mat4x4_ortho(mat4x4 M, float l, float r, float b, float t, float n, float f)
{
    M[0][0] = 2.f/(r-l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f/(t-b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][2] = -2.f/(f-n);
    M[2][0] = M[2][1] = M[2][3] = 0.f;

    M[3][0] = -(r+l)/(r-l);
    M[3][1] = -(t+b)/(t-b);
    M[3][2] = -(f+n)/(f-n);
    M[3][3] = 1.f;
}
static inline void mat4x4_perspective(mat4x4 m, float y_fov, float aspect, float n, float f)
{
    /* NOTE: Degrees are an unhandy unit to work with.
     * linmath.h uses radians for everything! */
    float const a = 1.f / (float) tan(y_fov / 2.f);

    m[0][0] = a / aspect;
    m[0][1] = 0.f;
    m[0][2] = 0.f;
    m[0][3] = 0.f;

    m[1][0] = 0.f;
    m[1][1] = a;
    m[1][2] = 0.f;
    m[1][3] = 0.f;

    m[2][0] = 0.f;
    m[2][1] = 0.f;
    m[2][2] = -((f + n) / (f - n));
    m[2][3] = -1.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = -((2.f * f * n) / (f - n));
    m[3][3] = 0.f;
}
static inline void mat4x4_look_at(mat4x4 m, vec3 eye, vec3 center, vec3 up)
{
    /* Adapted from Android's OpenGL Matrix.java.                        */
    /* See the OpenGL GLUT documentation for gluLookAt for a description */
    /* of the algorithm. We implement it in a straightforward way:       */

    /* TODO: The negation of of can be spared by swapping the order of
     *       operands in the following cross products in the right way. */
    vec3 f;
    vec3 s;
    vec3 t;

    vec3_sub(f, center, eye);
    vec3_norm(f, f);

    vec3_mul_cross(s, f, up);
    vec3_norm(s, s);

    vec3_mul_cross(t, s, f);

    m[0][0] =  s[0];
    m[0][1] =  t[0];
    m[0][2] = -f[0];
    m[0][3] =   0.f;

    m[1][0] =  s[1];
    m[1][1] =  t[1];
    m[1][2] = -f[1];
    m[1][3] =   0.f;

    m[2][0] =  s[2];
    m[2][1] =  t[2];
    m[2][2] = -f[2];
    m[2][3] =   0.f;

    m[3][0] =  0.f;
    m[3][1] =  0.f;
    m[3][2] =  0.f;
    m[3][3] =  1.f;

    mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
}

typedef float quat[4];
static inline void quat_identity(quat q)
{
    q[0] = q[1] = q[2] = 0.f;
    q[3] = 1.f;
}
static inline void quat_add(quat r, quat a, quat b)
{
    int i;
    for(i=0; i<4; ++i)
        r[i] = a[i] + b[i];
}
static inline void quat_sub(quat r, quat a, quat b)
{
    int i;
    for(i=0; i<4; ++i)
        r[i] = a[i] - b[i];
}
static inline void quat_mul(quat r, quat p, quat q)
{
    vec3 w;
    vec3_mul_cross(r, p, q);
    vec3_scale(w, p, q[3]);
    vec3_add(r, r, w);
    vec3_scale(w, q, p[3]);
    vec3_add(r, r, w);
    r[3] = p[3]*q[3] - vec3_mul_inner(p, q);
}
static inline void quat_scale(quat r, quat v, float s)
{
    int i;
    for(i=0; i<4; ++i)
        r[i] = v[i] * s;
}
static inline float quat_inner_product(quat a, quat b)
{
    float p = 0.f;
    int i;
    for(i=0; i<4; ++i)
        p += b[i]*a[i];
    return p;
}
static inline void quat_conj(quat r, quat q)
{
    int i;
    for(i=0; i<3; ++i)
        r[i] = -q[i];
    r[3] = q[3];
}
static inline void quat_rotate(quat r, float angle, vec3 axis) {
    int i;
    vec3 v;
    vec3_scale(v, axis, sinf(angle / 2));
    for(i=0; i<3; ++i)
        r[i] = v[i];
    r[3] = cosf(angle / 2);
}
#define quat_norm vec4_norm
static inline void quat_mul_vec3(vec3 r, quat q, vec3 v)
{
/*
 * Method by Fabian 'ryg' Giessen (of Farbrausch)
t = 2 * cross(q.xyz, v)
v' = v + q.w * t + cross(q.xyz, t)
 */
    vec3 t = {q[0], q[1], q[2]};
    vec3 u = {q[0], q[1], q[2]};

    vec3_mul_cross(t, t, v);
    vec3_scale(t, t, 2);

    vec3_mul_cross(u, u, t);
    vec3_scale(t, t, q[3]);

    vec3_add(r, v, t);
    vec3_add(r, r, u);
}
static inline void mat4x4_from_quat(mat4x4 M, quat q)
{
    float a = q[3];
    float b = q[0];
    float c = q[1];
    float d = q[2];
    float a2 = a*a;
    float b2 = b*b;
    float c2 = c*c;
    float d2 = d*d;

    M[0][0] = a2 + b2 - c2 - d2;
    M[0][1] = 2.f*(b*c + a*d);
    M[0][2] = 2.f*(b*d - a*c);
    M[0][3] = 0.f;

    M[1][0] = 2*(b*c - a*d);
    M[1][1] = a2 - b2 + c2 - d2;
    M[1][2] = 2.f*(c*d + a*b);
    M[1][3] = 0.f;

    M[2][0] = 2.f*(b*d + a*c);
    M[2][1] = 2.f*(c*d - a*b);
    M[2][2] = a2 - b2 - c2 + d2;
    M[2][3] = 0.f;

    M[3][0] = M[3][1] = M[3][2] = 0.f;
    M[3][3] = 1.f;
}

static inline void mat4x4o_mul_quat(mat4x4 R, mat4x4 M, quat q)
{
/*  XXX: The way this is written only works for othogonal matrices. */
/* TODO: Take care of non-orthogonal case. */
    quat_mul_vec3(R[0], q, M[0]);
    quat_mul_vec3(R[1], q, M[1]);
    quat_mul_vec3(R[2], q, M[2]);

    R[3][0] = R[3][1] = R[3][2] = 0.f;
    R[3][3] = 1.f;
}
static inline void quat_from_mat4x4(quat q, mat4x4 M)
{
    float r=0.f;
    int i;

    int perm[] = { 0, 1, 2, 0, 1 };
    int *p = perm;

    for(i = 0; i<3; i++) {
        float m = M[i][i];
        if( m < r )
            continue;
        m = r;
        p = &perm[i];
    }

    r = (float) sqrt(1.f + M[p[0]][p[0]] - M[p[1]][p[1]] - M[p[2]][p[2]] );

    if(r < 1e-6) {
        q[0] = 1.f;
        q[1] = q[2] = q[3] = 0.f;
        return;
    }

    q[0] = r/2.f;
    q[1] = (M[p[0]][p[1]] - M[p[1]][p[0]])/(2.f*r);
    q[2] = (M[p[2]][p[0]] - M[p[0]][p[2]])/(2.f*r);
    q[3] = (M[p[2]][p[1]] - M[p[1]][p[2]])/(2.f*r);
}

#define SPHERES_SIZE 4;

typedef struct Scene {
    vec3 CameraPos;
    vec3 LightPos;
    int SpheresNumber;
    int SpheresIdx;
    __global float* Input;
} Scene;

typedef struct Ray {
    vec3 From;
    vec3 Dir;
} Ray;

typedef struct Color {
    float R;
    float G;
    float B;
} Color;


float IntersectSphere(Scene* scene, int sphereIdx, Ray ray, float* material, vec3 normal) {
    vec3 spherePos = {scene->Input[sphereIdx + 0], scene->Input[sphereIdx + 1], scene->Input[sphereIdx + 2]};
    float sphereRadius = scene->Input[sphereIdx + 3];

    vec3 k;
    vec3_sub(k, ray.From, spherePos);

    float b = vec3_mul_inner(k, ray.Dir);
    float kLen = vec3_len(k);
    float c = kLen * kLen - sphereRadius * sphereRadius;
    float d = b*b - c;

    if (d < 0) {
        return -1.0f;
    }

    float sqrtfd = sqrt(d);
    float t1 = -b + sqrtfd;
    float t2 = -b - sqrtfd;

    float min_t  = min(t1,t2);
    float max_t = max(t1,t2);

    float dist = (min_t >= 0) ? min_t : max_t;
    if (dist <= 0) {
        return -1.0f;
    }

    vec3 dirToPoint;
    vec3_scale(dirToPoint, ray.Dir, dist);

    vec3 point;
    vec3_add(point, ray.From, dirToPoint);

    vec3 normDir;
    vec3_sub(normDir, point, spherePos);

    vec3_norm(normal, normDir);

    return dist;
}

void Intersect(Scene* scene, Ray ray, float* distance, float* material, vec3 normal) {
    float bestDistance = -1.0f;
    vec3 bestNormal;
    bestNormal[0] = 0;
    bestNormal[1] = 0;
    bestNormal[2] = 0;
    vec3 currNormal;
    for (int i = 0; i < scene->SpheresNumber; ++i) {
        int sphereIdx = scene->SpheresIdx + i * SPHERES_SIZE;
        float currDist = IntersectSphere(scene, sphereIdx, ray, 0, currNormal);
        if (currDist <= 0.0f) {
            continue;
        }
        if (bestDistance < 0.0f || currDist < bestDistance) {
            bestDistance = currDist;
            vec3_set(bestNormal, currNormal);
        }
    }
    *distance = bestDistance;
    vec3_set(normal, bestNormal);
}

bool IntersectAnything(Scene* scene, Ray ray) {
    float distance;
    vec3 normal;
    Intersect(scene, ray, &distance, 0, normal);
    return distance > 0;
}

Color GetColor(Scene* scene, Ray ray, float distance, float* material, vec3 normal, int depth) {
    vec3 dirToCam;
    vec3_scale(dirToCam, ray.Dir, -1.0f);

    vec3 dirToPoint;
    vec3_scale(dirToPoint, ray.Dir, distance);
    vec3 point;
    vec3_add(point, ray.From, dirToPoint);

    Color color;

    color.R = 0;
    color.G = 0;
    color.B = 0;

    vec3 dirToLight;
    vec3_sub(dirToLight, scene->LightPos, point);

    vec3 dirToLightNorm;
    vec3_norm(dirToLightNorm, dirToLight);

    Ray rayToLight;
    vec3_scale(rayToLight.From, dirToLightNorm, 0.001f);
    vec3_add(rayToLight.From, point, rayToLight.From);
    vec3_set(rayToLight.Dir, dirToLightNorm);

    if (IntersectAnything(scene, rayToLight)) {
        return color;
    }

    if (depth > 0) {
        vec3 reflDir;
        vec3_reflect2(reflDir, dirToCam, normal);

        Ray reflRay;
        vec3_scale(reflRay.From, reflDir, 0.01f);
        vec3_add(reflRay.From, point,reflRay.From);
        vec3_set(ray.Dir, reflDir);

        int newDepth = depth-1;

//        Color reflectedColor = TraceColored2(scene, reflRay, newDepth);
//        color.R += reflectedColor.R * 0.1f;
//        color.R += reflectedColor.G * 0.1f;
//        color.R += reflectedColor.B * 0.1f;
    }

    //            if (depth > 0) {
//                if (material.AlbedoCF.Z > 0) {
//                    Vector3 reflDir = dirToCamera.Reflect(normal);
//                    Color reflectedColor = TraceColored(Ray(point + reflDir * 0.01f, reflDir), depth-1);
//                    color.R += reflectedColor.R * material.AlbedoCF.Z;
//                    color.G += reflectedColor.G * material.AlbedoCF.Z;
//                    color.B += reflectedColor.B * material.AlbedoCF.Z;
//                }
//
//                if (material.RefractCF.X > 0) {
//                    Vector3 refrDir = ray.Dir.Refract(normal, material.RefractCF.Y).Normalized();
//                    Color refrColor = TraceColored(Ray(point + refrDir * 0.001f, refrDir), depth-1);
//                    color.R += refrColor.R * material.RefractCF.X;
//                    color.G += refrColor.G * material.RefractCF.X;
//                    color.B += refrColor.B * material.RefractCF.X;
//                }
//            }


    float dp = 0;
    dp = vec3_mul_inner(dirToLightNorm, normal);

    float diffuseCF = 0.7f;
    if (dp > 0) {
        color.R += dp * diffuseCF * 0.6f;
        color.G += dp * diffuseCF * 0.6f;
        color.B += dp * diffuseCF * 1.0f;
    }

    vec3 refl;
    vec3_reflect2(refl, dirToLightNorm, normal);

    dp = vec3_mul_inner(refl, dirToCam);

    float lightPower = 0.9f;
    float albedoCF1 = 30.0f;
    float albedoCF2 = 1.2f;

    if (dp > 0) {
        dp = pow(dp, albedoCF1);
        color.R += dp * lightPower * albedoCF2;
        color.G += dp * lightPower * albedoCF2;
        color.B += dp * lightPower * albedoCF2;
    }

    return color;
}


Color TraceColored(Scene* scene, Ray ray, int depth) {
    float dist = -1.0f;
    vec3 normal = {0, 0, 0};
    Intersect(scene, ray, &dist, 0, normal);
    Color color;
    if (dist < 0) {
        color.R = 1.0f;
        color.G = 1.0f;
        color.B = 1.0f;
        return color;
    }
    return GetColor(scene, ray, dist, 0, normal, depth);
}





// -----------------------------------------------------------------------------------------------------

__kernel void processRaytrace(__global float* input, __global float* output, const unsigned int count) {
    float SCALE = 0.01;

    int i = get_global_id(0);

    int width = (int)input[0];
    int height = (int)input[1];

    int tmp = (int)input[0];

    Scene scene;
    scene.Input = input;
    scene.CameraPos[0] = input[2];
    scene.CameraPos[1] = input[3];
    scene.CameraPos[2] = input[4];

    scene.LightPos[0] = input[5];
    scene.LightPos[1] = input[6];
    scene.LightPos[2] = input[7];

    scene.SpheresNumber = (int)input[8];
    scene.SpheresIdx = 9;

    if (i >= width * height) {
        return;
    }

    int ci = i / height;
    int cj = i % height;

    int pos = (cj * width + ci) * 3;

    output[pos] = 1;
    output[pos + 1] = 1;
    output[pos + 2] = 1;

    vec3 dir = {(ci - 0.5f * width) * SCALE, (cj - 0.5f * height) * SCALE, 20.0f};

    vec3 dirNorm;
    vec3_norm(dirNorm, dir);

    Ray ray;
    vec3_set(ray.From, scene.CameraPos);
    vec3_set(ray.Dir, dirNorm);

    Color color = TraceColored(&scene, ray, 2);
    output[pos] = color.R;
    output[pos + 1] = color.G;
    output[pos + 2] = color.B;
}

