import bpy

# Some initial stuff that needs to be done manually
# Join the three O's
# Move the square and the dots outside the blender.svg collection

for index, obj in enumerate(bpy.data.objects):
    if obj.name.startswith('path'):
        obj.name = 'obj%02d'% index
        
# Compute the bounding box        
for index in range(0, 110, 1):
    name = 'bound%02d' % index
    if name in bpy.data.objects: continue
    bpy.ops.mesh.primitive_circle_add(enter_editmode=False, align='WORLD', location=(0, 0, 0), scale=(1, 1, 1))
    # bpy.ops.outliner.item_activate(extend=False, deselect_all=True)
    # bpy.context.space_data.context = 'MODIFIER'
    bpy.ops.object.modifier_add(type='SHRINKWRAP')
    bpy.context.object.modifiers["Shrinkwrap"].wrap_method = 'NEAREST_VERTEX'
    bpy.context.object.modifiers["Shrinkwrap"].offset = 0.0025
    bpy.context.object.modifiers["Shrinkwrap"].target = bpy.data.objects["obj%02d" % index]
    bpy.ops.object.modifier_apply(modifier="Shrinkwrap", report=True)
    bpy.data.objects['Circle'].name = 'bound%02d' % index
    

def cone_extrude(object_name):
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = bpy.data.objects[object_name]
    bpy.ops.object.editmode_toggle()
    bpy.ops.mesh.extrude_region_move(MESH_OT_extrude_region={"use_normal_flip":False, "use_dissolve_ortho_edges":False, "mirror":False}, TRANSFORM_OT_translate={"value":(-0, -0, -0.018), "orient_type":'GLOBAL', "orient_matrix":((1, 0, 0), (0, 1, 0), (0, 0, 1)), "orient_matrix_type":'GLOBAL', "constraint_axis":(False, False, True), "mirror":False, "use_proportional_edit":False, "proportional_edit_falloff":'SMOOTH', "proportional_size":1, "use_proportional_connected":False, "use_proportional_projected":False, "snap":False, "snap_target":'CLOSEST', "snap_point":(0, 0, 0), "snap_align":False, "snap_normal":(0, 0, 0), "gpencil_strokes":False, "cursor_transform":False, "texture_space":False, "remove_on_cancel":False, "release_confirm":False, "use_accurate":False, "use_automerge_and_split":False})
    bpy.ops.transform.resize(value=(0.5, 0.5, 0.5), orient_type='GLOBAL', orient_matrix=((1, 0, 0), (0, 1, 0), (0, 0, 1)), orient_matrix_type='GLOBAL', mirror=True, use_proportional_edit=False, proportional_edit_falloff='SMOOTH', proportional_size=1, use_proportional_connected=False, use_proportional_projected=False)
    bpy.ops.object.editmode_toggle()

for obj in bpy.data.objects:
    if obj.name.startswith('bound'):
        cone_extrude(obj.name)    
