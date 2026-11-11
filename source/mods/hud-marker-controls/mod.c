#define MOD_INPUT_ACTIONS {{"hud-marker-controls.controls","H",'H'}, {"hud-marker-controls.type-1","1",'1'}, {"hud-marker-controls.type-2","2",'2'}, {"hud-marker-controls.type-3","3",'3'}, {"hud-marker-controls.type-4","4",'4'}, {"hud-marker-controls.type-5","5",'5'}, {"hud-marker-controls.range-less","Minus",0xbd}, {"hud-marker-controls.range-more","Equals",0xbb}, {"hud-marker-controls.distances","D",'D'}}
#define FEATURE 24
#define MOD_ID "hud-marker-controls"
#define HUD_Y 550
#include "runtime.h"
static struct {DRoot object;float saved;} groups[256];static int group_count;
static const char *categories[]={"Dust stations","Equipment","Other services","Players","Waypoints"};
static const float limits[]={10000000,100,250,500,1000};
static PTR field_array(PTR obj,const char *name,const char *expected){
 PTR f=get_field(object_class(obj),name);if(!f)return 0;PTR (*ft)(PTR)=(PTR(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_field_get_type");if(!ft)return 0;char *type=da.type_name(ft(f));int match=type&&deq(type,expected);if(type)da.free(type);PTR result=0;if(match)field_value(obj,f,&result);return result;
}
static void opacity(PTR component,float alpha){
 if(!component||!malive(component))return;int ok=1;PTR game=call(mm("UnityEngine","Component","get_gameObject",""),component,0,&ok);if(!game)return;PTR group=mc(component,"UnityEngine","CanvasGroup",&ok);
 if(!group){group=add(game,"UnityEngine","CanvasGroup",&ok);if(!group)return;}
 int found=-1;for(int i=0;i<group_count;i++)if(dget(groups[i].object)==group){found=i;break;}
 if(found<0){if(group_count>=256)return;found=group_count++;groups[found].object=dhold(group);groups[found].saved=dnumber(mm("UnityEngine","CanvasGroup","get_alpha",""),group,&ok);}
 mset(mm("UnityEngine","CanvasGroup","set_alpha","System.Single"),group,&alpha);
}
static void feature_end(void){for(int i=0;i<group_count;i++){PTR o=dget(groups[i].object);if(malive(o))mset(mm("UnityEngine","CanvasGroup","set_alpha","System.Single"),o,&groups[i].saved);ddrop(&groups[i].object);}group_count=0;}
static int kinds[5],kinds_ready;
static int bind_kinds(void){
 PTR game=GetModuleHandleA("GameAssembly.dll");PTR (*nested)(PTR,PTR*)=(PTR(*)(PTR,PTR*))GetProcAddress(game,"il2cpp_class_get_nested_types");const char *(*name)(PTR)=(const char *(*)(PTR))GetProcAddress(game,"il2cpp_class_get_name");void (*stat)(PTR,void*)=(void(*)(PTR,void*))GetProcAddress(game,"il2cpp_field_static_get_value");if(!nested||!name||!stat)return 0;PTR iter=0,c;while((c=nested(find_class("MoonCleaners","MoonServiceBeacon"),&iter)))if(deq(name(c),"ServiceKind")){const char *keys[]={"DustStation","Equipment","Other","Player","Waypoint"};for(int i=0;i<5;i++){PTR f=get_field(c,keys[i]);if(!f)return 0;stat(f,kinds+i);}return 1;}return 0;
}
static void apply(PTR compass){
 if(!kinds_ready){if(!bind_kinds())return;kinds_ready=1;}
 PTR icons=field_array(compass,"icons","MoonCleaners.MoonCompassServiceIcon[]"),rear=field_array(compass,"rearIcons","MoonCleaners.MoonCompassServiceIcon[]");
 PTR distances=field_array(compass,"distances","System.Single[]");
 PTR ranges[2]={field_array(compass,"ranges","UnityEngine.UI.Text[]"),field_array(compass,"rearRanges","UnityEngine.UI.Text[]")};
 PTR pins=field_array(compass,"pins","UnityEngine.RectTransform[]"),rear_pins=field_array(compass,"rearPins","UnityEngine.RectTransform[]");
 /* CanvasGroup alpha survives native LateUpdate's SetActive/layout work. Each
  * original alpha is restored when the owning world changes. */
 for(int side=0;side<2;side++){
  PTR array=side?rear:icons,ps=side?rear_pins:pins;if(!array)continue;U64 count=da.array_length(array);if(count>128)count=128;
  for(U64 i=0;i<count;i++){PTR icon=*(PTR*)darray(array,i);if(!icon)continue;int kind=-1;PTR k=get_field(object_class(icon),"kind");if(k)field_value(icon,k,&kind);int category=moon_map_category(kind,kinds,5);if(category<0)continue;float d=0;if(distances&&i<da.array_length(distances))dcopy(&d,(unsigned char*)distances+32+4*i,4);int visible=moon_map_marker_visible(m.known[0],category,d,limits[m.known[8]<=4?m.known[8]:4]);PTR pin=ps&&i<da.array_length(ps)?*(PTR*)darray(ps,i):icon;opacity(pin,visible?1:0);if(ranges[side]&&i<da.array_length(ranges[side]))opacity(*(PTR*)darray(ranges[side],i),visible&&!m.known[9]?1:0);
  }
 }

}
static void feature_tick(PTR walker,PTR world,int input){
 (void)world;(void)input;int ok=1;PTR compass=mc(walker,"MoonCleaners","MoonServiceCompass",&ok);if(compass)apply(compass);
 if(!mapopen(map())){show("",0);return;}
 if(m.pressed['H'])m.shop=!m.shop;
 if(!m.shop){show("HUD markers: H for type / distance controls",1);return;}
 int changed=0;for(int i=0;i<5;i++)if(m.pressed['1'+i]){m.known[0]^=(unsigned char)(1u<<i);changed=1;}
 if(m.pressed[0xbd]){m.known[8]=(unsigned char)((m.known[8]+4)%5);changed=1;}
 if(m.pressed[0xbb]){m.known[8]=(unsigned char)((m.known[8]+1)%5);changed=1;}
 if(m.pressed['D']){m.known[9]^=1;changed=1;}
 if(changed){m.dirty=1;save();mevent("hud-marker-settings");}
 char text[512],*p=cat(text,"HUD markers / H close / +/- range / D distances\n");for(int i=0;i<5;i++){p=num(p,(U64)i+1);p=cat(p," ");p=cat(p,categories[i]);p=cat(p,m.known[0]&(1u<<i)?" OFF  ":" ON  ");if(i==1||i==3)p=cat(p,"\n");}p=cat(p,"\nRange ");if(m.known[8]==0)p=cat(p,"unlimited");else{p=num(p,(U64)limits[m.known[8]<=4?m.known[8]:4]);p=cat(p," m");}p=cat(p,m.known[9]?" / distances hidden":" / distances shown");show(text,1);
}
