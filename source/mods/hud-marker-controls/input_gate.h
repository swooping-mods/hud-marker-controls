/* Reuse the native map framework's admission policy and inspected signatures.
 * The map owns UIBlocked itself; pause/dialog/other screens remain independent. */
#include "../map-control-framework/screen_gates.h"
static struct {PTR pause,pause_blocks,pause_modal,types[6],open[6];} hud_gate;
static int hud_map_input_bind(void){
 hud_gate.pause=mm("MoonCleaners","MoonWalker","get_PauseMenu","");
 hud_gate.pause_blocks=mm("MoonCleaners","MoonPauseMenu","get_BlocksGameplay","");
 hud_gate.pause_modal=mm("MoonCleaners","MoonPauseMenu","get_ModalOpen","");
 if(!hud_gate.pause||!hud_gate.pause_blocks||!hud_gate.pause_modal)return 0;
 const char *names[]={"MoonUpgradeShop","MoonGarageMenu","MoonRobotMenu","MoonRadioMenu","MoonDepotBuilder","MoonHowToPlay"};
 for(unsigned i=0;i<6;i++){hud_gate.types[i]=dtype("MoonCleaners",names[i]);hud_gate.open[i]=mm("MoonCleaners",names[i],"get_IsOpen","");if(!hud_gate.types[i]||!hud_gate.open[i])return 0;}
 return 1;
}
static int hud_map_input_allowed(PTR walker,int focused,int typing){
 int ok=1,open=mapopen(map()),paused=0,modal=0;unsigned screens=0;
 if(!open)return 0;
 PTR pause=call(hud_gate.pause,walker,0,&ok);
 if(malive_checked(pause,&ok)){paused=boolean(hud_gate.pause_blocks,pause,&ok);modal=boolean(hud_gate.pause_modal,pause,&ok);}
 for(unsigned i=0;i<6;i++){
  void *a[]={hud_gate.types[i]};PTR screen=call(m.component,walker,a,&ok);
  if(malive_checked(screen,&ok)&&boolean(hud_gate.open[i],screen,&ok))screens|=1u<<i;
 }
 return ok&&mmc_screen_input_allowed(open,focused,typing,0,paused,modal,screens);
}
