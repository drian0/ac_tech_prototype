# AssaultCube Tech Prototype #

## Purpose ##

<img src="/doc/screenshot_ac_complex_800.png" width="50%" alt="Tech Prototype: ac_complex"><img src="/doc/screenshot_ac_complex2_800.png" width="50%" alt="Tech Prototype: ac_complex">

Development of the first-person-shooter game AssaultCube ("AC") began in 2004 and was first released in 2006. It is based on the [Cube (1) Engine](http://cubeengine.com/). One of the current challenges is the fact that AssaultCube can no longer live up to the expectations of todays players in terms of visual appearance. It may not be possible to make AssaultCube look like a 2021 AAA game and compete with commercial games but it is realistic to catch up a bit.

It is safe to say that all the potential of the Cube Engine has been fully exploitet. It is not possible to make the game look better with this engine. The artists have done everything possible to use the available tools to the max and they achieved outstanding results considering the technological limits. 

The natural step forward is to renew the technology stack from the ground up and to provide new tools to the artists so that they can make existing content look much better plus can
produce fresh, modern content for the game.

The Cube Engine has meanwhile been succeeded by [Cube 2: Sauerbraten Engine](http://sauerbraten.org/) which in turn has been succeeded by [Tesseract Engine](http://tesseract.gg/). The basic assumption is that the artists and programmers in the AssaultCube community like the philosophy of the Cube engine(s) and want to move forward with that approach thus want to explore the capabilities of the Tesseract Engine.

The purpose of this tech prototype is to explore how the technology of AssaultCube could be renewed by using the Tesseract Engine.

## Disclaimer ##

This prototype has been put together by a programmer and not by an artists and so the game assest (maps, models, textures) are not of sufficiently good quality to showcase the full potential of the engine. For example, textures are currently of poor quality because the normalmaps and specularmaps are cheaply autogenerated and because the textures are generally of low resolution. The final result will look better once artists improve or replace the assets.

## Contents ##

1. [Quick Tour](#quick-tour)
2. [Changes](#changes)
3. [Findings](#findings)
4. [Asset Migration Path](#asset-migration-path)

## Quick Tour ##

Follow the steps of this quick tour to get an impression of the technology.

- Start assaultcube.bat / assaultcube.sh
- Navigate to "Options" 
  - Click the "Resolution" tab and make sure the proper resolution is set
  - Click the "Display" tab, tick "Fullscreen", set Anisotropic Filtering and Multisample AA to "16x" and set Morphological AA/Temporal AA/FXAA to "ultra"
  - Click the "Graphics" tab and set all options to "high"
  - In case you get a poor frame rate then lower some of these options.
- Navigate to "Map Browser" and click the map "ac_complex"
- Walk around and look at the red bricks (parallax mapping)
- Walk inside the buildings and notice the illusion of depths at walls and floors (normal mapping) 
- Look for metallice surfaces and note the reflections (specular mapping)
- Walk to the center square to see a few static player models 
- Press T and type `/thirdperson 2` to see an animated player model in action
- Press T and type `/addbot` to spawn a bot, then shoot the bot to see ragdoll physics in action
- Press E to enter edit mode and drag some lights around to see the dynamic lighting
- Select a few cubes from a wall surface then press G and scroll to change selection gridsize
- Release G and use the scrollwheel to push/pull a few cubes out/in the wall
- Try to build a simple building with three floors
- Press F2 and F3 to apply some textures and place some mapmodels into the scene
- Run a server instance with server.bat / server.sh
- Play with a friend on your server and figure out where the physics/movement feel like AC v1 and where it deviates

These steps should give you a first impression. If you would like to gain a deeper understanding please consult the next chapters.

## Changes ##

This tech prototype contains the following changes compared to vanilla Tesseract:

- 1 map from AC v1
- 49 textures from AC v1 with cheaply generated normalmaps/specmaps
- 5 hudgun models from AC v1 - the sniper rifle has a cheaply generated normalmap/specmap and the other guns have none
- 1 skeleton-animated player model with ragdoll physics support (some say they look more like Seaman than terrorists)
- 3 test map models (static player models)
- 1 game mode (insta i.e. OSOK with crosshair)
- hudgun sway and hudgun position from AC v1
- movement speed, crouch speed, straferunning from AC v1 (not perfect yet)
- disabled colouring of player models
- a little bit of AC theming for the UI
- import command for AC v1 maps 

## Findings ##

### Feasibility ###

Taking into account a) the capabilities of the new technology b) the capabilities of the development team and c) support of the AC community it is safe to say that renewing the technology stack of AssaultCube with Tesseract is feasible. 

### Methodology  ###

<img src="/doc/movingforward.jpg" width="50%">

**First Phase**
The first phase should be about publishing a game (*minimum playable product*) with a very, very limited scope. This ensures that it get's done and that it creates a sense of achievement which is important for momentum. The scope could include 1 game mode, 1 weapon, 1 player model and 1 map. For example: instagib (OSOK with crosshair), sniper rifle, seaman-terrorist-guy and ac_complex. This first release should not be a "demo" or a "prototype" but instead it sould be a full release that is meant to be played by people. It is a game of its own and deserves its own name such as "AssaultCube: Sniper Wars", "AC: Sniper Battle" or similar. There should be a group of at least one developer, one artist and one tester than can focus solely on this first product without distraction. Maintenance of AC v1.x and AC mobile should be shouldered by other members of the development team for this duration.

The basic assumption about the AC community is that it is more conservative than progressive. This assumption is based on the thought that the more progressive a person is the more likely it is that this person moves on to a newer game. Therefore the remaining people in the AC community may tend toward the conservative/traditional end. In terms of methodology the first phase might serve as a test to see if the community is willing to join the effort to renew AC and embrace the outcome. There is no point in building AssaultCube II if the first phase fails in this regard.

**Second Phase**
The knowledge gathered during this process will then be used to conceptualize and implement AssaultCube II in the second phase. What AssaultCube II actually constitutes needs to be figured out as part of the first phase.

### Balancing Innovation and Tradition ###

Renewing the tech stack of a game poses the inherent risk of a community split. Some will keep playing the old version while others move on to the new version. If too many people stick to the old version the endeavour fails. Therefore it is important to 'worship tradition'. This is especially crucial in the first releases or first months/years. The other risk is that if tradition is worshipped too much then there won't be sufficient innovation to attract new people - putting the whole mission at risk. There needs to be a plan in which areas innovation is allowed an in which areas tradition is ensured. For example the longterm idea could be to retain movement/physics/weapon strength the way it was in AC v1 while replacing all assets to achieve a similar visual realism as *Call of Duty: Warzone*. Sidenote: It may not be realistic to actually achieve this visual realism but it is worth aiming at it. 

Gameplay logic could be partially retained: Keeping the same weapon characteristics as in AC v1 but tackling the problem of [everybody using subgun](https://github.com/assaultcube/AC/issues/295). The problem could be resolved via two opposing directions: 
- Scarcity of primary weapons: Players start with pistol only and need to find primary weapons on the map. The weapons are shown on the minimap. Dead players drop their primary weapon and it can be picked up by others. Players can voluntarily drop their primary weapon for tactical reasons.
- Abundance of primary weapons: Players carry *all* primary weapons at the same time. The primary ammo pickup will equip all weapons. Primary ammo pickup only adds a fraction of the ammo compared to AC v1 so that players are forced to switch weapons often and use ammo more strategically.

### Hardware Support ###

AC v1 was marketed with the promise to run well on old computers. Most of the computers that were considered "old" back in 2004 do not exist anymore today. If we look at old computers by today's standards then Tessearct will support those when gfx settings are reduced.

## Asset Migration Path ##

### General ###

This chapter describes how assets from AC v1 could be migrated - or replaced. The description is differentiated into guidelines (what to do) and procedure (how to do).

### Map Migration ###

<img src="/doc/editmode_800.png" width="45%"> <img src="/doc/volumetric_lighting.png" width="45%"> 

#### Guidelines ####

Minimal:
- The lighting, specularity and normalmapping features of the engine **should be used wisely**. We should not create technology driven content. AC should retain its theme and should not get a metallic, overbright, overspecced look. The reason why things currently look that way is because it is a simple prototype. Please note that even the very first AC v1 maps (ac_complex and ac_desert) used zero to no colored lights probably for exactly that reason - technology should serve art and not vice versa.
- All used textures should have at least a normalmap/specmap.

Optimal:
- Remove all old textures and replace them by high-res textures.
- Add a lot of geometric details to the maps.


#### Import ####
- The tech prototype supports AssaultCube map format 10 from v1.3. Older formats are not supported.
- Place your map in the folder *\media\map\legacyformat*
  - To get started you should try to import ac_complex since its textures do already have some sort of normalmaps/specmaps.
- Start the tech prototype and type `/importassaultcube <yourmap> <newmapsize>`
- The map will be migrated and saved to */media/map/yourmap_imported.ogz* and */media/map/yourmap_imported.cfg*
- To prevent accidential overrides by succint imports you should save it with a new name. Do not forget to manually copy the .cfg as well.

#### Geometry ####
- After import, type E to enter edit mode, see also [basic editing](http://sauerbraten.org/docs/editing.html) and [editing binds](https://github.com/drian0/ac_tech_prototype/blob/master/config/default.cfg#L94)
- You will notice that certain textures on corner cubes may be broken - you you will need to fix this manually
- You will notice that certain heightfields that span more than one cube may be misaligned - you will need to fix this manually
- Tesseract allows for fine-grained cubes and so you should add more fine gemoetric details to the map. This is especially important once new high-res textures are imported. It looks strange to see high-res textures on old clunky geometry. No more long monotonous flat walls/floors.

#### Lighting ####
- After improving geometry, review the lights in the map
- Tesseract comes with a dynamic lighting system whereas AssaultCube v1 used a 2D lighting system. This means the imported lights do not make much sense - delete them with the command `/clearents light` 
- The imported map will have ceiling cubes with a skybox texture - remove these cubes so that the sunlight reaches into the map
- Place new [lights](http://sauerbraten.org/docs/editref.html#_light_) and [spotlights](http://sauerbraten.org/docs/editref.html#_spotlight_) in the map
- Tweak the sunlight with the commands [sunlight](http://sauerbraten.org/docs/editref.html#sunlight), [sunlightyaw](http://sauerbraten.org/docs/editref.html#sunlightyaw), [sunlightpitch](http://sauerbraten.org/docs/editref.html#sunlightpitch) and [sunlightscale](http://sauerbraten.org/docs/editref.html#sunlightscale)
- Tweak the [ambient](http://sauerbraten.org/docs/editref.html#ambient) lighting and [skylight](sauerbraten.org/docs/editref.html#skylight)
- Tweak diffuse global illumination with giscale, gidist, giaoscale as documented [here](http://tesseract.gg/README)
- Hint: Please note that some commands of the [Cube2:Sauerbraten editing reference](http://sauerbraten.org/docs/editref.html) have been advanced by Tesseract, check out the [Tesseract README](http://tesseract.gg/README) and [Tesseract Rending Pipeline Documentation](http://tesseract.gg/renderer.txt)

#### Textures ####
- After improving the lighting review the textures in the map
- You will notice that textures that are not power-of-two might have a wrong offset
  - Example: After importing ac_complex check out the 3x3 wooden boxes on the streets with misfit textures (texture *makke/box_3.jpg*)
  - This can be fixed my changing the texture offset on the given surface/cube in editmode by pressing O or P and scrolling. Alternatively put [texscale](sauerbraten.org/docs/editref.html#texscale) in the *\*.tex* configuration of the given texture. 
- Review the */media/map/yourmap_imported.cfg*
  - Look for the *texload* commands; `texload <yourtex>` will load the texture configured in *media/texture/yourtex.tex*
  - Note that if the *importassaultcube* command does not find a given *.tex file it will fallback to the legacy texture definition instead. Example:
    - New: `texload "makke/rock"; texscale 2.0; setshader "stdworld";`
    - Legacy: `texture 0 "makke/rock"; texscale %.1f`
  - The cfg may contain some broken lines such as "0 0 0 xyz" due to a bug in the importer, clear those lines 
  - If the cfg contains a *fog* command, fix the fog level by multiplying the value by 976 e.g. `fog 488292 // fog 500 in ac v1`
  - If the cfg contains a *shadowyaw* command, remove it since it is not supported anymore

### Texture Migration ###

<img src="/doc/normalmap.png" width="50%">

#### Guidelines ####

Minimal:
- All existing AC v1 textures should be migrated to this new configuration and should *at least have a normalmap*. Most likely there is no point in having some texture remain diffuse-only.

Optimal:
- The textures should be replaced by modern high-res textures.

#### Procedure ####

AC v1 supported diffuse textures only whereas Tesseract supports textures slots with eight different types: primary diffuse, secondary diffuse, decals, normal map, glow map, specularity map, depth map and environment map. There should be a *\*.tex* file per texture slot to define the types and that shader that is applied.

Examples:
- [Wood texture with Parallax Mapping](/media/texture/noctua/wood/planks02.tex)
- [Bricks texture with Parallax Mapping and Specular Mapping](/media/texture/mayang/bricks_2.tex)
- [Iron texture with Specular Mapping](/media/texture/noctua/metal/iron02.tex)

See also [texture types](http://sauerbraten.org/docs/editref.html#texture) and [shaders table](http://sauerbraten.org/docs/editref.html#setuniformparam).

Unfortunately [PBR](https://en.wikipedia.org/wiki/Physically_based_rendering) and [GLTF](https://en.wikipedia.org/wiki/GlTF) is not supported by Tesseract and it is not foreseeable that this functionality will be added to the engine in the near future. This means that the import of modern assets (such as models/textures from sketchfab.com) is not straightforward plus even if the import works there is a quality loss. The 'advantage' is that without PBR the engine will run on old machines and/or support large maps - plus the file size of the textures are not as big as in AAA games. So, if you download new PBR textures from the web you need to convert them first to conventional textures:

- COLOR + AO -> DIFFUSE
   - blend the COLOR texture with the AO texture using 'multiply' blending function 
   - save this diffuse texture as \**color.png*
   - GIMP: To do this in gimp, open the first texture, paste the second texture as new layer, edit layer attributes and set to mode to "Multiply", flatten image, export to png
- ROUGHNESS + METALLIC -> SPECULAR MAP
   - blend the ROUGHNESS texture in such a way with the METALLIC texture so that the dark/black areas of the METALLC texture remain dark
   - compose the resulting grayscale image into the red channel of a new image, keep the green and blue channels empty/white
   - save this specular map as \**specular.png*
   - GIMP: To do this in gimp, open the first texture, paste the second texture as new layer, edit layer attributes and set to mode to "Darken Only", flatten image, export to png
- NORMAL MAP -> ~NORMAL MAP
   - Tesseract requires normal maps to have white faces to be down and dark faces to be up in the green channel
   - If this is not the case yet, simply invert the green channel of the texture 
   - save the normal map as \**normal.png*
   - GIMP: To do this in gimp, open the texture, Colors->Components->Decompose, select green layer, Colors->Invert, Colors->Components->Compose, export to png
 - Model Skins
   - If you want to apply the texture to a model skin e.g. with the `md3skin` or `md5skin` or `iqmskin` commands then you need to encode the specular map into a mask
   - The mask is encoded as follows: red channel -> specular map, green channel -> glow map, blue channel -> environment map
   - GIMP: To do this in gimp, open the specular map (\**specular.png*) then add two additional empty layers, Color->Components-Compose, chose initial layer as 'green', export to png
 - Examples:
   - Sniper2 rifle PBR textures migrated to conventional textures: [Source](/media/model/hudgun/sniper2/originalpackage/textures) -> [Target](/media/model/hudgun/sniper2/) 

### Weapon Model Migration (HUD) ###

<img src="/doc/hudgun.PNG" width="45%"> <img src="/doc/sniper2.png" width="45%">

#### Guidelines ####

Minimal:
- The weapons shown in the head-up-display should have at least a good *normalmap* and a *specmap*. 
- The hands must not have any specularity. 
- The look of the weapons should generally be very close to v1 due to [tradition](#Balancing-Innovation-and-Tradition).

Optimal:
 - models could have more polygons so that the round surfaces like barrel and scope are smoothened.
 - the existing skin textures contain highlighted areas to simulate specularity, this could be removed because this should now be done via spemap instead
 - the skins textures could be re-created in high-res
 - animations could be added such as reload select/deselect weapon 

#### Procedure ####

Example:
- [Sniper rifle md3 config file](/media/model/hudgun/sniper/md3.cfg)
- [Sniper2 rifle iqm config file](/media/model/hudgun/sniper2/iqm.cfg)

Notes:
- `md3load <model>` loads a given md3 file
- `md3skin <mesh> <skin> <mask>` applies a diffuse texture *skin* to the *mesh* with a given *mask*. The *mask* is an ordinary RGB picture where the R(ed) channel stores a specular map, the G(reen) channel stores glow map and the B(blue) channel stores environment map.
- `md3bumpmap <mesh> <normalmap>` applies a *normalmap* to the *mesh*
- `md3spec <mesh> -1` disables specularity for the given *mesh*. This is a useful shortcut so that you do not need to provide a *mask* with disabled specularity.

### Weapon Model Migration (World) ####

Same applies as in [Weapon Model Migration (HUD)](#weapon-model-migration-hud) - except "hands".

### Playermodel Migration ###

<img src="/doc/terrorist.png" width="45%"> <img src="/doc/swat.png" width="45%">

#### Guidelines ####

Minimal:
- The AC v1 playermodels should not be retained. They stem from the Quake II era an d were already outdated when developmen on AC v1 began. 
- The replacement should have the same style that means special forces vs. rebels.
- Faces should be covered as in AC v1 - makes it less violent.
- Use freely available player models if there are any suitable ones. Examples: [terrorist](https://sketchfab.com/3d-models/terrorist-ccce1abd9086451da3bed9ec30c82c37), [thugs](https://sketchfab.com/3d-models/thugs-ee6139403e5c4840a93c63b66340a781) and [SWAT](https://sketchfab.com/3d-models/swat-character-d4235ba6f1ab4322ad262512183fbc6c)
- Reuse the existing skeletal animation of the Tesseract "bones" playermodel (requires a manual migration)

Optimal:
- Customize the shape of the freely available player model to fit the AC style (for example, remove beards and hat of "Seaman" playermodels and add a rebel-like hoodie).
  - And/Or: Customize the skin of the freely available player models (for example, add "CLA" or "RVSF" tags).
- Do not reuse the existing skeletal animation of the tesseract "bones" playermodel but create custom animations

#### Procedure ####

Static player models can be migrated to utilize the skeleton of the Tesseract "bones" playermodel. This allows to re-use the existing animations (run, jump, ..) and allows to re-use the existing ragdoll definition. 

Example:
- Install latest Blender
- Install [MD5 addon] for Blender (https://github.com/KozGit/Blender-2.8-MD5-import-export-addon) 
- Download the [thugs](https://sketchfab.com/3d-models/thugs-ee6139403e5c4840a93c63b66340a781) playermodels
- Import the \**.fbx* file into Blender
- Delete one of the players so that only one remains
- Place the remaining player at the center of the coordinate system
- Open a new instance of Blender and import the [mesh file](https://github.com/drian0/ac_tech_prototype/blob/master/media/model/player/bones/model.md5mesh) of the bones playermodel using the previously installed addon
- If you run into issues during the import open the mesh file with a text editor and ensure that after each `mesh {` there are at least these lines:

// meshes: hat1

shader "hat1"

numverts 1234

- Select the skeleton (Armature) and copy it
- In the first blender instance where the player resides paste the skeleton 
- Note that the player and skeleton are of different scale and therefore do not fit, also there might be other axis or offsets
- Move, rotate and scale the player so that the player aligns somewhat the pasted skeleton
- In Blender Pose Mode rotate the bones (based on the existing skeleton) so that the vertices fit perfectly to the new skeleton, do this perfectly including all finger bones
  - do not move or squeeze the bones - only rotate the angles in order to keep the ratio   
- Vertex Groups are mapped to Armature Bones by name, this means you need to rename the Vertex Groups match the bone names of the new skeleton
  - There are bones in the new skeleton that did not exist in the existing skeleton so you need to manually configure them in Weight Paint mode
- Apply the newly configured pose frome the Pose Mode to the Edit Mode [with this approach](https://nixart.wordpress.com/2013/03/28/modifying-the-rest-pose-in-blender/)
- Export the model to an md5 mesh file using the addon
- Open the file md5 mesh file with a text editor and ensure that all objects are contained and also ensure that after each `mesh`there are at least these lines:

// meshes: hat1

shader "hat1"

numverts 1234

- Import the first animation file of the bones player model using the addon (for example: run)
- Export the animation file using the addon - this ensure that the animation is compatible to the mesh (ordering of joints/etc. matters and so we need to import/export)
  - Do this for all animation files 
- Update the ragdoll.cfg so that the index of the joints are properly referenced, order matters
  - For example, review the [joints { section](/media/model/player/bones/model.md5mesh?token=ASWVSVZ5RHPQ2VWIMF3MNCDAUNNZY) of the bones player model and take note of the order of the joints, starting at index 0
  - Note the [rdjoint entries](/media/model/player/bones/ragdoll.cfg#L40) in the ragdoll.cfg of the bones playermodel  
  - For exaxmple `rdjoint 5 9 1` references join at index 5 which is "L_thigh" and so if the indexes changes after exporting the model then the ragdoll.cfg needs to be updated accordingly
  - To migrate textures/skins of the model please see section [Texture Migration](#procedure)

### Mapmodel Migration ###

Same applies as in [Weapon Model Migration (HUD)](#weapon-model-migration-hud) - except "hands".

### Sound Migration ###

Sounds/audio could be reused 100% initially.
