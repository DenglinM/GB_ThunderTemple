import json
def t(n,a): return execute_tool(n,json.dumps(a))
S='editor_toolset.toolsets.scene.SceneTools.'
O='editor_toolset.toolsets.object.ObjectTools.'
A='editor_toolset.toolsets.actor.ActorTools.'
MI=lambda m:{'refPath':'/Game/ThunderTemple/Materials/%s.%s'%(m,m)}
SP=lambda o,v:t(O+'set_properties',{'instance':o,'values':json.dumps(v)})
C=lambda a,c:{'refPath':a['refPath']+'.'+c}
def find(n): return t(S+'find_actors',{'name':n,'tag':'','collision_channels':[]})['returnValue']
def run():
    rep={}
    for a in find('GB_Drum'): SP(C(a,'Body'),{'OverrideMaterials':[MI('MI_GB_Yellow_Glow')]}); rep.setdefault('drum',0); rep['drum']+=1
    for a in find('GB_Battery'): SP(C(a,'Body'),{'OverrideMaterials':[MI('MI_GB_Yellow_Glow')]}); rep['battery']=1
    for a in find('GB_BrassPlate'): SP(C(a,'Plate'),{'OverrideMaterials':[MI('MI_GB_Yellow_Glow')]}); rep['plate']=1
    n=0
    for a in find('GB_Stalker'):
        SP(a,{'ActiveMaterial':MI('MI_GB_Enemy_Glow'),'PlatedMaterial':MI('MI_GB_Yellow_Glow')})
        SP(C(a,'Body'),{'OverrideMaterials':[MI('MI_GB_Enemy_Glow')]}); n+=1
    rep['stalkers']=n
    for a in find('GB_RodSub_L3_Conductor'): SP(C(a,'Pole'),{'OverrideMaterials':[MI('MI_GB_Yellow_Glow')]})
    g=0
    for a in find('GB_ThunderGate'): SP(C(a,'Door'),{'OverrideMaterials':[MI('MI_GB_Blue_EdgeGlow')]}); g+=1
    rep['gates']=g
    for a in find('GB_AccessPoint'): SP(C(a,'Plate'),{'OverrideMaterials':[MI('MI_GB_Blue_EdgeGlow')]})
    for a in find('GB_Exit'): SP(a,{'UnlockedMaterial':MI('MI_GB_BlueBright_Glow')}); rep['exit']=1
    mi=t('editor_toolset.toolsets.asset.AssetTools.find_assets',{'search':'MI_GB_Enemy_Glow'}) if False else None
    t('editor_toolset.toolsets.asset.AssetTools.save_assets',{'asset_paths':[]})
    return rep
