/* Feature-owned native marker adapter, also exercised against an inert boundary. */
static struct {DRoot object;float saved,written;int created,seen;} groups[512];
static int group_count;
static DRoot compass_root;
static const char *categories[]={"Dust stations","Equipment","Other services","Players","Waypoints"};
static const float limits[]={0,100,250,500,1000};
static PTR alpha_get,alpha_set;
static PTR field_array(PTR obj,const char *name,const char *expected){
 PTR f=get_field(object_class(obj),name);if(!f)return 0;
 PTR (*ft)(PTR)=(PTR(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_field_get_type");if(!ft)return 0;
 char *type=da.type_name(ft(f));int match=type&&deq(type,expected);if(type)da.free(type);
 PTR result=0;if(match)field_value(obj,f,&result);return result;
}
static int release_group(int index){
 PTR group=dget(groups[index].object);int ok=1;
 if(malive_checked(group,&ok)){
  float current=dnumber(alpha_get,group,&ok);
  /* Retry a failed native read/write while retaining ownership. A failed
   * invocation is not evidence that the group was destroyed. */
  if(!ok&&!groups[index].created)return 0;
  if(ok&&current==groups[index].written){void *a[]={&groups[index].saved};call(alpha_set,group,a,&ok);if(!ok)return 0;}
  /* Another writer may have replaced our last value. Do not roll it back. */
  if(groups[index].created){ok=1;void *a[]={group};call(m.destroy,0,a,&ok);if(!ok)return 0;}
 }
 if(!ok)return 0;
 ddrop(&groups[index].object);groups[index]=groups[--group_count];return 1;
}
static void feature_end(void){
 for(int i=group_count-1;i>=0;i--)release_group(i);
 ddrop(&compass_root);
}
static void opacity(PTR component,int hidden){
 if(!component||!malive(component))return;int ok=1;
 PTR group=mc(component,"UnityEngine","CanvasGroup",&ok);if(!ok)return;
 int found=-1;for(int i=0;i<group_count;i++)if(dget(groups[i].object)==group){found=i;break;}
 if(!hidden){if(found>=0)release_group(found);return;}
 if(found<0){
  if(group_count>=512)return;
  int created=0;
  if(!group){PTR game=call(mm("UnityEngine","Component","get_gameObject",""),component,0,&ok);if(!ok||!game)return;group=add(game,"UnityEngine","CanvasGroup",&ok);created=1;}
  if(!ok||!group)return;
  float saved=dnumber(alpha_get,group,&ok);DRoot root=ok?dhold(group):0;
  if(!root){if(created){void *a[]={group};call(m.destroy,0,a,&ok);}return;}
  found=group_count++;groups[found].object=root;groups[found].saved=saved;groups[found].written=saved;groups[found].created=created;
 }else{
  float current=dnumber(alpha_get,group,&ok);if(!ok)return;
  if(current!=groups[found].written)groups[found].saved=current;
 }
 groups[found].seen=1;float zero=0;void *a[]={&zero};
 /* Native setters may mutate before raising. Retain the attempted value so
  * release can recognize and restore even a partially failed write. */
 groups[found].written=zero;call(alpha_set,group,a,&ok);
}
static int kinds[5],kinds_ready;
static int bind_kinds(void){
 PTR game=GetModuleHandleA("GameAssembly.dll");
 PTR (*nested)(PTR,PTR*)=(PTR(*)(PTR,PTR*))GetProcAddress(game,"il2cpp_class_get_nested_types");
 const char *(*name)(PTR)=(const char *(*)(PTR))GetProcAddress(game,"il2cpp_class_get_name");
 void (*stat)(PTR,void*)=(void(*)(PTR,void*))GetProcAddress(game,"il2cpp_field_static_get_value");
 PTR beacon=find_class("MoonCleaners","MoonServiceBeacon");if(!beacon||!nested||!name||!stat)return 0;
 PTR iter=0,c;while((c=nested(beacon,&iter)))if(deq(name(c),"ServiceKind")){
  const char *keys[]={"DustStation","Equipment","Other","Player","Waypoint"};
  for(int i=0;i<5;i++){PTR f=get_field(c,keys[i]);if(!f)return 0;stat(f,kinds+i);for(int j=0;j<i;j++)if(kinds[j]==kinds[i])return 0;}
  alpha_get=mm("UnityEngine","CanvasGroup","get_alpha","");alpha_set=mm("UnityEngine","CanvasGroup","set_alpha","System.Single");
  return alpha_get&&alpha_set;
 }return 0;
}
static void apply(PTR compass){
 if(compass!=dget(compass_root)){feature_end();if(malive(compass))compass_root=dhold(compass);}
 if(!malive(compass)||!compass_root)return;
 if(!kinds_ready){if(!bind_kinds())return;kinds_ready=1;}
 for(int i=0;i<group_count;i++)groups[i].seen=0;
 PTR icons[2]={field_array(compass,"icons","MoonCleaners.MoonCompassServiceIcon[]"),field_array(compass,"rearIcons","MoonCleaners.MoonCompassServiceIcon[]")};
 PTR distances=field_array(compass,"distances","System.Single[]");
 PTR ranges[2]={field_array(compass,"ranges","UnityEngine.UI.Text[]"),field_array(compass,"rearRanges","UnityEngine.UI.Text[]")};
 PTR pins[2]={field_array(compass,"pins","UnityEngine.RectTransform[]"),field_array(compass,"rearPins","UnityEngine.RectTransform[]")};
 unsigned range=m.known[8]<=4?m.known[8]:0;
 for(int side=0;side<2;side++){
  if(!icons[side])continue;U64 count=da.array_length(icons[side]);if(count>128)count=128;
  for(U64 i=0;i<count;i++){
   PTR icon=*(PTR*)darray(icons[side],i);if(!malive(icon))continue;
   int kind=-1;PTR k=get_field(object_class(icon),"kind");if(k)field_value(icon,k,&kind);
   int category=moon_map_category(kind,kinds,5);if(category<0)continue;
   int in_range=1;
   if(range){
    /* Missing, negative or non-finite distances must not masquerade as zero.
     * Unlimited bypasses range data, including native not-yet-read slots. */
    float distance=-1;if(distances&&i<da.array_length(distances))dcopy(&distance,(unsigned char*)distances+32+4*i,4);
    in_range=distance>=0&&moon_map_marker_visible(0,category,distance,limits[range]);
   }
   int visible=in_range&&moon_map_marker_visible(m.known[0],category,0,0);
   PTR pin=pins[side]&&i<da.array_length(pins[side])?*(PTR*)darray(pins[side],i):icon;
   opacity(pin,!visible);
   if(ranges[side]&&i<da.array_length(ranges[side]))opacity(*(PTR*)darray(ranges[side],i),!visible||m.known[9]);
  }
 }
 /* Release slots no longer present, changed to unknown kinds, or invalidated
  * by a type mismatch. We own opacity only while actively suppressing a slot. */
 for(int i=group_count-1;i>=0;i--)if(!groups[i].seen)release_group(i);
}
static char *key_label(char *out,unsigned key,const char *name,const char *defaults){
 char label[64];if(!moon_input_label(name,defaults,label,sizeof(label)))moon_input_vk_binding(key,label);return cat(out,label);
}
static void feature_tick(PTR walker,PTR world,int input){
 (void)world;int ok=1;PTR compass=mc(walker,"MoonCleaners","MoonServiceCompass",&ok);apply(ok?compass:0);
 if(!input||!mapopen(map())){m.shop=0;show("",0);return;}
 if(m.pressed['H'])m.shop=!m.shop;
 char text[1536],*p=text;
 if(!m.shop){p=cat(p,"HUD markers: ");p=key_label(p,'H',"hud-marker-controls.controls","H");cat(p," for type / distance controls");show(text,1);return;}
 int changed=0;for(int i=0;i<5;i++)if(m.pressed['1'+i]){m.known[0]^=(unsigned char)(1u<<i);changed=1;}
 if(m.pressed[0xbd]){m.known[8]=(unsigned char)((m.known[8]+4)%5);changed=1;}
 if(m.pressed[0xbb]){m.known[8]=(unsigned char)((m.known[8]+1)%5);changed=1;}
 if(m.pressed['D']){m.known[9]^=1;changed=1;}
 if(changed){m.dirty=1;save();mevent("hud-marker-settings");apply(compass);}
 p=cat(p,"HUD markers / ");p=key_label(p,'H',"hud-marker-controls.controls","H");p=cat(p," close / ");
 p=key_label(p,0xbd,"hud-marker-controls.range-less","Minus");p=cat(p," / ");p=key_label(p,0xbb,"hud-marker-controls.range-more","Equals");p=cat(p," range / ");p=key_label(p,'D',"hud-marker-controls.distances","D");p=cat(p," distances\n");
 const char *ids[]={"hud-marker-controls.type-1","hud-marker-controls.type-2","hud-marker-controls.type-3","hud-marker-controls.type-4","hud-marker-controls.type-5"};
 for(int i=0;i<5;i++){char defaults[2]={(char)('1'+i),0};p=key_label(p,'1'+i,ids[i],defaults);p=cat(p," ");p=cat(p,categories[i]);p=cat(p,m.known[0]&(1u<<i)?" OFF  ":" ON  ");if(i==1||i==3)p=cat(p,"\n");}
 p=cat(p,"\nRange ");if(m.known[8]==0)p=cat(p,"unlimited");else{p=num(p,(U64)limits[m.known[8]<=4?m.known[8]:0]);p=cat(p," m");}cat(p,m.known[9]?" / distances hidden":" / distances shown");show(text,1);
}
