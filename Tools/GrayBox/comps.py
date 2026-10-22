import json,math
d=json.load(open('rhino_tris.json'))
def components(tris):
    key=lambda p:tuple(round(c/2) for c in p)
    parent={}
    def f(x):
        while parent.setdefault(x,x)!=x:
            parent[x]=parent[parent[x]]; x=parent[x]
        return x
    for t in tris:
        ks=[key(p) for p in t]
        for k in ks[1:]: parent[f(k)]=f(ks[0])
    g={}
    for t in tris: g.setdefault(f(key(t[0])),[]).append(t)
    return list(g.values())
def info(tris):
    P=[p for t in tris for p in t]
    up=0; angs=[]
    for a,b,c in tris:
        u=[b[k]-a[k] for k in range(3)]; w=[c[k]-a[k] for k in range(3)]
        n=[u[1]*w[2]-u[2]*w[1],u[2]*w[0]-u[0]*w[2],u[0]*w[1]-u[1]*w[0]]
        A=math.hypot(*n)/2
        if A==0: continue
        nz=abs(n[2])/(2*A)
        angs.append((math.degrees(math.acos(min(1,nz))),A))
    tot=sum(a for _,a in angs)
    steep=sum(a for g,a in angs if g>40)/tot*100 if tot else 0
    mx=max(g for g,_ in angs) if angs else 0
    r=lambda i:(round(min(p[i] for p in P)),round(max(p[i] for p in P)))
    return dict(n=len(tris),x=r(0),y=r(1),z=r(2),area=round(tot/1e4,1),steep_pct=round(steep,1),max_ang=round(mx))
cs=components(d['mesh_9'])
cs.sort(key=lambda c:-len(c))
for i,c in enumerate(cs[:30]):
    print(i,info(c))
print('total comps',len(cs))
json.dump([c for c in cs],open('mesh9_comps.json','w'))
