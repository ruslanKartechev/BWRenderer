#pragma once

inline const char* Shader_SkyboxDefault = "DefaultSkybox";

inline const char* ID_UNIFORM_MODEL = "MATRIX_MODEL";
inline const char* ID_UNIFORM_VIEW = "MATRIX_VIEW";
inline const char* ID_UNIFORM_PROJECTION = "MATRIX_PROJECTION";

inline const char* ID_UNIFORM_TIME = "GLOBAL_TIME";
inline const char* ID_UNIFORM_VIEW_POS = "VIEW_POS";
inline const char* ID_UNIFORM_AMBIENT_LIGHT_COLOR = "_AMBIENT_LIGHT_COLOR";
inline const char* ID_UNIFORM_AMBIENT_LIGHT_INTENSITY = "_AMBIENT_LIGHT_INTENSITY";

inline const char* ID_COLOR_TINT = "_COLOR_TINT";
inline const char* ID_BASE_MAP = "_BASE_MAP";
inline const char* ID_NORMAL_MAP = "_NORMAL_MAP";
inline const char* ID_ALPHA_CLIP_VALUE = "_ALPHA_CLIP_VALUE";

/// Tiling (xy) and Offset (zw)
inline const char* ID_BASE_MAP_TO = "_BASE_MAP_TO";

/// Reflections from the skybox
inline const char* ID_SMOOTHNESS = "_SMOOTHNESS";
inline const char* ID_METALLIC = "_METALLIC";

inline const char* ID_SKYBOX_BRIGHTNESS = "_BRIGHTNESS";

inline const char* ID_SKYBOX_CUBEMAP = "_SKYBOX";

inline const char* ID_UNIFORM_SCREEN_TEXTURE = "screenTexture";

inline const char* MESH_CUBE = "cube";
inline const char* MESH_SPHERE = "sphere";
inline const char* MESH_CAPSULE = "capsule";
inline const char* MESH_PLANE = "plane";
inline const char* MESH_QUAD = "quad";
inline const char* MESH_PYRAMID = "pyramid";
inline const char* MESH_DONUT = "donut";
inline const char* MESH_CUT_CONE = "cutCone";
