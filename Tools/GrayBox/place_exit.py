import json
def t(n,a): return execute_tool(n,json.dumps(a))
S='editor_toolset.toolsets.scene.SceneTools.'
A='editor_toolset.toolsets.actor.ActorTools.'
O='editor_toolset.toolsets.object.ObjectTools.'
MI=lambda m:{'refPath':'/Game/ThunderTemple/Materials/%s.%s'%(m,m)}
def floor(x,y):
    d=t(S+'trace_world',{'start':{'x':x,'y':y,'z':4000},'end':{'x':x,'y':y,'z':-2000}}).get('returnValue')
    return None if d is None else round(4000-d)
def run():
    for a in t(S+'find_actors',{'name':'GB_Exit','tag':'','collision_channels':[]})['returnValue']: t(S+'remove_from_scene',{'actor':a})
    # on the blue stroke at the top-left of the core, door facing the main rod
    x,y=1155,-688
    z=floor(x,y)
    a=t(S+'add_to_scene_from_class',{'actor_type':{'refPath':'/Script/GB_ThunderTemple.ThunderExit'},'name':'GB_Exit','xform':{'location':{'x':x,'y':y,'z':z},'rotation':{'pitch':0,'yaw':-40,'roll':0}}})['returnValue']
    t(A+'set_label',{'actor':a,'label':'GB_Exit'}); t(S+'set_actor_folder',{'actor':a,'folder_path':'GrayBox/Finale'})
    t(O+'set_properties',{'instance':a,'values':json.dumps({'LockedMaterial':MI('MI_GB_BlueDark'),'UnlockedMaterial':MI('MI_GB_BlueBright')})})
    # corners of the booth must stand on the floor
    import math
    c,s=math.cos(math.radians(-40)),math.sin(math.radians(-40))
    corners={}
    for lx,ly in [(-240,-340),(-240,340),(240,-340),(240,340),(500,0)]:
        corners['%d,%d'%(lx,ly)]=floor(x+lx*c-ly*s,y+lx*s+ly*c)
    t('editor_toolset.toolsets.asset.AssetTools.save_assets',{'asset_paths':[]})
    return {'z':z,'corners':corners}
