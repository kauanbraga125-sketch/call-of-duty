// COD1 PS4 - independent native map preview 0.7 (geometry experiment).
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
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>

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
    logmsg("BOOT 0.7 ENTER main - log opened");
}
static void log_rc(const char *stage,int code) {
    char line[200];snprintf(line,sizeof(line),"%s result=%d (0x%08X)",stage,code,(unsigned)code);
    logmsg(line);
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
// Private map preview generated on the owner's PC. No commercial assets in the PKG.
// Binary layout: <4sIffff> (24 bytes), then count * <9fI> (40 bytes).
struct MapTri { float xyz[9]; uint32_t color; };
static const unsigned MAX_MAP_TRIS = 1200;
static MapTri map_tris[MAX_MAP_TRIS];
static unsigned map_count=0;
static float map_spawn_yaw=0.0f;
static void load_map_preview() {
    // Prefer the single, self-contained PKG map. No user FTP required.
    const char *path="/app0/preview.c4m";
    FILE *f=fopen(path,"rb");
    if(!f) {
        path="/data/codps4/maps/preview.c4m"; // optional development fallback
        f=fopen(path,"rb");
    }
    if(!f) {logmsg("MAP 01 no preview.c4m found in PKG or data folder");return;}
    char magic[4]={};unsigned count=0;float origin[4]={};
    if(fread(magic,1,4,f)!=4 || memcmp(magic,"C4M1",4)!=0 ||
       fread(&count,4,1,f)!=1 || fread(origin,4,4,f)!=4 ||
       count==0 || count>MAX_MAP_TRIS) {
        logmsg("MAP ERROR invalid format/header/count");fclose(f);return;
    }
    if(fread(map_tris,sizeof(MapTri),count,f)!=count) {
        logmsg("MAP ERROR incomplete geometry");fclose(f);return;
    }
    fclose(f);
    for(unsigned i=0;i<count;i++) {
        for(int j=0;j<9;j++) {
            float v=map_tris[i].xyz[j];
            if(v!=v || fabsf(v)>100000.0f) {
                logmsg("MAP ERROR nonfinite/invalid vertex");
                return;
            }
        }
    }
    map_count=count;
    map_spawn_yaw=origin[3]*3.14159265f/180.0f;
    cam.x=0.0f;cam.z=0.0f;cam.yaw=map_spawn_yaw;cam.pitch=0.0f;
    char line[160];
    snprintf(line,sizeof(line),"MAP 02 loaded %u triangles - original game geometry",map_count);
    logmsg(line);
}

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
    if(map_count) {
        // Simple software wireframe preview. Not a texture renderer or full map engine.
        for(unsigned i=0;i<map_count;i++) {
            const MapTri &t=map_tris[i];
            uint32_t c=rgb((t.color>>16)&255,(t.color>>8)&255,t.color&255);
            const float *v=t.xyz;
            edge(v[0],v[1],v[2],v[3],v[4],v[5],c);
            edge(v[3],v[4],v[5],v[6],v[7],v[8],c);
            edge(v[6],v[7],v[8],v[0],v[1],v[2],c);
        }
        rect(22,24,1340,110,rgb(13,24,26));
        text(46,40,"COD1 / MAPA ORIGINAL - GEOMETRIA",3,rgb(243,219,170));
        text(46,88,"SEM TEXTURAS, COLISOES OU MISSOES",2,rgb(229,190,139));
        rect(24,962,1640,90,rgb(13,24,26));
        text(45,982,"ANALOGICOS: CAMERA / BOLA: VOLTAR",2,rgb(210,225,213));
        return;
    }
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
    text(62,996,"BOLA: MENU     OPTIONS: MENU    SEM MISSOES NA V0.7",2,rgb(241,195,140));
}
static void menu(int selection) {
    background();
    rect(114,145,1690,790,rgb(17,27,30));
    rect(114,145,18,790,rgb(181,145,93));
    text(203,208,"COD 1  -  PS4",8,rgb(239,219,174));
    text(206,310,"PROTOTIPO NATIVO   V0.7",3,rgb(139,178,164));
    rect(210,420,850,450,rgb(24,43,45));
    const char *opts[3]={map_count?"ABRIR MAPA ORIGINAL":"INICIAR TESTE 3D","CONTROLES","SAIR"};
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
    logmsg("VIDEO 01 before sceVideoOutOpen");
    video=sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN, ORBIS_VIDEO_OUT_BUS_MAIN, 0, NULL);
    log_rc("VIDEO 02 sceVideoOutOpen",video);
    if(video<0) {logmsg("ERROR sceVideoOutOpen");return false;}
    logmsg("VIDEO 03 before sceKernelCreateEqueue");
    int rc=sceKernelCreateEqueue(&flip_queue,"cod1 flip queue");
    log_rc("VIDEO 04 sceKernelCreateEqueue",rc);
    if(rc<0) {logmsg("ERROR create flip queue");return false;}
    rc=sceVideoOutAddFlipEvent(flip_queue,video,NULL);
    log_rc("VIDEO 05 sceVideoOutAddFlipEvent",rc);
    const size_t align=0x200000;
    direct_size=(FB_BYTES*COUNT+align-1)/align*align;
    logmsg("VIDEO 06 before sceKernelAllocateDirectMemory");
    rc=sceKernelAllocateDirectMemory(0,sceKernelGetDirectMemorySize(),direct_size,align,3,&direct_offset);
    log_rc("VIDEO 07 sceKernelAllocateDirectMemory",rc);
    if(rc<0) {
        logmsg("ERROR allocate direct video memory");return false;
    }
    logmsg("VIDEO 08 before sceKernelMapDirectMemory");
    rc=sceKernelMapDirectMemory(&mapped,direct_size,0x33,0,direct_offset,align);
    log_rc("VIDEO 09 sceKernelMapDirectMemory",rc);
    if(rc<0) {
        logmsg("ERROR map video memory");return false;
    }
    for(int i=0;i<COUNT;i++) buffers[i]=(uint32_t *)((uint8_t*)mapped+i*FB_BYTES);
    OrbisVideoOutBufferAttribute attr;
    memset(&attr,0,sizeof(attr));
    logmsg("VIDEO 10 before sceVideoOutSetBufferAttribute");
    sceVideoOutSetBufferAttribute(&attr,0x80000000,1,0,W,H,W);
    logmsg("VIDEO 11 before sceVideoOutRegisterBuffers");
    rc=sceVideoOutRegisterBuffers(video,0,(void **)buffers,COUNT,&attr);
    log_rc("VIDEO 12 sceVideoOutRegisterBuffers",rc);
    if(rc<0) {
        logmsg("ERROR register video buffers");return false;
    }
    logmsg("VIDEO 13 before sceVideoOutSetFlipRate");
    rc=sceVideoOutSetFlipRate(video,0);
    log_rc("VIDEO 14 sceVideoOutSetFlipRate",rc);
    logmsg("VIDEO 15 VideoOut ready 1920x1080");
    return true;
}
static void flip(int frame) {
    if(frame==1) logmsg("FRAME 01 before sceVideoOutSubmitFlip");
    int rc=sceVideoOutSubmitFlip(video,active,ORBIS_VIDEO_OUT_FLIP_VSYNC,frame);
    if(frame==1) log_rc("FRAME 02 sceVideoOutSubmitFlip",rc);
    if(rc<0) {
        logmsg("ERROR submit frame");sceKernelUsleep(16000);return;
    }
    for (;;) {
        OrbisVideoOutFlipStatus status;
        memset(&status,0,sizeof(status));
        rc=sceVideoOutGetFlipStatus(video,&status);
        if(frame==1 && rc<0)log_rc("FRAME 03 sceVideoOutGetFlipStatus failed",rc);
        if(status.flipArg==frame) break;
        OrbisKernelEvent evt;int num=0;
        if(sceKernelWaitEqueue(flip_queue,&evt,1,&num,NULL)<0) break;
    }
    if(frame==1) logmsg("FRAME 04 completed first flip");
    active=(active+1)%COUNT;
}
static float axis_value(uint8_t v) {
    float x=((int)v-128)/127.0f;
    return fabsf(x)<0.14f?0:clampf(x,-1,1);
}
int main() {
    setvbuf(stdout,NULL,_IONBF,0);
    begin_log();
    load_map_preview();
    logmsg("BOOT 0.7 ENTER init_video");
    // Explicitly load the graphics module, in line with OpenOrbis's internal
    // module IDs; record the result even if the module is already loaded.
    logmsg("MODULE 01 before VIDEO_OUT load");
    log_rc("MODULE 02 VIDEO_OUT load", (int)sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_VIDEO_OUT));
    if(!init_video()) {
        logmsg("FATAL graphics initialization failed");
        for(int i=0;i<200;i++) sceKernelUsleep(100000);
        // Do not return through _exit on PS4: preserve crash diagnostics.
        for (;;) sceKernelUsleep(500000);
    }
    // Draw/present a menu BEFORE touching UserService and Pad. A visible menu,
    // followed by a crash, then implicates pad/userservice, not video startup.
    logmsg("STAGE 01 before first video frame (no controller)");
    menu(0);
    flip(1);
    logmsg("STAGE 02 first video frame finished");
    sceKernelUsleep(1500000);
    logmsg("STAGE 03 before UserService load");
    log_rc("MODULE 03 USER_SERVICE load", (int)sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_USER_SERVICE));
    logmsg("BOOT 0.7 video returned OK, before UserService");
    OrbisUserServiceInitializeParams usr;
    memset(&usr,0,sizeof(usr));
    usr.priority=ORBIS_KERNEL_PRIO_FIFO_LOWEST;
    int user_id=-1;
    int urc=sceUserServiceInitialize(&usr);
    log_rc("USER 01 sceUserServiceInitialize",urc);
    urc=sceUserServiceGetInitialUser(&user_id);
    log_rc("USER 02 sceUserServiceGetInitialUser",urc);
    log_rc("USER 03 user ID",user_id);
    if(user_id>=0) {
        logmsg("MODULE 04 before PAD module load");
        int module_rc=(int)sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_PAD);
        log_rc("MODULE 05 PAD module load",module_rc);
        // If module loading fails, do not call into an uninitialized pad lib.
        if(module_rc>=0) {
        logmsg("PAD 01 before scePadInit");
        int prc=scePadInit();
        log_rc("PAD 02 scePadInit",prc);
        if(prc==0) {
            pad=scePadOpen(user_id,0,0,NULL);
            log_rc("PAD 03 scePadOpen",pad);
        }
        } else logmsg("PAD skipped: sysmodule load failed");
    } else logmsg("PAD: no valid user, skipping pad init");
    logmsg("BOOT 0.7 entering event loop");
    if(pad<0) logmsg("WARNING controller not ready");
    int screen=0,selected=0,frame=2;
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
                if(selected==0) {screen=1;logmsg(map_count?"Original map preview opened":"Scene 3D opened");}
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
        if(frame==2) logmsg("FRAME 00 before drawing menu");
        if(screen==0) menu(selected);else if(screen==1)scene3d();else controls();
        if(frame==2) logmsg("FRAME 00 done drawing menu");
        if(pad<0)text(65,1050,"AVISO: CONTROLE NAO DETECTADO",2,rgb(255,149,90));
        flip(frame++);
    }
    // Avoid returning from main(): certain PS4 OpenOrbis runtimes report
    // CE-34878-0 on normal _exit. Request dashboard exit via SystemService.
    if(pad>=0) scePadClose(pad);
    logmsg("EXIT 01 request dashboard exit");
    sceSystemServiceLoadExec("exit",NULL);
    logmsg("EXIT 02 return from LoadExec (unexpected); idle safely");
    for (;;) sceKernelUsleep(500000);

}
