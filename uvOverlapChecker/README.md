## FindUvOverlaps
Find overlapped UVs.

### Flags
| Longname | Shortname | Argument types | Default | Properties |
|:---------|----------:|:--------------:|:-------:|:----------:|
|verbose|v|bool|False|C|

### Example

```python
from maya import cmds
r = cmds.findUvOverlaps("|pSphere1")
print r
>>> [u'|pPlane1|pPlaneShape1.map[38]', u'|pPlane1|pPlaneShape1.map[39]', ....]
```

For multiple object check, select multiple objects and just run the command without path argument.

```python
cmds.findUvOverlaps()
```

* Single object selected or object path specified as command argment

    <img src="https://github.com/minoue/CheckTools/blob/media/media/uvOverlaps_single.gif" alt="Image" style="width: 300px;"/>

* Multiple object selected

    <img src="https://github.com/minoue/CheckTools/blob/media/media/uvOverlaps_multipleObj.gif" alt="Image" style="width: 300px;"/>
