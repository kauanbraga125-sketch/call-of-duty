// COD1 PS4 - independent native boot prototype 0.1.
// Clean-room homebrew; this is NOT the original Call of Duty engine or campaign.
// VideoOut software framebuffer adapted from documented OpenOrbis graphics sample.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>
#include <orbis/Pad.h>
#include <orbis/UserService.h>

static const int W = 1920, H = 1080, COUNT = 2;
static const size_t FB_BYTES = 1920u * 1080u * 4u;
static uint32_t *buffers[COUNT] = { nullptr, nullptr };
static void *mapped = nullptr;
static off_t direct_offset = 0;
static size_t direct_size = 0;
static int video = -1, active = 0, pad = -1;
static OrbisKernelEqueue flip_queue;
static FILE *logfile = nullptr;

static void logmsg(const char *s) {
    if (logfile) { fprintf(logfile, "%s\n", s); fflush(logfile); }
    printf("[CODPS4] %s\n", s);
}
static void begin_log() {
    mkdir("/data/codps4", 0777);
    logfile = fopen("/data/codps4/boot.log", "a");
    logmsg("Boot prototype 0.1 starting");
}
static uint32_t rgb(int r,int g,int b) {
    return 0x80000000u | ((uint32_t)(r & 255) << 16) | ((uint32_t)(g & 255) << 8) | (uint32_t)(b & 255);
}
static void rect(int x,int y,int w,int h,uint32_t color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x+w > W) w = W-x;
    if (y+h > H) h = H-y;
    if (w <= 0 || h <= 0) return;
    uint32_t *p = buffers[active];
    for (int j=y;j<y+h;j++) {
        uint32_t *dst = p + j*W + x;
        for (int i=0;i<w;i++) dst[i] = color;
    }
}
static void line(int x0,int y0,int x1,int y1,uint32_t color) {
    if ((x0 < -3000 && x1 < -3000) || (x0 > W+3000 && x1 > W+3000) ||
        (y0 < -3000 && y1 < -3000) || (y0 > H+3000 && y1 > H+3000)) return;
    // Guard against huge projected off-screen line segments.
    x0 = x0 < -4000 ? -4000 : x0 > W+4000 ? W+4000 : x0;
    x1 = x1 < -4000 ? -4000 : x1 > W+4000 ? W+4000 : x1;
    y0 = y0 < -4000 ? -4000 : y0 > H+4000 ? H+4000 : y0;
    y1 = y1 < -4000 ? -4000 : y1 > H+4000 ? H+4000 : y1;
    int dx=abs(x1-x0),dy=abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1;
    int err=dx-dy;
    for (;;) {
        if (x0>=0 && x0<W && y0>=0 && y0<H) buffers[active][y0*W+x0]=color;
        if (x0==x1 && y0==y1) break;
        int e=err*2;
        if (e>-dy) {err-=dy;x0+=sx;}
        if (e<dx) {err+=dx;y0+=sy;}
    }
}
// 5x7 capital glyphs. No external fonts or copyrighted game artwork needed.
struct Glyph { char code; uint8_t rows[7]; };
static const Glyph glyphs[] = {
{'A',{14,17,17,31,17,17,17}},{'B',{30,17,17,30,17,17,30}},
{'C',{14,17,16,16,16,17,14}},{'D',{30,17,17,17,17,17,30}},
{'E',{31,16,16,30,16,16,31}},{'F',{31,16,16,30,16,16,16}},
{'G',{14,17,16,23,17,17,15}},{'H',{17,17,17,31,17,17,17}},
{'I',{14,4,4,4,4,4,14}},{'J',{7,2,2,2,18,18,12}},
{'K',{17,18,20,24,20,18,17}},{'L',{16,16,16,16,16,16,31}},
{'M',{17,27,21,21,17,17,17}},{'N',{17,25,21,19,17,17,17}},
{'O',{14,17,17,17,17,17,14}},{'P',{30,17,17,30,16,16,16}},
{'Q',{14,17,17,17,21,18,13}},{'R',{30,17,17,30,20,18,17}},
{'S',{15,16,16,14,1,1,30}},{'T',{31,4,4,4,4,4,4}},
{'U',{17,17,17,17,17,17,14}},{'V',{17,17,17,17,17,10,4}},
{'W',{17,17,17,21,21,21,10}},{'X',{17,17,10,4,10,17,17}},
{'Y',{17,17,10,4,4,4,4}},{'Z',{31,1,2,4,8,16,31}},
{'0',{14,17,19,21,25,17,14}},{'1',{4,12,4,4,4,4,14}},
{'2',{14,17,1,2,4,8,31}},{'3',{30,1,1,14,1,1,30}},
{'4',{2,6,10,18,31,2,2}},{'5',{31,16,16,30,1,1,30}},
{'6',{14,16,16,30,17,17,14}},{'7',{31,1,2,4,8,8,8}},
{'8',{14,17,17,14,17,17,14}},{'9',{14,17,17,15,1,1,14}},
{'.',{0,0,0,0,0,12,12}},{'-',{0,0,0,31,0,0,0}},
{':',{0,12,12,0,12,12,0}},{'/',{1,2,2,4,8,8,16}},
{'!',{4,4,4,4,4,0,4}},{'+',{0,4,4,31,4,4,0}},
{'?',{14,17,1,2,4,0,4}},{'(',{2,4,8,8,8,4,2}},
{')',{8,4,2,2,2,4,8}}
};
static void text(int x,int y,const char *s,int scale,uint32_t col) {
    int start=x;
    for (const char *t=s; *t; ++t) {
        if (*t=='\n') { y+=9*scale; x=start; continue; }
        char c = *t>='a' && *t<='z' ? *t-32 : *t;
        if (c==' ') {x+=6*scale;continue;}
        const Glyph *g=nullptr;
        for (unsigned i=0;i<sizeof(glyphs)/sizeof(glyphs[0]);i++) {
            if (glyphs[i].code==c) {g=&glyphs[i];break;}
        }
        if (g) {
            for (int j=0;j<7;j++) for(int k=0;k<5;k++)
                if (g->rows[j] & (1 << (4-k))) rect(x+k*scale,y+j*scale,scale,scale,col);
        }
        x+=6*scale;
    }
}
struct Camera { float x,z,yaw,pitch; };
static Camera cam={0,0,0,0};
static float clampf(float x,float lo,float hi) {return x<lo?lo:x>hi?hi:x;}
static bool project(float x,float y,float z,int &sx,int &sy) {
    float dx=x-cam.x,dz=z-cam.z,cy=cosf(cam.yaw),sn=sinf(cam.yaw);
    float px=dx*cy-dz*sn,pz=dx*sn+dz*cy;
    float dy=y-1.7f,cp=cosf(cam.pitch),sp=sinf(cam.pitch);
    float py=dy*cp-pz*sp,deep=dy*sp+pz*cp;
    if (deep < .28f) return false;
    sx=(int)(960.0f+960.0f*px/deep);
    sy=(int)(530.0f-960.0f*py/deep);
    return true;
}
static void edge(float ax,float ay,float az,float bx,float by,float bz,uint32_t col) {
    int x0,y0,x1,y1;
    if (project(ax,ay,az,x0,y0)&&project(bx,by,bz,x1,y1)) line(x0,y0,x1,y1,col);
}
static void cube(float x,float y,float z,float width,float height,float depth,uint32_t col) {
    float x0=x-width*.5f,x1=x+width*.5f,y0=y,y1=y+height;
    float z0=z-depth*.5f,z1=z+depth*.5f;
    edge(x0,y0,z0,x1,y0,z0,col);edge(x1,y0,z0,x1,y0,z1,col);
    edge(x1,y0,z1,x0,y0,z1,col);edge(x0,y0,z1,x0,y0,z0,col);
    edge(x0,y1,z0,x1,y1,z0,col);edge(x1,y1,z0,x1,y1,z1,col);
    edge(x1,y1,z1,x0,y1,z1,col);edge(x0,y1,z1,x0,y1,z0,col);
    edge(x0,y0,z0,x0,y1,z0,col);edge(x1,y0,z0,x1,y1,z0,col);
    edge(x1,y0,z1,x1,y1,z1,col);edge(x0,y0,z1,x0,y1,z1,col);
}
static void background() {
    // Low-cost GPU-free color bands so the first boot works without Piglet shaders.
    for(int y=0;y<H;y+=12) {
        if(y<535) {int t=y/14;rect(0,y,W,12,rgb(16+t/2,27+t/2,39+t/2));}
        else {int t=(y-535)/21;rect(0,y,W,12,rgb(28+t/3,36+t/2,32+t/3));}
    }
    rect(0,527,W,4,rgb(118,138,135));
}
static void scene3d() {
    background();
    // Navigation grid in world-space; positive Z is forward.
    uint32_t dim=rgb(64,98,94),bright=rgb(132,172,154);
    for(int x=-60;x<=60;x+=3)edge((float)x,0,-30,(float)x,0,100,dim);
    for(int z=-30;z<=100;z+=3)edge(-60,0,(float)z,60,0,(float)z,dim);
    cube(-8,0,20,10,9,10,bright);
    cube(9,0,25,10,14,9,bright);
    cube(0,0,42,13,5,15,bright);
    cube(-5,0,11,2,2,2,rgb(200,169,106));
    cube(5,0,15,2,2,2,rgb(200,169,106));
    cube(-17,0,35,3,8,3,rgb(211,143,107));
    // Center crosshair and HUD.
    line(938,540,951,540,rgb(225,223,205));
    line(969,540,982,540,rgb(225,223,205));
    line(960,518,960,532,rgb(225,223,205));
    line(960,548,960,562,rgb(225,223,205));
    rect(34,28,1170,68,rgb(13,24,26));
    text(60,42,"COD1 PS4  /  TESTE DE CAMERA",3,rgb(241,229,198));
    rect(35,941,1540,97,rgb(12,23,24));
    text(62,960,"ANALOGICO ESQUERDO: MOVER   DIREITO: OLHAR",2,rgb(198,218,212));
    text(62,996,"BOLA: MENU     OPTIONS: MENU    SEM MISSOES NA V0.1",2,rgb(241,195,140));
}
static void menu(int selection) {
    background();
    rect(114,145,1690,790,rgb(17,27,30));
    rect(114,145,18,790,rgb(181,145,93));
    text(203,208,"COD 1  -  PS4",8,rgb(239,219,174));
    text(206,310,"PROTOTIPO NATIVO   V0.1",3,rgb(139,178,164));
    rect(210,420,850,450,rgb(24,43,45));
    const char *opts[3]={"INICIAR TESTE 3D","CONTROLES","SAIR"};
    for(int i=0;i<3;i++) {
        int y=476+i*110;
        rect(252,y-15,735,80,i==selection?rgb(133,106,67):rgb(37,61,61));
        text(282,y,opts[i],4,i==selection?rgb(255,247,212):rgb(183,206,196));
    }
    rect(1135,419,576,451,rgb(28,44,47));
    text(1190,466,"SINGLE PLAYER",4,rgb(240,212,156));
    text(1190,547,"META PRINCIPAL",3,rgb(193,204,196));
    text(1190,616,"AINDA NAO",4,rgb(242,163,116));
    text(1190,683,"IMPLEMENTADO",4,rgb(242,163,116));
    text(212,912,"X: SELECIONAR      DIRECIONAL: NAVEGAR",2,rgb(174,191,180));
}
static void controls() {
    background();
    text(145,136,"CONTROLES",7,rgb(238,223,183));
    text(155,345,"X: CONFIRMAR",4,rgb(225,225,214));
    text(155,435,"BOLA: VOLTAR",4,rgb(225,225,214));
    text(155,525,"DIRECIONAL: MENU",4,rgb(225,225,214));
    text(155,615,"ANALOGICO ESQUERDO: MOVER",4,rgb(225,225,214));
    text(155,705,"ANALOGICO DIREITO: OLHAR",4,rgb(225,225,214));
    text(155,843,"OPTIONS: MENU",3,rgb(217,177,126));
}
static bool init_video() {
    video=sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN, ORBIS_VIDEO_OUT_BUS_MAIN, 0, NULL);
    if(video<0) {logmsg("ERROR sceVideoOutOpen");return false;}
    if(sceKernelCreateEqueue(&flip_queue,"cod1 flip queue")<0) {logmsg("ERROR create flip queue");return false;}
    sceVideoOutAddFlipEvent(flip_queue,video,NULL);
    const size_t align=0x200000;
    direct_size=(FB_BYTES*COUNT+align-1)/align*align;
    if(sceKernelAllocateDirectMemory(0,sceKernelGetDirectMemorySize(),direct_size,align,3,&direct_offset)<0) {
        logmsg("ERROR allocate direct video memory");return false;
    }
    if(sceKernelMapDirectMemory(&mapped,direct_size,0x33,0,direct_offset,align)<0) {
        logmsg("ERROR map video memory");return false;
    }
    for(int i=0;i<COUNT;i++) buffers[i]=(uint32_t *)((uint8_t*)mapped+i*FB_BYTES);
    OrbisVideoOutBufferAttribute attr;
    memset(&attr,0,sizeof(attr));
    sceVideoOutSetBufferAttribute(&attr,0x80000000,1,0,W,H,W);
    if(sceVideoOutRegisterBuffers(video,0,(void **)buffers,COUNT,&attr)<0) {
        logmsg("ERROR register video buffers");return false;
    }
    sceVideoOutSetFlipRate(video,0);
    logmsg("VideoOut ready 1920x1080");
    return true;
}
static void flip(int frame) {
    if(sceVideoOutSubmitFlip(video,active,ORBIS_VIDEO_OUT_FLIP_VSYNC,frame)<0) {
        logmsg("ERROR submit frame");sceKernelUsleep(16000);return;
    }
    for (;;) {
        OrbisVideoOutFlipStatus status;
        memset(&status,0,sizeof(status));
        sceVideoOutGetFlipStatus(video,&status);
        if(status.flipArg==frame) break;
        OrbisKernelEvent evt;int num=0;
        if(sceKernelWaitEqueue(flip_queue,&evt,1,&num,NULL)<0) break;
    }
    active=(active+1)%COUNT;
}
static float axis_value(uint8_t v) {
    float x=((int)v-128)/127.0f;
    return fabsf(x)<0.14f?0:clampf(x,-1,1);
}
int main() {
    setvbuf(stdout,NULL,_IONBF,0);
    begin_log();
    if(!init_video()) {
        logmsg("FATAL graphics initialization failed");
        for(int i=0;i<200;i++) sceKernelUsleep(100000);
        return 1;
    }
    OrbisUserServiceInitializeParams usr;
    memset(&usr,0,sizeof(usr));
    usr.priority=ORBIS_KERNEL_PRIO_FIFO_LOWEST;
    int user_id=-1;
    sceUserServiceInitialize(&usr);
    sceUserServiceGetInitialUser(&user_id);
    if(scePadInit()==0) pad=scePadOpen(user_id,0,0,NULL);
    if(pad<0) logmsg("WARNING controller not ready");
    int screen=0,selected=0,frame=1;
    uint32_t prev=0;
    for (;;) {
        OrbisPadData d;memset(&d,0,sizeof(d));
        if(pad>=0 && scePadReadState(pad,&d)<0) memset(&d,0,sizeof(d));
        uint32_t keys=d.buttons,pressed=keys & ~prev;prev=keys;
        const uint32_t back=ORBIS_PAD_BUTTON_CIRCLE | ORBIS_PAD_BUTTON_OPTIONS;
        if(screen==0) {
            if(pressed & ORBIS_PAD_BUTTON_UP) selected=(selected+2)%3;
            if(pressed & ORBIS_PAD_BUTTON_DOWN) selected=(selected+1)%3;
            if(pressed & ORBIS_PAD_BUTTON_CROSS) {
                if(selected==0) {screen=1;logmsg("Scene 3D opened");}
                else if(selected==1) screen=2;
                else {logmsg("User exited");break;}
            }
        } else if(screen==1) {
            if(pressed & back) {screen=0;logmsg("Return to menu");}
            else {
                float fwd=-axis_value(d.leftStick.y),side=axis_value(d.leftStick.x);
                float turn=axis_value(d.rightStick.x),look=-axis_value(d.rightStick.y);
                cam.yaw+=turn*.032f;
                cam.pitch=clampf(cam.pitch+look*.017f,-.65f,.65f);
                cam.x+=(sinf(cam.yaw)*fwd+cosf(cam.yaw)*side)*.16f;
                cam.z+=(cosf(cam.yaw)*fwd-sinf(cam.yaw)*side)*.16f;
                cam.x=clampf(cam.x,-55,55);cam.z=clampf(cam.z,-25,80);
            }
        } else if (screen==2 && (pressed & (back | ORBIS_PAD_BUTTON_CROSS))) screen=0;
        if(screen==0) menu(selected);else if(screen==1)scene3d();else controls();
        if(pad<0)text(65,1050,"AVISO: CONTROLE NAO DETECTADO",2,rgb(255,149,90));
        flip(frame++);
    }
    if(pad>=0) scePadClose(pad);
    if(logfile)fclose(logfile);
    return 0;
}
