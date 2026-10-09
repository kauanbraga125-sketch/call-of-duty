// Software rasterizer for textured camera-space triangles.
#ifndef COD1_MAP3D_DRAW_H
#define COD1_MAP3D_DRAW_H
struct MVert {float x,y,z,u,v;};
static MVert m_lerp(const MVert&a,const MVert&b,float t){
 MVert o={a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,
          a.z+(b.z-a.z)*t,a.u+(b.u-a.u)*t,a.v+(b.v-a.v)*t};
 return o;
}
static void m_drawtri(const MVert&a,const MVert&b,const MVert&c,unsigned mat){
 float x0=240+240*a.x/a.z, y0=133-240*a.y/a.z;
 float x1=240+240*b.x/b.z, y1=133-240*b.y/b.z;
 float x2=240+240*c.x/c.z, y2=133-240*c.y/c.z;
 if(!isfinite(x0)||!isfinite(x1)||!isfinite(x2)||
    !isfinite(y0)||!isfinite(y1)||!isfinite(y2))return;
 float xlo=fminf(x0,fminf(x1,x2)), xhi=fmaxf(x0,fmaxf(x1,x2));
 float ylo=fminf(y0,fminf(y1,y2)), yhi=fmaxf(y0,fmaxf(y1,y2));
 if(xhi<0||xlo>=M_W||yhi<0||ylo>=M_H)return;
 int L=(int)fmaxf(0,floorf(xlo)),R=(int)fminf(M_W-1,ceilf(xhi));
 int T=(int)fmaxf(0,floorf(ylo)),B=(int)fminf(M_H-1,ceilf(yhi));
 float A=(y1-y2)*(x0-x2)+(x2-x1)*(y0-y2);
 if(fabsf(A)<0.0003f)return;
 float inv=1.0f/A;
 float iz0=1/a.z,iz1=1/b.z,iz2=1/c.z;
 float u0=a.u*iz0,u1=b.u*iz1,u2=c.u*iz2;
 float v0=a.v*iz0,v1=b.v*iz1,v2=c.v*iz2;
 const uint16_t*tex=m_tex+mat*M_TS*M_TS;
 for(int y=T;y<=B;y++)for(int x=L;x<=R;x++){
    float px=x+0.5f,py=y+0.5f;
    float w0=((y1-y2)*(px-x2)+(x2-x1)*(py-y2))*inv;
    float w1=((y2-y0)*(px-x2)+(x0-x2)*(py-y2))*inv;
    float w2=1-w0-w1;
    if(w0<-.0001f||w1<-.0001f||w2<-.0001f)continue;
    float z=w0*iz0+w1*iz1+w2*iz2;
    int k=y*M_W+x;
    if(z<=m_zbuf[k])continue;
    float u=(w0*u0+w1*u1+w2*u2)/z;
    float v=(w0*v0+w1*v1+w2*v2)/z;
    int iu=((int)floorf(u*M_TS))&(M_TS-1);
    int iv=((int)floorf(v*M_TS))&(M_TS-1);
    uint16_t t=tex[iv*M_TS+iu];
    int r=((t>>11)&31)*255/31, g=((t>>5)&63)*255/63, bl=(t&31)*255/31;
    m_frame[k]=rgb(r,g,bl);m_zbuf[k]=z;
 }
}
#endif
