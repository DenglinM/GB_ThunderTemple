import json,math
def t(n,a): return execute_tool(n,json.dumps(a))
S='editor_toolset.toolsets.scene.SceneTools.'
A='editor_toolset.toolsets.actor.ActorTools.'
O='editor_toolset.toolsets.object.ObjectTools.'
MI=lambda m:{'refPath':'/Game/ThunderTemple/Materials/%s.%s'%(m,m)}
SP=lambda o,v:t(O+'set_properties',{'instance':o,'values':json.dumps(v)})
def C(a,c): return {'refPath':a['refPath']+'.'+c}
def floor(x,y):
    d=t(S+'trace_world',{'start':{'x':x,'y':y,'z':4000},'end':{'x':x,'y':y,'z':-2000}}).get('returnValue')
    return None if d is None else round(4000-d)
def one(l):
    r=t(S+'find_actors',{'name':l,'tag':'','collision_channels':[]})['returnValue']
    return r[0] if r else None
def add_cls(cls,n,x,y,z,yaw=0,scale=None):
    xf={'location':{'x':x,'y':y,'z':z},'rotation':{'pitch':0,'yaw':yaw,'roll':0}}
    if scale: xf['scale']=scale
    a=t(S+'add_to_scene_from_class',{'actor_type':{'refPath':'/Script/'+cls},'name':n,'xform':xf})['returnValue']
    t(A+'set_label',{'actor':a,'label':n}); t(S+'set_actor_folder',{'actor':a,'folder_path':'GrayBox/Level3'})
    return a
def run():
    rep={}
    for l in ['GB_Checkpoint_L3','GB_Cover_L3','GB_Drum_L3','GB_Stalker_L3','GB_RodSub_L3','GB_ThunderGate_L3','GB_AccessPoint_L3','GB_Level3Area']:
        for a in t(S+'find_actors',{'name':l,'tag':'','collision_channels':[]})['returnValue']: t(S+'remove_from_scene',{'actor':a})
    ref=one('GB_RodSub_CoreA')
    snd=json.loads(t(O+'get_properties',{'instance':ref,'properties':['ThunderSounds']})['returnValue'])['ThunderSounds']
    samples=[{'Sound':s['sound'],'StartTime':s['startTime']} for s in snd]
    cp2=one('GB_Checkpoint_L2')
    padmat=json.loads(t(O+'get_properties',{'instance':C(cp2,'Pad'),'properties':['OverrideMaterials']})['returnValue'])['OverrideMaterials']
    Z=1100
    # save point at the room entrance, facing into the room
    cp=add_cls('GB_ThunderTemple.ThunderCheckpoint','GB_Checkpoint_L3',5208,825,floor(5208,825),yaw=-60)
    SP(C(cp,'Pad'),{'OverrideMaterials':[m if isinstance(m,dict) else {'refPath':m} for m in padmat]})
    # two waist-high covers between the entrance and the stalker
    for n,a,b in [('GB_Cover_L3_01',(5819,815),(5434,358)),('GB_Cover_L3_02',(6362,618),(6059,224))]:
        x,y=(a[0]+b[0])/2,(a[1]+b[1])/2; L=math.dist(a,b)/100; yaw=math.degrees(math.atan2(b[1]-a[1],b[0]-a[0]))
        c=t(S+'add_to_scene_from_asset',{'asset_path':'/Engine/BasicShapes/Cube','name':n,'xform':{'location':{'x':x,'y':y,'z':floor(x,y)+50},'rotation':{'pitch':0,'yaw':yaw,'roll':0},'scale':{'x':L,'y':0.8,'z':1.0}}})['returnValue']
        t(A+'set_label',{'actor':c,'label':n}); t(S+'set_actor_folder',{'actor':c,'folder_path':'GrayBox/Level3'})
        SP(C(c,'StaticMeshComponent0'),{'OverrideMaterials':[MI('MI_GB_Wall')]})
    # drum
    d=add_cls('GB_ThunderTemple.ThunderDrum','GB_Drum_L3',5833,224,floor(5833,224)+45)
    SP(d,{'BeatSound':{'refPath':'/Game/ThunderTemple/SE_Drum/S_Drum_Beat.S_Drum_Beat'}})
    SP(C(d,'Body'),{'OverrideMaterials':[MI('MI_GB_Yellow')]})
    # area the stalker hunts in
    area=add_cls('Engine.TriggerBox','GB_Level3Area',6500,-200,Z+300,scale={'x':56.25,'y':50,'z':9.4})
    # conductor rod carried by the stalker once plated
    rod=add_cls('GB_ThunderTemple.LightningRodSub','GB_RodSub_L3_Conductor',6622,-431,floor(6622,-431)+125)
    SP(rod,{'BoltMaterial':MI('M_GB_Lightning'),'ThunderSounds':samples,'ThunderDuration':2.0,'bTriggerOnApproach':False,'PoleHeight':200.0,'PoleRadius':6.0,'TriggerRadius':500.0,'StrikeRadius':150.0,'Cooldown':6.0})
    SP(C(rod,'Pole'),{'OverrideMaterials':[MI('MI_GB_Yellow')]})
    # stalker guarding the room
    st=add_cls('GB_ThunderTemple.Stalker','GB_Stalker_L3_01',6622,-431,floor(6622,-431)+127)
    SP(st,{'ActiveMaterial':MI('MI_GB_Enemy'),'PausedMaterial':MI('MI_GB_Wall'),'PlatedMaterial':MI('MI_GB_Yellow'),'bChasePlayer':True,'DetectionRadius':1800.0,'ChaseSpeedMultiplier':1.2,'PlatedChaseSpeedMultiplier':1.1,'ActivationArea':area,'ConductorRod':rod})
    SP(C(st,'Body'),{'OverrideMaterials':[MI('MI_GB_Enemy')]})
    # blue gate across the exit to the core bridge; only the access point opens it
    gx,gy=(7324+7849)/2,(-1691-1628)/2
    g=add_cls('GB_ThunderTemple.ThunderGate','GB_ThunderGate_L3',gx,gy,floor(gx,gy),yaw=math.degrees(math.atan2(63,525))-90)
    SP(g,{'DoorSize':{'X':40.0,'Y':600.0,'Z':350.0},'bOpenOnThunder':False,'bCallLightningWhenWaiting':False})
    SP(C(g,'Door'),{'OverrideMaterials':[MI('MI_GB_Blue')]})
    ap=add_cls('GB_ThunderTemple.ThunderAccessPoint','GB_AccessPoint_L3',7310,-1340,floor(7310,-1340)+15)
    SP(ap,{'Gate':g,'Radius':400.0,'OpenTime':2.0})
    SP(C(ap,'Plate'),{'OverrideMaterials':[MI('MI_GB_Blue')]})
    SP(C(ap,'RangeMarker'),{'OverrideMaterials':[MI('MI_GB_Blue')]})
    t('editor_toolset.toolsets.asset.AssetTools.save_assets',{'asset_paths':[]})
    for n,o in [('st',st),('gate',g),('ap',ap),('rod',rod)]:
        rep[n]=t(A+'get_actor_transform',{'actor':o})['returnValue']['location']
    rep['stprops']=t(O+'get_properties',{'instance':st,'properties':['ConductorRod','ActivationArea','PlatedMaterial']})['returnValue']
    rep['gfloor']=floor(gx,gy)
    return rep
