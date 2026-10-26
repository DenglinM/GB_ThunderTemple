import json
def t(n,a): return execute_tool(n,json.dumps(a))
S='editor_toolset.toolsets.scene.SceneTools.'
O='editor_toolset.toolsets.object.ObjectTools.'
A='editor_toolset.toolsets.actor.ActorTools.'
C=lambda a,c:{'refPath':a['refPath']+'.'+c}
def floor(x,y):
    d=t(S+'trace_world',{'start':{'x':x,'y':y,'z':2500},'end':{'x':x,'y':y,'z':-1000}}).get('returnValue')
    return None if d is None else round(2500-d)
def label(a,n,fo):
    t(A+'set_label',{'actor':a,'label':n}); t(S+'set_actor_folder',{'actor':a,'folder_path':'GrayBox/'+fo})
def run():
    for l in ['GB_CoreArea','GB_Checkpoint_Core']:
        for a in t(S+'find_actors',{'name':l,'tag':'','collision_channels':[]})['returnValue']: t(S+'remove_from_scene',{'actor':a})
    # the drawn box around the main rod: stepping in sets every core stalker on the player
    area=t(S+'add_to_scene_from_class',{'actor_type':{'refPath':'/Script/Engine.TriggerBox'},'name':'GB_CoreArea','xform':{'location':{'x':3232,'y':-2142,'z':400},'rotation':{'pitch':0,'yaw':-73.5,'roll':0},'scale':{'x':25,'y':28.3,'z':12.5}}})['returnValue']
    label(area,'GB_CoreArea','Core')
    n=[]
    for a in t(S+'find_actors',{'name':'GB_Stalker_0','tag':'','collision_channels':[]})['returnValue']:
        t(O+'set_properties',{'instance':a,'values':json.dumps({'bChasePlayer':True,'DetectionRadius':0.0,'ChaseSpeedMultiplier':1.2,'ActivationArea':area})})
        n.append(t(A+'get_label',{'actor':a})['returnValue'])
    # save point just past the core entrance trigger
    x,y=2980,-450
    z=floor(x,y)
    cp=t(S+'add_to_scene_from_class',{'actor_type':{'refPath':'/Script/GB_ThunderTemple.ThunderCheckpoint'},'name':'GB_Checkpoint_Core','xform':{'location':{'x':x,'y':y,'z':z},'rotation':{'pitch':0,'yaw':-90,'roll':0}}})['returnValue']
    label(cp,'GB_Checkpoint_Core','Core')
    ref=t(S+'find_actors',{'name':'GB_Checkpoint_L3','tag':'','collision_channels':[]})['returnValue'][0]
    mats=json.loads(t(O+'get_properties',{'instance':C(ref,'Pad'),'properties':['OverrideMaterials']})['returnValue'])['OverrideMaterials']
    t(O+'set_properties',{'instance':C(cp,'Pad'),'values':json.dumps({'OverrideMaterials':mats})})
    t('editor_toolset.toolsets.asset.AssetTools.save_assets',{'asset_paths':[]})
    return {'stalkers':n,'cpz':z,'mats':mats}
