import json
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
def add_cls(cls,n,x,y,z):
    a=t(S+'add_to_scene_from_class',{'actor_type':{'refPath':'/Script/'+cls},'name':n,'xform':{'location':{'x':x,'y':y,'z':z}}})['returnValue']
    t(A+'set_label',{'actor':a,'label':n}); t(S+'set_actor_folder',{'actor':a,'folder_path':'GrayBox/Finale'})
    return a
def run():
    for l in ['GB_Battery','GB_Finale']:
        for a in t(S+'find_actors',{'name':l,'tag':'','collision_channels':[]})['returnValue']: t(S+'remove_from_scene',{'actor':a})
    main=t(S+'find_actors',{'name':'GB_RodMain','tag':'','collision_channels':[]})['returnValue'][0]
    m=t(A+'get_actor_transform',{'actor':main})['returnValue']['location']
    snd=json.loads(t(O+'get_properties',{'instance':one('GB_RodSub_CoreA'),'properties':['ThunderSounds']})['returnValue'])['ThunderSounds']
    fin=add_cls('GB_ThunderTemple.ThunderFinale','GB_Finale',m['x'],m['y'],m['z'])
    SP(fin,{'MainRod':main,'WaveMaterial':MI('M_GB_SoundWave'),'WaveThunder':{'Sound':snd[0]['sound'],'StartTime':snd[0]['startTime']}})
    bx,by=m['x']+63,m['y']-261
    z=floor(bx,by)
    b=add_cls('GB_ThunderTemple.BatteryPickup','GB_Battery',bx,by,z+50)
    SP(b,{'Finale':fin})
    SP(C(b,'Body'),{'OverrideMaterials':[MI('MI_GB_Yellow')]})
    t('editor_toolset.toolsets.asset.AssetTools.save_assets',{'asset_paths':[]})
    return {'main':m,'floor':z,'snd':snd[0]}
