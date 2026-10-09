#ifndef M_MOVE_H
#define M_MOVE_H
static float ma(uint8_t v){float x=((int)v-128)/127.f;return fabsf(x)<.14f?0:x;}
static void map3d_update(const OrbisPadData&d,uint32_t p){
cam.yaw+=ma(d.rightStick.x)*.034f;
cam.pitch=fmaxf(-1.2f,fminf(1.2f,cam.pitch-ma(d.rightStick.y)*.024f));
float fw=-ma(d.leftStick.y),sd=ma(d.leftStick.x);
float speed=(d.buttons&ORBIS_PAD_BUTTON_L3)?.17f:.10f;
float dx=(sinf(cam.yaw)*fw+cosf(cam.yaw)*sd)*speed;
float dz=(cosf(cam.yaw)*fw-sinf(cam.yaw)*sd)*speed;
if(!map3d_wall(cam.x+dx,cam.z,m_feet))cam.x+=dx;
if(!map3d_wall(cam.x,cam.z+dz,m_feet))cam.z+=dz;
if(m_grounded&&(p&ORBIS_PAD_BUTTON_CROSS)){m_vy=.23f;m_grounded=false;}
m_vy-=.011f;m_feet+=m_vy;
float h;
if(map3d_floor(cam.x,cam.z,m_feet+.55f,&h)&&m_feet<=h&&m_vy<=0){
m_feet=h;m_vy=0;m_grounded=true;
}else m_grounded=false;
if(m_feet < -18.f){cam.x=cam.z=0;m_feet=m_vy=0;}
}
#endif
