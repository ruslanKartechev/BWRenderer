#pragma once

inline const char* Shader_SkyboxDefault = "DefaultSkybox";
inline const char* Shader_DefaultForward = "Default3D";
inline const char* Shader_DefaultDeferredG = "Deferred_G_3D";
inline const char* Shader_DefaultDeferredL = "Deferred_L_3D";

inline const char* Shader_Default2D = "Default2D";

inline const char* Shader_Debug = "DebugShader";
inline const char* Shader_ScreenRender = "ScreenRenderTexture";

inline const char* ID_UNIFORM_MODEL = "MATRIX_MODEL";
inline const char* ID_UNIFORM_VIEW = "MATRIX_VIEW";
inline const char* ID_UNIFORM_PROJECTION = "MATRIX_PROJECTION";
inline const char* ID_UNIFORM_PROJECTION_INVERSE = "MATRIX_INVERSE_PROJECTION";

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
inline const char* ID_SSR_POWER = "_SSR_POWER";

inline const char* ID_MAIN_TEXTURE = "mainTex";
inline const char* ID_FAR_PLANE = "FAR_PLANE";
inline const char* ID_NEAR_PLANE = "NEAR_PLANE";

inline const char* ID_DEPTH_TEX = "u_DepthTex";
inline const char* ID_COLOR_TEX = "u_ColorTex";
inline const char* ID_NORMALS_TEX = "u_NormalTex";


inline const char* MESH_CUBE = "cube";
inline const char* MESH_SPHERE = "sphere";
inline const char* MESH_CAPSULE = "capsule";
inline const char* MESH_PLANE = "plane";
inline const char* MESH_QUAD = "quad";
inline const char* MESH_PYRAMID = "pyramid";
inline const char* MESH_DONUT = "donut";
inline const char* MESH_CUT_CONE = "cutCone";


inline const char* TEX_DEFAULT_NORMAL = "default_normal";
inline const char* TEX_DEFAULT_WHITE = "default_white";
