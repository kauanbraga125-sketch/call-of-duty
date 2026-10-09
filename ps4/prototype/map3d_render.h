#ifndef COD1_MAP3D_RENDER_H
#define COD1_MAP3D_RENDER_H
static void map3d_render(){
    const uint32_t sky=rgb(87,116,146),floorc=rgb(79,79,68);
    for(int y=0;y<M_H;y++)for(int x=0;x<M_W;x++){
        int k=y*M_W+x;m_frame[k]=y<135?sky:floorc;m_zbuf[k]=0;
    }
    const float s=sinf(cam.yaw),c=cosf(cam.yaw);
    const float sp=sinf(cam.pitch),cp=cosf(cam.pitch);
    const float eye=m_feet+1.65f;
    const float near=0.18f;
    for(unsigned i=0;i<map_count;i++){
        const MapTri2&t=m_tri[i];MVert tri[3];float maxz=-1e10f;
        for(int j=0;j<3;j++){
            const float *v=&t.v[j*5];
            float dx=v[0]-cam.x,dy=v[1]-eye,dz=v[2]-cam.z;
            float px=dx*c-dz*s, pz=dx*s+dz*c;
            MVert q={px,dy*cp-pz*sp,dy*sp+pz*cp,v[3],v[4]};
            tri[j]=q;if(q.z>maxz)maxz=q.z;
        }
        if(maxz<near)continue;
        // Sutherland-Hodgman clip against z=near before rasterization.
        MVert clip[5];int n=0;
        for(int j=0;j<3;j++){
            const MVert &a=tri[j],&b=tri[(j+1)%3];
            bool ia=a.z>=near,ib=b.z>=near;
            if(ia)clip[n++]=a;
            if(ia!=ib && n<5)clip[n++]=m_lerp(a,b,(near-a.z)/(b.z-a.z));
        }
        if(n<3)continue;
        for(int j=1;j+1<n;j++)m_drawtri(clip[0],clip[j],clip[j+1],t.mat);
    }
    // Native VideoOut stays 1080p. Upscaling 480x270 is cheaper than rendering
    // 1920x1080 pixels with a software texture/depth engine.
    for(int y=0;y<M_H;y++){
        uint32_t *dst=buffers[active]+4*y*W;
        for(int x=0;x<M_W;x++){
            uint32_t color=m_frame[y*M_W+x];int p=x*4;
            dst[p]=dst[p+1]=dst[p+2]=dst[p+3]=color;
        }
        for(int k=1;k<4;k++)memcpy(dst+k*W,dst,W*4);
    }
}
#endif
