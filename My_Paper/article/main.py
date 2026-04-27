from latextool_basic import *
p = Plot()
p += Graph.node(x=0, y=0, r=0.3, label='$a$', name='a')
p += Graph.node(x=2, y=0, r=0.3, label='$b$', name='b')
p += Graph.arc(names=['a', 'b'])
print(p)

