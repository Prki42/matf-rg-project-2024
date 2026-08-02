# Portal2Ep1

52/2022 - Aleksa Prtenjača

## Controls

```
W     -> Camera forwards
A     -> Camera left
S     -> Camera backwards
D     -> Camera right
Space -> Camera up
Shift -> Camera down

Q     -> Quit
F     -> Flashlight toggle
E     -> Spawn ominous temple

F2    -> Open GUI
```

## Features

### Fundamental:

- [x] Model with lighting
- [x] Two types of lighting with customizable colors and movement through GUI or ACTIONS
- [x] Press E --- 1 second --- Triggers ---> Spawn temple & change lights --- 3 seconds --- Triggers ---> Reset

### Group A:
- [ ] Frame-buffers with post-processing
- [ ] Off-screen Anti-Aliasing
- [ ] Parallax Mapping
- [x] Bloom with the use of HDR

### Group B:
- [ ] Deferred Shading
- [x] Point Shadows
- [ ] SSAO

### Other:
- [x] Normal mapping
- [x] Spot shadows

### Engine improvement:

- **Better glTF support**. Support up to two different UV mappings (TexCoords1/2). Save textures per mesh so different UV indices can be used with different meshes on the same texture. Texture type is also used in cache key generation so the same texture could be used for multiple purposes (eg. diffuse + emissive). Transforms in the scene graph are baked into the mesh (and applied adequately to (bi)tangents).
- **Textures/materials**. Provide shininess, emission and opacity to shader. Generate 1x1 fallback textures using specified color (eg. AI_MATKEY_COLOR_SPECULAR) or set some default. Convert height maps to normal maps on load time. Generate (bi)tangents from normals when not specified.
- **Nix file**. Not really engine improvement. Setup dev environment with `nix develop --impure` and run with `nixGL ./APP`.

## Models:

[Some Portal 2 models](https://www.deviantart.com/hazelcat3s/art/Portal-2-Model-Pack-for-Blender-992522894)
[Room model](https://sketchfab.com/3d-models/3december-2020-portal-test-chamber-65518f8e657f4194935d4dc67b82c278)
