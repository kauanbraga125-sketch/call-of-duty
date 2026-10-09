// Textured CoD1 BSP59 map preview, native PS4 (experimental).
// Only executable code; the owner's extracted assets stay outside GitHub.
#ifndef COD1_MAP_DATA_H
#define COD1_MAP_DATA_H
static const unsigned M_MAX_TRI=7200, M_MAX_MAT=256, M_TS=64;
static const int M_W=480, M_H=270;
struct MapTri2 {float v[15];uint32_t mat;};
static MapTri2 m_tri[M_MAX_TRI];
static uint16_t m_tex[M_MAX_MAT*M_TS*M_TS];
static uint32_t m_frame[M_W*M_H];
static float m_zbuf[M_W*M_H];
static unsigned map_count=0,m_matcount=0;
static float m_feet=0,m_vy=0;
static bool m_grounded=false;
static void map3d_load(){
    FILE *f=fopen("/app0/preview.c4m","rb");
    if(!f){logmsg("MAP 01 no integrated C4M2 - test geometry fallback");return;}
    char magic[4];uint32_t n=0,mc=0,ts=0;float origin[4]={};
    if(fread(magic,1,4,f)!=4||memcmp(magic,"C4M2",4)!=0||
       fread(&n,4,1,f)!=1||fread(&mc,4,1,f)!=1||
       fread(&ts,4,1,f)!=1||fread(origin,4,4,f)!=4||
       !n||n>M_MAX_TRI||!mc||mc>M_MAX_MAT||ts!=M_TS){
       logmsg("MAP ERROR C4M2 header invalid");fclose(f);return;
    }
    if(fread(m_tri,sizeof(MapTri2),n,f)!=n||
       fread(m_tex,2,mc*M_TS*M_TS,f)!=(size_t)(mc*M_TS*M_TS)){
       logmsg("MAP ERROR truncated map or textures");fclose(f);return;
    }
    fclose(f);
    for(unsigned i=0;i<n;i++){
       if(m_tri[i].mat>=mc){logmsg("MAP ERROR bad material");return;}
       for(int k=0;k<15;k++)if(!isfinite(m_tri[i].v[k])||
          fabsf(m_tri[i].v[k])>100000.0f){logmsg("MAP ERROR bad vertex/uv");return;}
    }
    map_count=n;m_matcount=mc;m_feet=0;m_vy=0;m_grounded=false;
    cam.x=0;cam.z=0;cam.pitch=0;cam.yaw=origin[3]*0.01745329252f;
    char buf[120];snprintf(buf,sizeof(buf),"MAP loaded %u triangles, %u original materials",n,mc);
    logmsg(buf);
}
#endif
