# Level 3 sketch (displayed 2000px wide) -> world; anchored on the main rod and core rod B, 4.81 cm/px, image down=+X, right=-Y
K=4.81
f=lambda p:(round(3337+(p[1]-254)*K), round(-2826-(p[0]-1284)*K))
P={'save':(525,643),'obs1a':(527,770),'obs1b':(622,690),'drum':(650,773),'obs2a':(568,883),'obs2b':(650,820),'stalker':(786,937),'doorA':(1048,1083),'doorB':(1035,1192),'access':(975,1080),'room_c':(760,980),'bridge':(1110,1110),'entry':(470,560),'rodB':(945,383)}
if __name__=='__main__':
    import json; print(json.dumps({k:f(v) for k,v in P.items()}))
