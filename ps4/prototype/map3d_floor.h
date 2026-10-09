#ifndef COD1_MAP3D_FLOOR_H
#define COD1_MAP3D_FLOOR_H
// Ground probe against the owner's original triangle geometry.
static bool map3d_floor(float x,float z,float ceiling,float *height){
    float best=-99999;bool found=false;
    for(unsigned i=0;i<map_count;i++){
        const float *p=m_tri[i].v;
        float ax=p[0],ay=p[1],az=p[2],bx=p[5],by=p[6],bz=p[7],cx=p[10],cy=p[11],cz=p[12];
        if(x<fminf(ax,fminf(bx,cx))-.02f||x>fmaxf(ax,fmaxf(bx,cx))+.02f||
           z<fminf(az,fminf(bz,cz))-.02f||z>fmaxf(az,fmaxf(bz,cz))+.02f)continue;
        float det=(bz-cz)*(ax-cx)+(cx-bx)*(az-cz);
        if(fabsf(det)<.0000001f)continue;
        float a=((bz-cz)*(x-cx)+(cx-bx)*(z-cz))/det;
        float b=((cz-az)*(x-cx)+(ax-cx)*(z-cz))/det;
        float c=1-a-b;
        if(a<-.015f||b<-.015f||c<-.015f)continue;
        float h=a*ay+b*by+c*cy;
        if(h<=ceiling && h>best){best=h;found=true;}
    }
    if(found)*height=best;
    return found;
}
#endif
