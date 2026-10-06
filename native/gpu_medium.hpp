#pragma once
#if defined(__ANDROID__) || defined(MEDIUM_GL_HOST)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#ifdef __ANDROID__
#include <android/native_window_jni.h>
#endif
namespace av {
const char *mediumVertexShader=R"GLSL(#version 300 es
precision highp float;
out vec2 uv;
void main(){vec2 p=vec2(gl_VertexID==1?3.0:-1.0,gl_VertexID==2?3.0:-1.0);gl_Position=vec4(p,0,1);uv=vec2((p.x+1.0)*.5,1.0-(p.y+1.0)*.5);}
)GLSL";
const char *mediumFragmentShader=R"GLSL(#version 300 es
precision highp float;
in vec2 uv;out vec4 frag;
uniform sampler2D albedoMap,normalMap,uiMap;
uniform int mediumMode,lightCount,heavyEffects;
uniform vec3 sunDirection,ambientColor,sunColor;
uniform vec4 lights[4];
uniform float worldTime,focusAmount,cameraZoom;
uniform vec2 player;
vec3 albedo(vec2 q){return texture(albedoMap,q).bgr;}
void main(){
 // Logical-resolution postprocess output already has the correct RGB order.
 if(mediumMode==2){frag=texture(albedoMap,vec2(uv.x,1.0-uv.y));return;}
 vec2 sceneUV=mediumMode==0?uv:(player+(uv*vec2(640,360)-player)/cameraZoom)/vec2(640,360);
 vec3 base=albedo(sceneUV);
 if(mediumMode==0){frag=vec4(base,1);return;}
 vec4 normalData=texture(normalMap,sceneUV);
 vec3 normal=normalize(normalData.bgr*2.0-1.0);
 float channel=floor(normalData.a*255.0+.5);
 float surfaceHeight=mod(channel,128.0);
 vec2 point=sceneUV*vec2(640,360);
 float diffuse=max(0.0,dot(normal,normalize(sunDirection)));
 vec3 color=base*(ambientColor+sunColor*diffuse);
 for(int i=0;i<4;i++){
   if(i>=lightCount)break;
   vec2 delta=lights[i].xy-point;
   float falloff=max(0.0,1.0-length(delta)/lights[i].z);
   vec3 direction=normalize(vec3(delta,42.0));
   float lambert=max(.08,dot(normal,direction));
   float shadow=0.0;
   if(heavyEffects!=0&&i==0&&falloff>.01){
     for(int k=1;k<=6;k++){
       float t=float(k)/7.0;
       vec2 sampleUV=mix(point,lights[i].xy,t)/vec2(640,360);
       float blocker=mod(floor(texture(normalMap,sampleUV).a*255.0+.5),128.0);
       float rayHeight=mix(surfaceHeight,42.0,t)+7.0;
       shadow=max(shadow,smoothstep(rayHeight,rayHeight+12.0,blocker)*.72);
     }
   }
   color+=base*vec3(1.0,.64,.30)*falloff*falloff*lambert*lights[i].w*(1.0-shadow);
 }
 float emission=step(128.0,channel);
 color+=base*emission*.8;
 if(heavyEffects!=0){
 // Restrained bloom around emissive pixels; never applied to UI.
 vec3 bloom=vec3(0);
 for(int i=0;i<4;i++){
   vec2 d=vec2(i==0?2.0:i==1?-2.0:0.0,i==2?2.0:i==3?-2.0:0.0)/vec2(640,360);
   bloom+=albedo(sceneUV+d)*step(127.5,texture(normalMap,sceneUV+d).a*255.0);
 }
 color+=bloom*.09;
 if(base.g>base.r*1.08){
   float dapple=.975+.025*sin(point.x*.043+worldTime*.7)*sin(point.y*.05+worldTime*.4);
   color*=dapple;
 }
 float shafts=pow(max(0.0,sin((point.x+point.y*.55+sin(worldTime*.15)*12.0)*.036)),18.0);
 color+=vec3(1.0,.90,.70)*shafts*(1.0-smoothstep(0.0,320.0,point.y))*sunColor.r*.055;


 float edge=smoothstep(115.0,340.0,length(point-player));
 vec3 soft=(albedo(sceneUV+vec2(1.0/640.0,0))+albedo(sceneUV-vec2(1.0/640.0,0)))*.5;
 color=mix(color,color*(soft+.02)/(base+.02),edge*focusAmount*.12);
 }
 // Palette grading: warm highlights and slightly cool shadows, subtle only.
 float lum=dot(color,vec3(.299,.587,.114));
 color=mix(color,color*vec3(.94,1.01,1.07),clamp(.32-lum,0.0,.18));
 float vignette=1.0-.13*dot(uv-.5,uv-.5)*2.0;
 color*=vignette;
 if(heavyEffects!=0){
   float grain=fract(sin(dot(floor(point),vec2(12.9898,78.233))+floor(worldTime*8.0))*43758.5453)-.5;
   color+=grain/510.0;
 }
 vec4 ui=texture(uiMap,uv).bgra;
 frag=vec4(ui.rgb+clamp(color,0.0,1.0)*(1.0-ui.a),1);
}
)GLSL";
struct MediumGPU {
  EGLDisplay display=EGL_NO_DISPLAY;
  EGLContext context=EGL_NO_CONTEXT;
  EGLSurface surface=EGL_NO_SURFACE;
  GLuint program=0,textures[3]{},vao=0,logicalFbo=0,logicalColor=0;
  bool ready=false;
  std::string driver,error;
#ifdef __ANDROID__
  ANativeWindow *window=nullptr;
#endif
  GLuint shader(GLenum type,const char*source){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&source,nullptr);glCompileShader(s);
    GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[2048]{};glGetShaderInfoLog(s,sizeof(log),nullptr,log);error=log;glDeleteShader(s);return 0;}return s;
  }
  void destroy(){
    ready=false;
    auto p=graphicsController.device();p.shaderStartupPassed=false;p.mediumAssetsReady=false;graphicsController.setDevice(p);
    if(display!=EGL_NO_DISPLAY){
      if(context!=EGL_NO_CONTEXT&&surface!=EGL_NO_SURFACE&&eglMakeCurrent(display,surface,surface,context)){
        glDeleteTextures(3,textures);
        if(logicalColor)glDeleteTextures(1,&logicalColor);
        if(logicalFbo)glDeleteFramebuffers(1,&logicalFbo);
        if(program)glDeleteProgram(program);if(vao)glDeleteVertexArrays(1,&vao);
      }
      eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
      if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);
      if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);
      eglTerminate(display);
    }
    display=EGL_NO_DISPLAY;context=EGL_NO_CONTEXT;surface=EGL_NO_SURFACE;program=vao=logicalFbo=logicalColor=0;
    for(auto&t:textures)t=0;
#ifdef __ANDROID__
    if(window)ANativeWindow_release(window);window=nullptr;
#endif
  }
  bool create(void *native=nullptr,int bufferW=W,int bufferH=H){
    destroy();error.clear();
#ifdef __ANDROID__
    window=static_cast<ANativeWindow*>(native);
#endif
    display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display==EGL_NO_DISPLAY||!eglInitialize(display,nullptr,nullptr)){error="EGL display unavailable";destroy();return false;}
    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint configAttribs[]={EGL_SURFACE_TYPE,native?EGL_WINDOW_BIT:EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
    EGLConfig config;EGLint count=0;
    if(!eglChooseConfig(display,configAttribs,&config,1,&count)||count<1){error="No GLES3 configuration";destroy();return false;}
    EGLint ctx[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    context=eglCreateContext(display,config,EGL_NO_CONTEXT,ctx);
    if(native){
#ifdef __ANDROID__
      EGLint visual=0;eglGetConfigAttrib(display,config,EGL_NATIVE_VISUAL_ID,&visual);ANativeWindow_setBuffersGeometry(window,0,0,visual);
      surface=eglCreateWindowSurface(display,config,window,nullptr);
#endif
    }else{EGLint size[]={EGL_WIDTH,bufferW,EGL_HEIGHT,bufferH,EGL_NONE};surface=eglCreatePbufferSurface(display,config,size);}
    if(context==EGL_NO_CONTEXT||surface==EGL_NO_SURFACE||!eglMakeCurrent(display,surface,surface,context)){error="EGL context/surface failed";destroy();return false;}
    GLuint vs=shader(GL_VERTEX_SHADER,mediumVertexShader),fs=shader(GL_FRAGMENT_SHADER,mediumFragmentShader);
    if(!vs||!fs){if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);destroy();return false;}
    program=glCreateProgram();glAttachShader(program,vs);glAttachShader(program,fs);glLinkProgram(program);glDeleteShader(vs);glDeleteShader(fs);
    GLint linked=0;glGetProgramiv(program,GL_LINK_STATUS,&linked);
    if(!linked){char log[2048]{};glGetProgramInfoLog(program,sizeof(log),nullptr,log);error=log;destroy();return false;}
    glGenVertexArrays(1,&vao);glGenTextures(3,textures);
    for(auto tex:textures){glBindTexture(GL_TEXTURE_2D,tex);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,W,H,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);}
    // Shade 640x360, not millions of physical display pixels. Present the
    // completed pixel-art image separately with one nearest texture sample.
    glGenTextures(1,&logicalColor);glBindTexture(GL_TEXTURE_2D,logicalColor);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,W,H,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glGenFramebuffers(1,&logicalFbo);glBindFramebuffer(GL_FRAMEBUFFER,logicalFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,logicalColor,0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){error="Logical framebuffer incomplete";destroy();return false;}
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDisable(GL_DITHER);
    if(glGetError()!=GL_NO_ERROR){error="GPU allocation failed";destroy();return false;}
    GLint size=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&size);
    driver=reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    auto p=graphicsController.device();p.glesMajor=3;p.maxTextureSize=size;p.shaderStartupPassed=true;p.mediumAssetsReady=true;graphicsController.setDevice(p);
    eglSwapInterval(display,1);ready=true;return true;
  }
  GLint uniform(const char*n){return glGetUniformLocation(program,n);}
  bool draw(const C*frame,int screenW=W,int screenH=H,float ox=0,float oy=0,float scale=1,bool swap=true){
    if(!ready)return false;
    if(!eglMakeCurrent(display,surface,surface,context)){error="EGL make-current failed: "+std::to_string(eglGetError());return false;}
    bool medium=mediumEnabled()&&mediumWorldCaptured&&!g.overlay&&g.scene==PLAY;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(0,0,screenW,screenH);glClearColor(.025f,.038f,.047f,1);glClear(GL_COLOR_BUFFER_BIT);
    glViewport(int(ox),int(screenH-oy-H*scale),std::max(1,int(W*scale)),std::max(1,int(H*scale)));
    if(medium){glBindFramebuffer(GL_FRAMEBUFFER,logicalFbo);glViewport(0,0,W,H);}
    glUseProgram(program);glBindVertexArray(vao);
    const C*buffers[]={medium?mediumAlbedo.data():frame,mediumNormals.data(),mediumOverlay.data()};
    const char*names[]={"albedoMap","normalMap","uiMap"};
    for(int i=0;i<(medium?3:1);i++){glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_2D,textures[i]);glTexSubImage2D(GL_TEXTURE_2D,0,0,0,W,H,GL_RGBA,GL_UNSIGNED_BYTE,buffers[i]);glUniform1i(uniform(names[i]),i);}
    glUniform1i(uniform("mediumMode"),medium?1:0);
    if(medium){
      float hour=worldHour(),sun=std::max(0.f,std::sin((hour-6)/12*PI));
      float golden=std::max(0.f,1-sun*1.7f);
      glUniform3f(uniform("sunDirection"),std::sin((hour-12)/12*PI),-.6f,.65f+sun*.5f);
      glUniform3f(uniform("ambientColor"),.21f+sun*.39f,.27f+sun*.35f,.39f+sun*.18f);
      glUniform3f(uniform("sunColor"),sun*(.48f+golden*.19f),sun*(.47f-golden*.12f),sun*(.42f-golden*.20f));
      auto budget=graphicsController.features();
      glUniform1i(uniform("heavyEffects"),budget.bloom?1:0);
      vistaGatherLights();int count=std::min(budget.pointLights,int(vistaLights.size()));float lights[16]{};
      for(int i=0;i<count;i++){auto&l=vistaLights[i];lights[i*4]=sx(l.x);lights[i*4+1]=syAt(l.x,l.y)-14;lights[i*4+2]=l.r*1.25f;lights[i*4+3]=1.5f-sun*.85f+std::sin(g.time*11+i)*.08f;}
      glUniform1i(uniform("lightCount"),count);glUniform4fv(uniform("lights"),4,lights);
      glUniform1f(uniform("cameraZoom"),mediumCameraZoom);glUniform1f(uniform("worldTime"),g.time);glUniform1f(uniform("focusAmount"),vistaFocus?1:0);glUniform2f(uniform("player"),sx(g.px),syAt(g.px,g.py)-25);
    }
    glDrawArrays(GL_TRIANGLES,0,3);
    if(medium){
      glBindFramebuffer(GL_FRAMEBUFFER,0);
      glViewport(int(ox),int(screenH-oy-H*scale),std::max(1,int(W*scale)),std::max(1,int(H*scale)));
      glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,logicalColor);
      glUniform1i(uniform("albedoMap"),0);glUniform1i(uniform("mediumMode"),2);
      glDrawArrays(GL_TRIANGLES,0,3);
    }
    GLenum failure=glGetError();
    if(failure!=GL_NO_ERROR){error="GL draw failed: "+std::to_string(failure);return false;}
    if(swap&&!eglSwapBuffers(display,surface)){error="EGL swap failed: "+std::to_string(eglGetError());return false;}
    return true;
  }
} mediumGPU;
} // namespace av
#endif
