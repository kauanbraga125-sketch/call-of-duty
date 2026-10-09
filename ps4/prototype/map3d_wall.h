#ifndef COD1_MAP3D_WALL_H
#define COD1_MAP3D_WALL_H
// Approximate steep triangle contact; this is not CoD's original collision.
static bool map3d_wall(float x,float z,float feet){
    const float radius=.16f;
    for(unsigned i=0;i<map_count;i++){
        const float*p=m_tri[i].v;
        float ax=p[0],ay=p[1],az=p[2],bx=p[5],by=p[6],bz=p[7],cx=p[10],cy=p[11],cz=p[12];
        if(x<fminf(ax,fminf(bx,cx))-radius||x>fmaxf(ax,fmaxf(bx,cx))+radius||
           z<fminf(az,fminf(bz,cz))-radius||z>fmaxf(az,fmaxf(bz,cz))+radius)continue;
        if(fmaxf(ay,fmaxf(by,cy))<feet+.16f||fminf(ay,fminf(by,cy))>feet+1.42f)continue;
        float ux=bx-ax,uy=by-ay,uz=bz-az,vx=cx-ax,vy=cy-ay,vz=cz-az;
        float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;
        float mag=sqrtf(nx*nx+nz*nz);
        if(mag<.0001f||fabsf(ny)>mag*.85f)continue;
        float dist=fabsf((x-ax)*nx+(z-az)*nz+(feet+.8f-ay)*ny)/
              sqrtf(nx*nx+ny*ny+nz*nz);
        if(dist<radius)return true;
    }
    return false;
}
#endif
