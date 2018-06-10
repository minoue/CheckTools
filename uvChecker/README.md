## UVChecker
Find general uv errors

### Check numbers
0. Udim border intersections
1. Non-mapped UV faces
2. Zero-area Uv faces

### Flags
| Longname | Shortname | Argument types | Default | Properties |
|:---------|----------:|:--------------:|:-------:|:----------:|
|check|c|integer||C|
|uvArea|uva|double|0.000001|C|
|verbose|v|bool|False|C|

### Example
```python
from maya import cmds
```

## FindUvOverlaps 
Find overlapped UVs with other shells or itself.

### Flags
| Longname | Shortname | Argument types | Default | Properties |
|:---------|----------:|:--------------:|:-------:|:----------:|
|uvSet|set|string||C|
|verbose|v|bool|False|C|

### Example
```python
from maya import cmds
r = cmds.findUvOverlaps("|pSphere1")
print r
[u'|pPlane1|pPlaneShape1.map[38]', u'|pPlane1|pPlaneShape1.map[39]', ....]
```

* Single object selected or object path specified as command argment

    <img src="https://github.com/minoue/CheckTools/blob/media/media/uvOverlaps_single.gif" alt="Image" style="width: 300px;"/>

* Multiple object selected

    <img src="https://github.com/minoue/CheckTools/blob/media/media/uvOverlaps_multipleObj.gif" alt="Image" style="width: 300px;"/>
