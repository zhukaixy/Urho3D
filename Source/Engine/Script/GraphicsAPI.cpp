//
// Copyright (c) 2008-2014 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Precompiled.h"
#include "AnimatedModel.h"
#include "Animation.h"
#include "AnimationController.h"
#include "AnimationState.h"
#include "APITemplates.h"
#include "Camera.h"
#include "CustomGeometry.h"
#include "DebugRenderer.h"
#include "DecalSet.h"
#include "Graphics.h"
#include "Light.h"
#include "Material.h"
#include "Octree.h"
#include "ParticleEmitter.h"
#include "Renderer.h"
#include "RenderPath.h"
#include "Scene.h"
#include "SmoothedTransform.h"
#include "StaticModelGroup.h"
#include "Technique.h"
#include "Terrain.h"
#include "TerrainPatch.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "TextureCube.h"
#include "Skybox.h"
#include "Zone.h"

#ifdef _MSC_VER
#pragma warning(disable:4345)
#endif

namespace Urho3D
{

void FakeAddRef(void* ptr);
void FakeReleaseRef(void* ptr);

static void RegisterCamera(asIScriptEngine* engine)
{
    engine->RegisterGlobalProperty("const uint VO_NONE", (void*)&VO_NONE);
    engine->RegisterGlobalProperty("const uint VO_LOW_MATERIAL_QUALITY", (void*)&VO_LOW_MATERIAL_QUALITY);
    engine->RegisterGlobalProperty("const uint VO_DISABLE_SHADOWS", (void*)&VO_DISABLE_SHADOWS);
    engine->RegisterGlobalProperty("const uint VO_DISABLE_OCCLUSION", (void*)&VO_DISABLE_OCCLUSION);
    
    engine->RegisterEnum("FillMode");
    engine->RegisterEnumValue("FillMode", "FILL_SOLID", FILL_SOLID);
    engine->RegisterEnumValue("FillMode", "FILL_WIREFRAME", FILL_WIREFRAME);
    engine->RegisterEnumValue("FillMode", "FILL_POINT", FILL_POINT);
    
    RegisterComponent<Camera>(engine, "Camera");
    engine->RegisterObjectMethod("Camera", "void SetOrthoSize(const Vector2&in)", asMETHODPR(Camera, SetOrthoSize, (const Vector2&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Frustum GetSplitFrustum(float, float) const", asMETHOD(Camera, GetSplitFrustum), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Ray GetScreenRay(float, float) const", asMETHOD(Camera, GetScreenRay), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Vector2 WorldToScreenPoint(const Vector3&in) const", asMETHOD(Camera, WorldToScreenPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Vector3 ScreenToWorldPoint(const Vector3&in) const", asMETHOD(Camera, ScreenToWorldPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float GetDistance(const Vector3&in) const", asMETHOD(Camera, GetDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float GetDistanceSquared(const Vector3&in) const", asMETHOD(Camera, GetDistanceSquared), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_nearClip(float)", asMETHOD(Camera, SetNearClip), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_nearClip() const", asMETHOD(Camera, GetNearClip), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_farClip(float)", asMETHOD(Camera, SetFarClip), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_farClip() const", asMETHOD(Camera, GetFarClip), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_fov(float)", asMETHOD(Camera, SetFov), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_fov() const", asMETHOD(Camera, GetFov), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_orthoSize(float)", asMETHODPR(Camera, SetOrthoSize, (float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_orthoSize() const", asMETHOD(Camera, GetOrthoSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_aspectRatio(float)", asMETHOD(Camera, SetAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_aspectRatio() const", asMETHOD(Camera, GetAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_zoom(float)", asMETHOD(Camera, SetZoom), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_zoom() const", asMETHOD(Camera, GetZoom), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_lodBias(float)", asMETHOD(Camera, SetLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_lodBias() const", asMETHOD(Camera, GetLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_orthographic(bool)", asMETHOD(Camera, SetOrthographic), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "bool get_orthographic() const", asMETHOD(Camera, IsOrthographic), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_autoAspectRatio(bool)", asMETHOD(Camera, SetAutoAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "bool get_autoAspectRatio() const", asMETHOD(Camera, GetAutoAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_projectionOffset(const Vector2&in)", asMETHOD(Camera, SetProjectionOffset), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Vector2& get_projectionOffset() const", asMETHOD(Camera, GetProjectionOffset), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_useReflection(bool)", asMETHOD(Camera, SetUseReflection), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "bool get_useReflection() const", asMETHOD(Camera, GetUseReflection), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_reflectionPlane(const Plane&in) const", asMETHOD(Camera, SetReflectionPlane), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Plane& get_reflectionPlane() const", asMETHOD(Camera, GetReflectionPlane), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_useClipping(bool)", asMETHOD(Camera, SetUseClipping), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "bool get_useClipping() const", asMETHOD(Camera, GetUseClipping), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_clipPlane(const Plane&in) const", asMETHOD(Camera, SetClipPlane), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Plane& get_clipPlane() const", asMETHOD(Camera, GetClipPlane), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_viewMask(uint)", asMETHOD(Camera, SetViewMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "uint get_viewMask() const", asMETHOD(Camera, GetViewMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_viewOverrideFlags(uint)", asMETHOD(Camera, SetViewOverrideFlags), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "uint get_viewOverrideFlags() const", asMETHOD(Camera, GetViewOverrideFlags), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "void set_fillMode(FillMode)", asMETHOD(Camera, SetFillMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "FillMode get_fillMode() const", asMETHOD(Camera, GetFillMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Frustum& get_frustum() const", asMETHOD(Camera, GetFrustum), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Matrix4& get_projection() const", asMETHODPR(Camera, GetProjection, () const, const Matrix4&), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "const Matrix3x4& get_view() const", asMETHOD(Camera, GetView), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Frustum get_viewSpaceFrustum() const", asMETHOD(Camera, GetViewSpaceFrustum), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "float get_halfViewSize() const", asMETHOD(Camera, GetHalfViewSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Matrix3x4 get_effectiveWorldTransform() const", asMETHOD(Camera, GetEffectiveWorldTransform), asCALL_THISCALL);

    // Register Variant GetPtr() for Camera. This is deprecated, GetPtr() should be used instead.
    engine->RegisterObjectMethod("Variant", "Camera@+ GetCamera(const String&in binding = \"deprecated:GetCamera\") const", asFUNCTION(GetVariantPtr<Camera>), asCALL_CDECL_OBJLAST);
}

static Node* BoneGetNode(Bone* ptr)
{
    return ptr->node_;
}

static void BoneSetNode(Node* node, Bone* ptr)
{
    ptr->node_ = node;
}

static void RegisterSkeleton(asIScriptEngine* engine)
{
    engine->RegisterObjectType("Bone", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("Bone", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Bone", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("Bone", "const String name", offsetof(Bone, name_));
    engine->RegisterObjectProperty("Bone", "const Vector3 initialPosition", offsetof(Bone, initialPosition_));
    engine->RegisterObjectProperty("Bone", "const Quaternion initialRotation", offsetof(Bone, initialRotation_));
    engine->RegisterObjectProperty("Bone", "const Vector3 initialScale", offsetof(Bone, initialScale_));
    engine->RegisterObjectProperty("Bone", "bool animated", offsetof(Bone, animated_));
    engine->RegisterObjectProperty("Bone", "float radius", offsetof(Bone, radius_));
    engine->RegisterObjectProperty("Bone", "const BoundingBox boundingBox", offsetof(Bone, boundingBox_));
    engine->RegisterObjectMethod("Bone", "void set_node(Node@+)", asFUNCTION(BoneSetNode), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Bone", "Node@+ get_node() const", asFUNCTION(BoneGetNode), asCALL_CDECL_OBJLAST);
    
    engine->RegisterObjectType("Skeleton", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("Skeleton", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Skeleton", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Skeleton", "void Reset()", asMETHOD(Skeleton, Reset), asCALL_THISCALL);
    engine->RegisterObjectMethod("Skeleton", "Bone@+ GetBone(const String&in) const", asMETHODPR(Skeleton, GetBone, (const String&), Bone*), asCALL_THISCALL);
    engine->RegisterObjectMethod("Skeleton", "Bone@+ get_rootBone() const", asMETHOD(Skeleton, GetRootBone), asCALL_THISCALL);
    engine->RegisterObjectMethod("Skeleton", "uint get_numBones() const", asMETHOD(Skeleton, GetNumBones), asCALL_THISCALL);
    engine->RegisterObjectMethod("Skeleton", "Bone@+ get_bones(uint)", asMETHODPR(Skeleton, GetBone, (unsigned), Bone*), asCALL_THISCALL);
}

static Viewport* ConstructViewport()
{
    return new Viewport(GetScriptContext());
}

static Viewport* ConstructViewportSceneCamera(Scene* scene, Camera* camera, RenderPath* renderPath)
{
    return new Viewport(GetScriptContext(), scene, camera, renderPath);
}

static Viewport* ConstructViewportSceneCameraRect(Scene* scene, Camera* camera, const IntRect& rect, RenderPath* renderPath)
{
    return new Viewport(GetScriptContext(), scene, camera, rect, renderPath);
}

static bool Texture2DLoad(Image* image, bool useAlpha, Texture2D* ptr)
{
    return ptr->Load(SharedPtr<Image>(image), useAlpha);
}

static bool Texture3DLoad(Image* image, bool useAlpha, Texture3D* ptr)
{
    return ptr->Load(SharedPtr<Image>(image), useAlpha);
}

static bool TextureCubeLoad(CubeMapFace face, Image* image, bool useAlpha, TextureCube* ptr)
{
    return ptr->Load(face, SharedPtr<Image>(image), useAlpha);
}

static void ConstructRenderTargetInfo(RenderTargetInfo* ptr)
{
    new(ptr) RenderTargetInfo();
}

static void ConstructRenderTargetInfoCopy(const RenderTargetInfo& info, RenderTargetInfo* ptr)
{
    new(ptr) RenderTargetInfo(info);
}

static void DestructRenderTargetInfo(RenderTargetInfo* ptr)
{
    ptr->~RenderTargetInfo();
}

static void ConstructRenderPathCommand(RenderPathCommand* ptr)
{
    new(ptr) RenderPathCommand();
}

static void ConstructRenderPathCommandCopy(const RenderPathCommand& command, RenderPathCommand* ptr)
{
    new(ptr) RenderPathCommand(command);
}

static void DestructRenderPathCommand(RenderPathCommand* ptr)
{
    ptr->~RenderPathCommand();
}

static RenderPath* ConstructRenderPath()
{
    return new RenderPath();
}

static RenderPath* RenderPathClone(RenderPath* ptr)
{
    SharedPtr<RenderPath> clone = ptr->Clone();
    // The shared pointer will go out of scope, so have to increment the reference count
    // (here an auto handle can not be used)
    clone->AddRef();
    return clone.Get();
}

static RenderTargetInfo* RenderPathGetRenderTarget(unsigned index, RenderPath* ptr)
{
    if (index >= ptr->renderTargets_.Size())
    {
        asIScriptContext* context = asGetActiveContext();
        if (context)
            context->SetException("Index out of bounds");
        return 0;
    }
    else
        return &ptr->renderTargets_[index];
}

static RenderPathCommand* RenderPathGetCommand(unsigned index, RenderPath* ptr)
{
    if (index >= ptr->commands_.Size())
    {
        asIScriptContext* context = asGetActiveContext();
        if (context)
            context->SetException("Index out of bounds");
        return 0;
    }
    else
        return &ptr->commands_[index];
}

static void RegisterRenderPath(asIScriptEngine* engine)
{
    engine->RegisterEnum("RenderCommandType");
    engine->RegisterEnumValue("RenderCommandType", "CMD_NONE", CMD_NONE);
    engine->RegisterEnumValue("RenderCommandType", "CMD_CLEAR", CMD_CLEAR);
    engine->RegisterEnumValue("RenderCommandType", "CMD_SCENEPASS", CMD_SCENEPASS);
    engine->RegisterEnumValue("RenderCommandType", "CMD_QUAD", CMD_QUAD);
    engine->RegisterEnumValue("RenderCommandType", "CMD_FORWARDLIGHTS", CMD_FORWARDLIGHTS);
    engine->RegisterEnumValue("RenderCommandType", "CMD_LIGHTVOLUMES", CMD_LIGHTVOLUMES);
    
    engine->RegisterEnum("RenderCommandSortMode");
    engine->RegisterEnumValue("RenderCommandSortMode", "SORT_FRONTTOBACK", SORT_FRONTTOBACK);
    engine->RegisterEnumValue("RenderCommandSortMode", "SORT_BACKTOFRONT", SORT_BACKTOFRONT);
    
    engine->RegisterEnum("RenderTargetSizeMode");
    engine->RegisterEnumValue("RenderTargetSizeMode", "SIZE_ABSOLUTE", SIZE_ABSOLUTE);
    engine->RegisterEnumValue("RenderTargetSizeMode", "SIZE_RENDERTARGETDIVISOR", SIZE_RENDERTARGETDIVISOR);
    engine->RegisterEnumValue("RenderTargetSizeMode", "SIZE_VIEWPORTDIVISOR", SIZE_VIEWPORTDIVISOR);
    
    engine->RegisterEnum("TextureUnit");
    engine->RegisterEnumValue("TextureUnit", "TU_DIFFUSE", TU_DIFFUSE);
    engine->RegisterEnumValue("TextureUnit", "TU_NORMAL", TU_NORMAL);
    engine->RegisterEnumValue("TextureUnit", "TU_SPECULAR", TU_SPECULAR);
    engine->RegisterEnumValue("TextureUnit", "TU_EMISSIVE", TU_EMISSIVE);
    engine->RegisterEnumValue("TextureUnit", "TU_ENVIRONMENT", TU_ENVIRONMENT);
    engine->RegisterEnumValue("TextureUnit", "TU_LIGHTRAMP", TU_LIGHTRAMP);
    engine->RegisterEnumValue("TextureUnit", "TU_LIGHTSHAPE", TU_LIGHTSHAPE);
    engine->RegisterEnumValue("TextureUnit", "TU_SHADOWMAP", TU_SHADOWMAP);
    engine->RegisterEnumValue("TextureUnit", "TU_FACESELECT", TU_FACESELECT);
    engine->RegisterEnumValue("TextureUnit", "TU_INDIRECTION", TU_INDIRECTION);
    engine->RegisterEnumValue("TextureUnit", "TU_ALBEDOBUFFER", TU_ALBEDOBUFFER);
    engine->RegisterEnumValue("TextureUnit", "TU_NORMALBUFFER", TU_NORMALBUFFER);
    engine->RegisterEnumValue("TextureUnit", "TU_DEPTHBUFFER", TU_DEPTHBUFFER);
    engine->RegisterEnumValue("TextureUnit", "TU_LIGHTBUFFER", TU_LIGHTBUFFER);
    engine->RegisterEnumValue("TextureUnit", "TU_VOLUMEMAP", TU_VOLUMEMAP);
    engine->RegisterEnumValue("TextureUnit", "MAX_MATERIAL_TEXTURE_UNITS", MAX_MATERIAL_TEXTURE_UNITS);
    engine->RegisterEnumValue("TextureUnit", "MAX_TEXTURE_UNITS", MAX_TEXTURE_UNITS);
    
    engine->RegisterObjectType("RenderTargetInfo", sizeof(RenderTargetInfo), asOBJ_VALUE | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("RenderTargetInfo", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructRenderTargetInfo), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("RenderTargetInfo", asBEHAVE_CONSTRUCT, "void f(const RenderTargetInfo&in)", asFUNCTION(ConstructRenderTargetInfoCopy), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("RenderTargetInfo", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructRenderTargetInfo), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RenderTargetInfo", "RenderTargetInfo& opAssign(const RenderTargetInfo&in)", asMETHOD(RenderTargetInfo, operator =), asCALL_THISCALL);
    engine->RegisterObjectProperty("RenderTargetInfo", "String name", offsetof(RenderTargetInfo, name_));
    engine->RegisterObjectProperty("RenderTargetInfo", "String tag", offsetof(RenderTargetInfo, tag_));
    engine->RegisterObjectProperty("RenderTargetInfo", "uint format", offsetof(RenderTargetInfo, format_));
    engine->RegisterObjectProperty("RenderTargetInfo", "IntVector2 size", offsetof(RenderTargetInfo, size_));
    engine->RegisterObjectProperty("RenderTargetInfo", "RenderTargetSizeMode sizeMode", offsetof(RenderTargetInfo, sizeMode_));
    engine->RegisterObjectProperty("RenderTargetInfo", "bool enabled", offsetof(RenderTargetInfo, enabled_));
    engine->RegisterObjectProperty("RenderTargetInfo", "bool filtered", offsetof(RenderTargetInfo, filtered_));
    engine->RegisterObjectProperty("RenderTargetInfo", "bool sRGB", offsetof(RenderTargetInfo, sRGB_));
    engine->RegisterObjectProperty("RenderTargetInfo", "bool persistent", offsetof(RenderTargetInfo, persistent_));
    
    engine->RegisterObjectType("RenderPathCommand", sizeof(RenderPathCommand), asOBJ_VALUE | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("RenderPathCommand", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructRenderPathCommand), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("RenderPathCommand", asBEHAVE_CONSTRUCT, "void f(const RenderPathCommand&in)", asFUNCTION(ConstructRenderPathCommandCopy), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("RenderPathCommand", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructRenderPathCommand), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RenderPathCommand", "void RemoveShaderParameter(const String&in)", asMETHOD(RenderPathCommand, RemoveShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "RenderPathCommand& opAssign(const RenderPathCommand&in)", asMETHOD(RenderPathCommand, operator =), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "void set_textureNames(TextureUnit, const String&in)", asMETHOD(RenderPathCommand, SetTextureName), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "const String& get_textureNames(TextureUnit) const", asMETHOD(RenderPathCommand, GetTextureName), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "void set_shaderParameters(const String&in, const Variant&in)", asMETHOD(RenderPathCommand, SetShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "const Variant& get_shaderParameters(const String&in) const", asMETHOD(RenderPathCommand, GetShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "void set_numOutputs(uint)", asMETHOD(RenderPathCommand, SetNumOutputs), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "uint get_numOutputs() const", asMETHOD(RenderPathCommand, GetNumOutputs), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "void set_outputNames(uint, const String&in)", asMETHOD(RenderPathCommand, SetOutputName), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPathCommand", "const String& get_outputNames(uint) const", asMETHOD(RenderPathCommand, GetOutputName), asCALL_THISCALL);
    engine->RegisterObjectProperty("RenderPathCommand", "String tag", offsetof(RenderPathCommand, tag_));
    engine->RegisterObjectProperty("RenderPathCommand", "RenderCommandType type", offsetof(RenderPathCommand, type_));
    engine->RegisterObjectProperty("RenderPathCommand", "RenderCommandSortMode sortMode", offsetof(RenderPathCommand, sortMode_));
    engine->RegisterObjectProperty("RenderPathCommand", "String pass", offsetof(RenderPathCommand, pass_));
    engine->RegisterObjectProperty("RenderPathCommand", "String metadata", offsetof(RenderPathCommand, metadata_));
    engine->RegisterObjectProperty("RenderPathCommand", "uint clearFlags", offsetof(RenderPathCommand, clearFlags_));
    engine->RegisterObjectProperty("RenderPathCommand", "Color clearColor", offsetof(RenderPathCommand, clearColor_));
    engine->RegisterObjectProperty("RenderPathCommand", "float clearDepth", offsetof(RenderPathCommand, clearDepth_));
    engine->RegisterObjectProperty("RenderPathCommand", "uint clearStencil", offsetof(RenderPathCommand, clearStencil_));
    engine->RegisterObjectProperty("RenderPathCommand", "bool enabled", offsetof(RenderPathCommand, enabled_));
    engine->RegisterObjectProperty("RenderPathCommand", "bool useFogColor", offsetof(RenderPathCommand, useFogColor_));
    engine->RegisterObjectProperty("RenderPathCommand", "bool markToStencil", offsetof(RenderPathCommand, markToStencil_));
    engine->RegisterObjectProperty("RenderPathCommand", "bool vertexLights", offsetof(RenderPathCommand, vertexLights_));
    engine->RegisterObjectProperty("RenderPathCommand", "bool useLitBase", offsetof(RenderPathCommand, useLitBase_));
    engine->RegisterObjectProperty("RenderPathCommand", "String vertexShaderName", offsetof(RenderPathCommand, vertexShaderName_));
    engine->RegisterObjectProperty("RenderPathCommand", "String pixelShaderName", offsetof(RenderPathCommand, pixelShaderName_));
    engine->RegisterObjectProperty("RenderPathCommand", "String vertexShaderDefines", offsetof(RenderPathCommand, vertexShaderDefines_));
    engine->RegisterObjectProperty("RenderPathCommand", "String pixelShaderDefines", offsetof(RenderPathCommand, pixelShaderDefines_));
    
    RegisterRefCounted<RenderPath>(engine, "RenderPath");
    engine->RegisterObjectBehaviour("RenderPath", asBEHAVE_FACTORY, "RenderPath@+ f()", asFUNCTION(ConstructRenderPath), asCALL_CDECL);
    engine->RegisterObjectMethod("RenderPath", "RenderPath@ Clone()", asFUNCTION(RenderPathClone), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RenderPath", "bool Load(XMLFile@+)", asMETHOD(RenderPath, Load), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "bool Append(XMLFile@+)", asMETHOD(RenderPath, Append), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void SetEnabled(const String&in, bool)", asMETHOD(RenderPath, SetEnabled), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void ToggleEnabled(const String&in)", asMETHOD(RenderPath, ToggleEnabled), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void AddRenderTarget(const RenderTargetInfo&in)", asMETHOD(RenderPath, AddRenderTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void RemoveRenderTarget(uint)", asMETHODPR(RenderPath, RemoveRenderTarget, (unsigned), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void RemoveRenderTarget(const String&in)", asMETHODPR(RenderPath, RemoveRenderTarget, (const String&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void RemoveRenderTargts(const String&in)", asMETHOD(RenderPath, RemoveRenderTargets), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void AddCommand(const RenderPathCommand&in)", asMETHOD(RenderPath, AddCommand), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void InsertCommand(uint, const RenderPathCommand&in)", asMETHOD(RenderPath, InsertCommand), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void RemoveCommand(uint)", asMETHOD(RenderPath, RemoveCommand), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void RemoveCommands(const String&in)", asMETHOD(RenderPath, RemoveCommands), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "uint get_numRenderTargets() const", asMETHOD(RenderPath, GetNumRenderTargets), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void set_renderTargets(uint, const RenderTargetInfo&in)", asMETHOD(RenderPath, SetRenderTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "const RenderTargetInfo& get_renderTargets(uint) const", asFUNCTION(RenderPathGetRenderTarget), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RenderPath", "uint get_numCommands() const", asMETHOD(RenderPath, GetNumCommands), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "void set_commands(uint, const RenderPathCommand&in)", asMETHOD(RenderPath, SetCommand), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "const RenderPathCommand& get_commands(uint) const", asFUNCTION(RenderPathGetCommand), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RenderPath", "void set_shaderParameters(const String&in, const Variant&in)", asMETHOD(RenderPath, SetShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderPath", "const Variant& get_shaderParameters(const String&in) const", asMETHOD(RenderPath, GetShaderParameter), asCALL_THISCALL);
}

static void RegisterTextures(asIScriptEngine* engine)
{
    /// \todo Expose getting/setting raw texture data
    
    engine->RegisterEnum("TextureUsage");
    engine->RegisterEnumValue("TextureUsage", "TEXTURE_STATIC", TEXTURE_STATIC);
    engine->RegisterEnumValue("TextureUsage", "TEXTURE_DYNAMIC", TEXTURE_DYNAMIC);
    engine->RegisterEnumValue("TextureUsage", "TEXTURE_RENDERTARGET", TEXTURE_RENDERTARGET);
    engine->RegisterEnumValue("TextureUsage", "TEXTURE_DEPTHSTENCIL", TEXTURE_DEPTHSTENCIL);
    
    engine->RegisterEnum("TextureFilterMode");
    engine->RegisterEnumValue("TextureFilterMode", "FILTER_NEAREST", FILTER_NEAREST);
    engine->RegisterEnumValue("TextureFilterMode", "FILTER_BILINEAR", FILTER_BILINEAR);
    engine->RegisterEnumValue("TextureFilterMode", "FILTER_TRILINEAR", FILTER_TRILINEAR);
    engine->RegisterEnumValue("TextureFilterMode", "FILTER_ANISOTROPIC", FILTER_ANISOTROPIC);
    engine->RegisterEnumValue("TextureFilterMode", "FILTER_DEFAULT", FILTER_DEFAULT);
    
    engine->RegisterEnum("TextureAddressMode");
    engine->RegisterEnumValue("TextureAddressMode", "ADDRESS_WRAP", ADDRESS_WRAP);
    engine->RegisterEnumValue("TextureAddressMode", "ADDRESS_MIRROR", ADDRESS_MIRROR);
    engine->RegisterEnumValue("TextureAddressMode", "ADDRESS_CLAMP", ADDRESS_CLAMP);
    engine->RegisterEnumValue("TextureAddressMode", "ADDRESS_BORDER", ADDRESS_BORDER);
    
    engine->RegisterEnum("TextureCoordinate");
    engine->RegisterEnumValue("TextureCoordinate", "COORD_U", COORD_U);
    engine->RegisterEnumValue("TextureCoordinate", "COORD_V", COORD_V);
    engine->RegisterEnumValue("TextureCoordinate", "COORD_W", COORD_W);
    
    engine->RegisterEnum("CubeMapFace");
    engine->RegisterEnumValue("CubeMapFace", "FACE_POSITIVE_X", FACE_POSITIVE_X);
    engine->RegisterEnumValue("CubeMapFace", "FACE_NEGATIVE_X", FACE_NEGATIVE_X);
    engine->RegisterEnumValue("CubeMapFace", "FACE_POSITIVE_Y", FACE_POSITIVE_Y);
    engine->RegisterEnumValue("CubeMapFace", "FACE_NEGATIVE_Y", FACE_NEGATIVE_Y);
    engine->RegisterEnumValue("CubeMapFace", "FACE_POSITIVE_Z", FACE_POSITIVE_Z);
    engine->RegisterEnumValue("CubeMapFace", "FACE_NEGATIVE_Z", FACE_NEGATIVE_Z);
    
    engine->RegisterEnum("RenderSurfaceUpdateMode");
    engine->RegisterEnumValue("RenderSurfaceUpdateMode", "SURFACE_MANUALUPDATE", SURFACE_MANUALUPDATE);
    engine->RegisterEnumValue("RenderSurfaceUpdateMode", "SURFACE_UPDATEVISIBLE", SURFACE_UPDATEVISIBLE);
    engine->RegisterEnumValue("RenderSurfaceUpdateMode", "SURFACE_UPDATEALWAYS", SURFACE_UPDATEALWAYS);
    RegisterTexture<Texture>(engine, "Texture");
    
    RegisterObject<Viewport>(engine, "Viewport");
    engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY, "Viewport@+ f()", asFUNCTION(ConstructViewport), asCALL_CDECL);
    engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY, "Viewport@+ f(Scene@+, Camera@+, RenderPath@+ renderPath = null)", asFUNCTION(ConstructViewportSceneCamera), asCALL_CDECL);
    engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY, "Viewport@+ f(Scene@+, Camera@+, const IntRect&in, RenderPath@+ renderPath = null)", asFUNCTION(ConstructViewportSceneCameraRect), asCALL_CDECL);
    engine->RegisterObjectMethod("Viewport", "void SetRenderPath(XMLFile@+)", asMETHODPR(Viewport, SetRenderPath, (XMLFile*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "void set_scene(Scene@+)", asMETHOD(Viewport, SetScene), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "Scene@+ get_scene() const", asMETHOD(Viewport, GetScene), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "void set_camera(Camera@+)", asMETHOD(Viewport, SetCamera), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "Camera@+ get_camera() const", asMETHOD(Viewport, GetCamera), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "void set_renderPath(RenderPath@+)", asMETHODPR(Viewport, SetRenderPath, (RenderPath*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "RenderPath@+ get_renderPath() const", asMETHOD(Viewport, GetRenderPath), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "void set_rect(const IntRect&in)", asMETHOD(Viewport, SetRect), asCALL_THISCALL);
    engine->RegisterObjectMethod("Viewport", "const IntRect& get_rect() const", asMETHOD(Viewport, GetRect), asCALL_THISCALL);
    
    engine->RegisterObjectType("RenderSurface", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("RenderSurface", asBEHAVE_ADDREF, "void f()", asMETHOD(RenderSurface, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour("RenderSurface", asBEHAVE_RELEASE, "void f()", asMETHOD(RenderSurface, ReleaseRef), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void QueueUpdate()", asMETHOD(RenderSurface, QueueUpdate), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "Texture@+ get_parentTexture() const", asMETHOD(RenderSurface, GetParentTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "int get_width() const", asMETHOD(RenderSurface, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "int get_height() const", asMETHOD(RenderSurface, GetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "TextureUsage get_usage() const", asMETHOD(RenderSurface, GetUsage), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void set_numViewports(uint)", asMETHOD(RenderSurface, SetNumViewports), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "uint get_numViewports() const", asMETHOD(RenderSurface, GetNumViewports), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void set_viewports(uint, Viewport@+)", asMETHOD(RenderSurface, SetViewport), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "Viewport@+ get_viewports(uint) const", asMETHOD(RenderSurface, GetViewport), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void set_updateMode(RenderSurfaceUpdateMode)", asMETHOD(RenderSurface, SetUpdateMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "RenderSurfaceUpdateMode get_updateMode() const", asMETHOD(RenderSurface, GetUpdateMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void set_linkedRenderTarget(RenderSurface@+)", asMETHOD(RenderSurface, SetLinkedRenderTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "RenderSurface@+ get_linkedRenderTarget() const", asMETHOD(RenderSurface, GetLinkedRenderTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "void set_linkedDepthStencil(RenderSurface@+)", asMETHOD(RenderSurface, SetLinkedDepthStencil), asCALL_THISCALL);
    engine->RegisterObjectMethod("RenderSurface", "RenderSurface@+ get_linkedDepthStencil() const", asMETHOD(RenderSurface, GetLinkedDepthStencil), asCALL_THISCALL);
    
    // Register Variant GetPtr() for RenderSurface. This is deprecated, GetPtr() should be used instead.
    engine->RegisterObjectMethod("Variant", "RenderSurface@+ GetRenderSurface(const String&in binding = \"deprecated:GetRenderSurface\") const", asFUNCTION(GetVariantPtr<RenderSurface>), asCALL_CDECL_OBJLAST);
    
    RegisterTexture<Texture2D>(engine, "Texture2D");
    engine->RegisterObjectMethod("Texture2D", "bool SetSize(int, int, uint, TextureUsage usage = TEXTURE_STATIC)", asMETHOD(Texture2D, SetSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Texture2D", "bool Load(Image@+, bool useAlpha = false)", asFUNCTION(Texture2DLoad), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Texture2D", "RenderSurface@+ get_renderSurface() const", asMETHOD(Texture2D, GetRenderSurface), asCALL_THISCALL);

    RegisterTexture<Texture3D>(engine, "Texture3D");
    engine->RegisterObjectMethod("Texture3D", "bool SetSize(int, int, uint, TextureUsage usage = TEXTURE_STATIC)", asMETHOD(Texture3D, SetSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Texture3D", "bool Load(Image@+, bool useAlpha = false)", asFUNCTION(Texture3DLoad), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Texture3D", "RenderSurface@+ get_renderSurface() const", asMETHOD(Texture3D, GetRenderSurface), asCALL_THISCALL);
    
    RegisterTexture<TextureCube>(engine, "TextureCube");
    engine->RegisterObjectMethod("TextureCube", "bool SetSize(int, uint, TextureUsage usage = TEXTURE_STATIC)", asMETHOD(TextureCube, SetSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("TextureCube", "bool Load(CubeMapFace, Image@+, bool useAlpha = false)", asFUNCTION(TextureCubeLoad), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("TextureCube", "RenderSurface@+ get_renderSurfaces(CubeMapFace) const", asMETHOD(TextureCube, GetRenderSurface), asCALL_THISCALL);
    
    // Register Variant GetPtr() for Texture. This is deprecated, GetPtr() should be used instead.
    engine->RegisterObjectMethod("Variant", "Texture@+ GetTexture(const String&in binding = \"deprecated:GetTexture\") const", asFUNCTION(GetVariantPtr<Texture>), asCALL_CDECL_OBJLAST);
    
    engine->RegisterGlobalFunction("uint GetAlphaFormat()", asFUNCTION(Graphics::GetAlphaFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetLuminanceFormat()", asFUNCTION(Graphics::GetLuminanceFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetLuminanceAlphaFormat()", asFUNCTION(Graphics::GetLuminanceAlphaFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGBFormat()", asFUNCTION(Graphics::GetRGBFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGBAFormat()", asFUNCTION(Graphics::GetRGBAFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGBA16Format()", asFUNCTION(Graphics::GetRGBA16Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGBAFloat16Format()", asFUNCTION(Graphics::GetRGBAFloat16Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGBAFloat32Format()", asFUNCTION(Graphics::GetRGBAFloat32Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRG16Format()", asFUNCTION(Graphics::GetRG16Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGFloat16Format()", asFUNCTION(Graphics::GetRGFloat16Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetRGFloat32Format()", asFUNCTION(Graphics::GetRGFloat32Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetFloat16Format()", asFUNCTION(Graphics::GetFloat16Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetFloat32Format()", asFUNCTION(Graphics::GetFloat32Format), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetDepthStencilFormat()", asFUNCTION(Graphics::GetDepthStencilFormat), asCALL_CDECL);
    engine->RegisterGlobalFunction("uint GetFormat(const String&in)", asFUNCTIONPR(Graphics::GetFormat, (const String&), unsigned), asCALL_CDECL);
}

static Material* MaterialClone(const String& cloneName, Material* ptr)
{
    SharedPtr<Material> clone = ptr->Clone(cloneName);
    // The shared pointer will go out of scope, so have to increment the reference count
    // (here an auto handle can not be used)
    clone->AddRef();
    return clone.Get();
}

static void ConstructBiasParameters(BiasParameters* ptr)
{
    new(ptr) BiasParameters(0.0f, 0.0f);
}

static void ConstructBiasParametersCopy(BiasParameters& parameters, BiasParameters* ptr)
{
    new(ptr) BiasParameters(parameters);
}

static void ConstructBiasParametersInit(float constantBias, float slopeScaledBias, BiasParameters* ptr)
{
    new(ptr) BiasParameters(constantBias, slopeScaledBias);
}

static void ConstructTechniqueEntry(TechniqueEntry* ptr)
{
    new(ptr) TechniqueEntry();
}

static void ConstructTechniqueEntryCopy(const TechniqueEntry& entry, TechniqueEntry* ptr)
{
    new(ptr) TechniqueEntry(entry);
}

static void DestructTechniqueEntry(TechniqueEntry* ptr)
{
    ptr->~TechniqueEntry();
}

static void TechniqueEntrySetTechnique(Technique* technique, TechniqueEntry* ptr)
{
    ptr->technique_ = technique;
}

static Technique* TechniqueEntryGetTechnique(TechniqueEntry* ptr)
{
    return ptr->technique_;
}

static CScriptArray* MaterialGetShaderParameterNames(Material* material)
{
    Vector<String> result;

    const HashMap<StringHash, MaterialShaderParameter>& parameters = material->GetShaderParameters();
    for (HashMap<StringHash, MaterialShaderParameter>::ConstIterator i = parameters.Begin(); i != parameters.End(); ++i)
        result.Push(i->second_.name_);

    Sort(result.Begin(), result.End());
    return VectorToArray<String>(result, "Array<String>");
}

static TechniqueEntry noTechniqueEntry;

static const TechniqueEntry& MaterialGetTechniqueEntry(unsigned index, Material* ptr)
{
    if (index >= ptr->GetNumTechniques())
    {
        asGetActiveContext()->SetException("Index out of bounds");
        return noTechniqueEntry;
    }
    
    return ptr->GetTechniqueEntry(index);
}

static void RegisterMaterial(asIScriptEngine* engine)
{
    engine->RegisterObjectType("BiasParameters", sizeof(BiasParameters), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("BiasParameters", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructBiasParameters), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("BiasParameters", asBEHAVE_CONSTRUCT, "void f(const BiasParameters&in)", asFUNCTION(ConstructBiasParametersCopy), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("BiasParameters", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(ConstructBiasParametersInit), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("BiasParameters", "float constantBias", offsetof(BiasParameters, constantBias_));
    engine->RegisterObjectProperty("BiasParameters", "float slopeScaledBias", offsetof(BiasParameters, slopeScaledBias_));
    
    engine->RegisterEnum("BlendMode");
    engine->RegisterEnumValue("BlendMode", "BLEND_REPLACE", BLEND_REPLACE);
    engine->RegisterEnumValue("BlendMode", "BLEND_ADD", BLEND_ADD);
    engine->RegisterEnumValue("BlendMode", "BLEND_MULTIPLY", BLEND_MULTIPLY);
    engine->RegisterEnumValue("BlendMode", "BLEND_ALPHA", BLEND_ALPHA);
    engine->RegisterEnumValue("BlendMode", "BLEND_ADDALPHA", BLEND_ADDALPHA);
    engine->RegisterEnumValue("BlendMode", "BLEND_PREMULALPHA", BLEND_PREMULALPHA);
    engine->RegisterEnumValue("BlendMode", "BLEND_INVDESTALPHA", BLEND_INVDESTALPHA);
    engine->RegisterEnumValue("BlendMode", "BLEND_SUBTRACT", BLEND_SUBTRACT);
    engine->RegisterEnumValue("BlendMode", "BLEND_SUBTRACTALPHA", BLEND_SUBTRACTALPHA);
    
    engine->RegisterEnum("CompareMode");
    engine->RegisterEnumValue("CompareMode", "CMP_ALWAYS", CMP_ALWAYS);
    engine->RegisterEnumValue("CompareMode", "CMP_EQUAL", CMP_EQUAL);
    engine->RegisterEnumValue("CompareMode", "CMP_NOTEQUAL", CMP_NOTEQUAL);
    engine->RegisterEnumValue("CompareMode", "CMP_LESS", CMP_LESS);
    engine->RegisterEnumValue("CompareMode", "CMP_LESSEQUAL", CMP_LESSEQUAL);
    engine->RegisterEnumValue("CompareMode", "CMP_GREATER", CMP_GREATER);
    engine->RegisterEnumValue("CompareMode", "CMP_GREATEREQUAL", CMP_GREATEREQUAL);
    
    engine->RegisterEnum("CullMode");
    engine->RegisterEnumValue("CullMode", "CULL_NONE", CULL_NONE);
    engine->RegisterEnumValue("CullMode", "CULL_CCW", CULL_CCW);
    engine->RegisterEnumValue("CullMode", "CULL_CW", CULL_CW);
    
    engine->RegisterEnum("PassLightingMode");
    engine->RegisterEnumValue("PassLightingMode", "LIGHTING_UNLIT", LIGHTING_UNLIT);
    engine->RegisterEnumValue("PassLightingMode", "LIGHTING_PERVERTEX", LIGHTING_PERVERTEX);
    engine->RegisterEnumValue("PassLightingMode", "LIGHTING_PERPIXEL", LIGHTING_PERPIXEL);
    
    RegisterRefCounted<Pass>(engine, "Pass");
    engine->RegisterObjectMethod("Pass", "void set_blendMode(BlendMode)", asMETHOD(Pass, SetBlendMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "BlendMode get_blendMode() const", asMETHOD(Pass, GetBlendMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_depthTestMode(CompareMode)", asMETHOD(Pass, SetDepthTestMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "CompareMode get_depthTestMode() const", asMETHOD(Pass, GetDepthTestMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_lightingMode(PassLightingMode)", asMETHOD(Pass, SetLightingMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "PassLightingMode get_lightingMode() const", asMETHOD(Pass, GetLightingMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_depthWrite(bool)", asMETHOD(Pass, SetDepthWrite), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "bool get_depthWrite() const", asMETHOD(Pass, GetDepthWrite), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_alphaMask(bool)", asMETHOD(Pass, SetAlphaMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "bool get_alphaMask() const", asMETHOD(Pass, GetAlphaMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_sm3(bool)", asMETHOD(Technique, SetIsSM3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "bool get_sm3() const", asMETHOD(Technique, IsSM3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_vertexShader(const String&in)", asMETHOD(Pass, SetVertexShader), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "const String& get_vertexShader() const", asMETHOD(Pass, GetVertexShader), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_pixelShader(const String&in)", asMETHOD(Pass, SetPixelShader), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "const String& get_pixelShader() const", asMETHOD(Pass, GetPixelShader), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_vertexShaderDefines(const String&in)", asMETHOD(Pass, SetVertexShaderDefines), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "const String& get_vertexShaderDefines() const", asMETHOD(Pass, GetVertexShaderDefines), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "void set_pixelShaderDefines(const String&in)", asMETHOD(Pass, SetPixelShaderDefines), asCALL_THISCALL);
    engine->RegisterObjectMethod("Pass", "const String& get_pixelShaderDefines() const", asMETHOD(Pass, GetPixelShaderDefines), asCALL_THISCALL);
    
    RegisterResource<Technique>(engine, "Technique");
    engine->RegisterObjectMethod("Technique", "Pass@+ CreatePass(StringHash)", asMETHOD(Technique, CreatePass), asCALL_THISCALL);
    engine->RegisterObjectMethod("Technique", "void RemovePass(StringHash)", asMETHOD(Technique, RemovePass), asCALL_THISCALL);
    engine->RegisterObjectMethod("Technique", "bool HasPass(StringHash) const", asMETHOD(Technique, HasPass), asCALL_THISCALL);
    engine->RegisterObjectMethod("Technique", "void set_sm3(bool)", asMETHOD(Technique, SetIsSM3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Technique", "bool get_sm3() const", asMETHOD(Technique, IsSM3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Technique", "Pass@+ get_passes(StringHash)", asMETHOD(Technique, GetPass), asCALL_THISCALL);
    
    engine->RegisterObjectType("TechniqueEntry", sizeof(TechniqueEntry), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
    engine->RegisterObjectBehaviour("TechniqueEntry", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructTechniqueEntry), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("TechniqueEntry", asBEHAVE_CONSTRUCT, "void f(const TechniqueEntry&in)", asFUNCTION(ConstructTechniqueEntry), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("TechniqueEntry", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructTechniqueEntry), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("TechniqueEntry", "TechniqueEntry& opAssign(const TechniqueEntry&in)", asMETHOD(TechniqueEntry, operator =), asCALL_THISCALL);
    engine->RegisterObjectMethod("TechniqueEntry", "void set_technique(Technique@+)", asFUNCTION(TechniqueEntrySetTechnique), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("TechniqueEntry", "Technique@+ get_technique() const", asFUNCTION(TechniqueEntryGetTechnique), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("TechniqueEntry", "int qualityLevel", offsetof(TechniqueEntry, qualityLevel_));
    engine->RegisterObjectProperty("TechniqueEntry", "float lodDistance", offsetof(TechniqueEntry, lodDistance_));

    RegisterResource<Material>(engine, "Material");
    engine->RegisterObjectMethod("Material", "bool Load(const XMLElement&in)", asMETHODPR(Material, Load, (const XMLElement&), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "bool Save(XMLElement&in) const", asMETHODPR(Material, Save, (XMLElement&) const, bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void SetTechnique(uint, Technique@+, uint qualityLevel = 0, float lodDistance = 0.0)", asMETHOD(Material, SetTechnique), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void SetUVTransform(const Vector2&in, float, const Vector2&in)", asMETHODPR(Material, SetUVTransform, (const Vector2&, float, const Vector2&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void SetUVTransform(const Vector2&in, float, float)", asMETHODPR(Material, SetUVTransform, (const Vector2&, float, float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void RemoveShaderParameter(const String&in)", asMETHOD(Material, RemoveShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void SortTechniques()", asMETHOD(Material, SortTechniques), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "Material@ Clone(const String&in cloneName = String()) const", asFUNCTION(MaterialClone), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Material", "void set_numTechniques(uint)", asMETHOD(Material, SetNumTechniques), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "uint get_numTechniques() const", asMETHOD(Material, GetNumTechniques), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "Technique@+ get_techniques(uint)", asMETHOD(Material, GetTechnique), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "const TechniqueEntry& get_techniqueEntries(uint) const", asFUNCTION(MaterialGetTechniqueEntry), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Material", "void set_shaderParameters(const String&in, const Variant&in)", asMETHOD(Material, SetShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "const Variant& get_shaderParameters(const String&in) const", asMETHOD(Material, GetShaderParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "Array<String>@ get_shaderParameterNames() const", asFUNCTION(MaterialGetShaderParameterNames), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Material", "void set_textures(uint, Texture@+)", asMETHOD(Material, SetTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "Texture@+ get_textures(uint) const", asMETHOD(Material, GetTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "bool get_occlusion()", asMETHOD(Material, GetOcclusion), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void set_cullMode(CullMode)", asMETHOD(Material, SetCullMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "CullMode get_cullMode() const", asMETHOD(Material, GetCullMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void set_shadowCullMode(CullMode)", asMETHOD(Material, SetShadowCullMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "CullMode get_shadowCullMode() const", asMETHOD(Material, GetShadowCullMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "void set_depthBias(const BiasParameters&in)", asMETHOD(Material, SetDepthBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Material", "const BiasParameters& get_depthBias() const", asMETHOD(Material, GetDepthBias), asCALL_THISCALL);
    
    engine->RegisterGlobalFunction("String GetTextureUnitName(TextureUnit)", asFUNCTION(Material::GetTextureUnitName), asCALL_CDECL);
}

static void RegisterModel(asIScriptEngine* engine)
{
    RegisterResource<Model>(engine, "Model");
    engine->RegisterObjectMethod("Model", "const BoundingBox& get_boundingBox() const", asMETHOD(Model, GetBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("Model", "Skeleton@+ get_skeleton()", asMETHOD(Model, GetSkeleton), asCALL_THISCALL);
    engine->RegisterObjectMethod("Model", "uint get_numGeometries() const", asMETHOD(Model, GetNumGeometries), asCALL_THISCALL);
    engine->RegisterObjectMethod("Model", "uint get_numGeometryLodLevels(uint) const", asMETHOD(Model, GetNumGeometryLodLevels), asCALL_THISCALL);
    engine->RegisterObjectMethod("Model", "uint get_numMorphs() const", asMETHOD(Model, GetNumMorphs), asCALL_THISCALL);
}

static AnimationTriggerPoint* AnimationGetTrigger(unsigned index, Animation* animation)
{
    const Vector<AnimationTriggerPoint>& points = animation->GetTriggers();
    return index < points.Size() ? const_cast<AnimationTriggerPoint*>(&points[index]) : (AnimationTriggerPoint*)0;
}

static void RegisterAnimation(asIScriptEngine* engine)
{
    engine->RegisterObjectType("AnimationTriggerPoint", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("AnimationTriggerPoint", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("AnimationTriggerPoint", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("AnimationTriggerPoint", "float time", offsetof(AnimationTriggerPoint, time_));
    engine->RegisterObjectProperty("AnimationTriggerPoint", "Variant data", offsetof(AnimationTriggerPoint, data_));
    
    RegisterResource<Animation>(engine, "Animation");
    engine->RegisterObjectMethod("Animation", "const String& get_animationName() const", asMETHOD(Animation, GetAnimationName), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "void AddTrigger(float, bool, const Variant&in)", asMETHOD(Animation, AddTrigger), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "void RemoveTrigger(uint)", asMETHOD(Animation, RemoveTrigger), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "void RemoveAllTriggers()", asMETHOD(Animation, RemoveAllTriggers), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "float get_length() const", asMETHOD(Animation, GetLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "uint get_numTracks() const", asMETHOD(Animation, GetNumTracks), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "void set_numTriggers(uint)", asMETHOD(Animation, SetNumTriggers), asCALL_THISCALL);
    engine->RegisterObjectMethod("Animation", "AnimationTriggerPoint@+ get_triggers(uint) const", asFUNCTION(AnimationGetTrigger), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Animation", "uint get_numTriggers() const", asMETHOD(Animation, GetNumTriggers), asCALL_THISCALL);
}

static void RegisterDrawable(asIScriptEngine* engine)
{
    engine->RegisterGlobalProperty("uint DRAWABLE_GEOMETRY", (void*)&DRAWABLE_GEOMETRY);
    engine->RegisterGlobalProperty("uint DRAWABLE_LIGHT", (void*)&DRAWABLE_LIGHT);
    engine->RegisterGlobalProperty("uint DRAWABLE_ZONE", (void*)&DRAWABLE_ZONE);
    engine->RegisterGlobalProperty("uint DRAWABLE_ANY", (void*)&DRAWABLE_ANY);
    engine->RegisterGlobalProperty("uint DEFAULT_VIEWMASK", (void*)&DEFAULT_VIEWMASK);
    engine->RegisterGlobalProperty("uint DEFAULT_LIGHTMASK", (void*)&DEFAULT_LIGHTMASK);
    
    RegisterDrawable<Drawable>(engine, "Drawable");
}

static void ConstructCascadeParameters(CascadeParameters* ptr)
{
    new(ptr) CascadeParameters(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

static void ConstructCascadeParametersCopy(CascadeParameters& parameters, CascadeParameters* ptr)
{
    new(ptr) CascadeParameters(parameters);
}

static void ConstructCascadeParametersInit(float split1, float split2, float split3, float split4, float fadeStart, float biasAutoAdjust, CascadeParameters* ptr)
{
    new(ptr) CascadeParameters(split1, split2, split3, split4, fadeStart, biasAutoAdjust);
}

static void ConstructFocusParameters(FocusParameters* ptr)
{
    new(ptr) FocusParameters(false, false, false, 0.0f, 0.0f);
}

static void ConstructFocusParametersCopy(FocusParameters& parameters, FocusParameters* ptr)
{
    new(ptr) FocusParameters(parameters);
}

static void ConstructFocusParametersInit(bool focus, bool nonUniform, bool autoSize, float quantize, float minView, FocusParameters* ptr)
{
    new(ptr) FocusParameters(focus, nonUniform, autoSize, quantize, minView);
}

static void RegisterLight(asIScriptEngine* engine)
{
    engine->RegisterEnum("LightType");
    engine->RegisterEnumValue("LightType", "LIGHT_DIRECTIONAL", LIGHT_DIRECTIONAL);
    engine->RegisterEnumValue("LightType", "LIGHT_SPOT", LIGHT_SPOT);
    engine->RegisterEnumValue("LightType", "LIGHT_POINT", LIGHT_POINT);
    
    engine->RegisterObjectType("CascadeParameters", sizeof(CascadeParameters), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("CascadeParameters", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructCascadeParameters), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("CascadeParameters", asBEHAVE_CONSTRUCT, "void f(const CascadeParameters&in)", asFUNCTION(ConstructCascadeParametersCopy), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("CascadeParameters", asBEHAVE_CONSTRUCT, "void f(float, float, float, float, float, float biasAutoAdjust = 1.0)", asFUNCTION(ConstructCascadeParametersInit), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("CascadeParameters", "float split1", offsetof(CascadeParameters, splits_[0]));
    engine->RegisterObjectProperty("CascadeParameters", "float split2", offsetof(CascadeParameters, splits_[1]));
    engine->RegisterObjectProperty("CascadeParameters", "float split3", offsetof(CascadeParameters, splits_[2]));
    engine->RegisterObjectProperty("CascadeParameters", "float split4", offsetof(CascadeParameters, splits_[3]));
    engine->RegisterObjectProperty("CascadeParameters", "float fadeStart", offsetof(CascadeParameters, fadeStart_));
    engine->RegisterObjectProperty("CascadeParameters", "float biasAutoAdjust", offsetof(CascadeParameters, biasAutoAdjust_));
    
    engine->RegisterObjectType("FocusParameters", sizeof(FocusParameters), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("FocusParameters", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructFocusParameters), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("FocusParameters", asBEHAVE_CONSTRUCT, "void f(const FocusParameters&in)", asFUNCTION(ConstructFocusParametersCopy), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("FocusParameters", asBEHAVE_CONSTRUCT, "void f(bool, bool, bool, float, float)", asFUNCTION(ConstructFocusParametersInit), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("FocusParameters", "bool focus", offsetof(FocusParameters, focus_));
    engine->RegisterObjectProperty("FocusParameters", "bool nonUniform", offsetof(FocusParameters, nonUniform_));
    engine->RegisterObjectProperty("FocusParameters", "bool autoSize", offsetof(FocusParameters, autoSize_));
    engine->RegisterObjectProperty("FocusParameters", "float quantize", offsetof(FocusParameters, quantize_));
    engine->RegisterObjectProperty("FocusParameters", "float minView", offsetof(FocusParameters, minView_));
    
    RegisterDrawable<Light>(engine, "Light");
    engine->RegisterObjectMethod("Light", "void set_lightType(LightType)", asMETHOD(Light, SetLightType), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "LightType get_lightType() const", asMETHOD(Light, GetLightType), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_perVertex(bool)", asMETHOD(Light, SetPerVertex), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "bool get_perVertex() const", asMETHOD(Light, GetPerVertex), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_color(const Color&in)", asMETHOD(Light, SetColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "const Color& get_color() const", asMETHOD(Light, GetColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_specularIntensity(float)", asMETHOD(Light, SetSpecularIntensity), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_specularIntensity() const", asMETHOD(Light, GetSpecularIntensity), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_brightness(float)", asMETHOD(Light, SetBrightness), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_brightness() const", asMETHOD(Light, GetBrightness), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_range(float)", asMETHOD(Light, SetRange), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_range() const", asMETHOD(Light, GetRange), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_fov(float)", asMETHOD(Light, SetFov), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_fov() const", asMETHOD(Light, GetFov), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_aspectRatio(float)", asMETHOD(Light, SetAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_aspectRatio() const", asMETHOD(Light, GetAspectRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_fadeDistance(float)", asMETHOD(Light, SetFadeDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_fadeDistance() const", asMETHOD(Light, GetFadeDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowBias(const BiasParameters&in)", asMETHOD(Light, SetShadowBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "const BiasParameters& get_shadowBias() const", asMETHOD(Light, GetShadowBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowCascade(const CascadeParameters&in)", asMETHOD(Light, SetShadowCascade), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "const CascadeParameters& get_shadowCascade() const", asMETHOD(Light, GetShadowCascade), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowFocus(const FocusParameters&in)", asMETHOD(Light, SetShadowFocus), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "const FocusParameters& get_shadowFocus() const", asMETHOD(Light, GetShadowFocus), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowFadeDistance(float)", asMETHOD(Light, SetShadowFadeDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_shadowFadeDistance() const", asMETHOD(Light, GetShadowFadeDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowIntensity(float)", asMETHOD(Light, SetShadowIntensity), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_shadowIntensity() const", asMETHOD(Light, GetShadowIntensity), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowResolution(float)", asMETHOD(Light, SetShadowResolution), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_shadowResolution() const", asMETHOD(Light, GetShadowResolution), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shadowNearFarRatio(float)", asMETHOD(Light, SetShadowNearFarRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_shadowNearFarRatio() const", asMETHOD(Light, GetShadowNearFarRatio), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_rampTexture(Texture@+)", asMETHOD(Light, SetRampTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "Texture@+ get_rampTexture() const", asMETHOD(Light, GetRampTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "void set_shapeTexture(Texture@+)", asMETHOD(Light, SetShapeTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "Texture@+ get_shapeTexture() const", asMETHOD(Light, GetShapeTexture), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "Frustum get_frustum() const", asMETHOD(Light, GetFrustum), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "int get_numShadowSplits() const", asMETHOD(Light, GetNumShadowSplits), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "bool get_negative() const", asMETHOD(Light, IsNegative), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "Color get_effectiveColor() const", asMETHOD(Light, GetEffectiveColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Light", "float get_effectiveSpecularIntensity() const", asMETHOD(Light, GetEffectiveSpecularIntensity), asCALL_THISCALL);
}

static void RegisterZone(asIScriptEngine* engine)
{
    RegisterDrawable<Zone>(engine, "Zone");
    engine->RegisterObjectMethod("Zone", "void set_boundingBox(const BoundingBox&in)", asMETHOD(Zone, SetBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "const Matrix3x4& get_inverseWorldTransform() const", asMETHOD(Zone, GetInverseWorldTransform), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_ambientColor(const Color&in)", asMETHOD(Zone, SetAmbientColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "const Color& get_ambientColor() const", asMETHOD(Zone, GetAmbientColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "const Color& get_ambientStartColor()", asMETHOD(Zone, GetAmbientStartColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "const Color& get_ambientEndColor()", asMETHOD(Zone, GetAmbientEndColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_fogColor(const Color&in)", asMETHOD(Zone, SetFogColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "const Color& get_fogColor() const", asMETHOD(Zone, GetFogColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_fogStart(float)", asMETHOD(Zone, SetFogStart), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "float get_fogStart() const", asMETHOD(Zone, GetFogStart), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_fogEnd(float)", asMETHOD(Zone, SetFogEnd), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "float get_fogEnd() const", asMETHOD(Zone, GetFogEnd), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_fogHeight(float)", asMETHOD(Zone, SetFogHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "float get_fogHeight() const", asMETHOD(Zone, GetFogHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_fogHeightScale(float)", asMETHOD(Zone, SetFogHeightScale), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "float get_fogHeightScale() const", asMETHOD(Zone, GetFogHeightScale), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_priority(int)", asMETHOD(Zone, SetPriority), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "int get_priority() const", asMETHOD(Zone, GetPriority), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_heightFog(bool)", asMETHOD(Zone, SetHeightFog), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "bool get_heightFog() const", asMETHOD(Zone, GetHeightFog), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_override(bool)", asMETHOD(Zone, SetOverride), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "bool get_override() const", asMETHOD(Zone, GetOverride), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "void set_ambientGradient(bool)", asMETHOD(Zone, SetAmbientGradient), asCALL_THISCALL);
    engine->RegisterObjectMethod("Zone", "bool get_ambientGradient() const", asMETHOD(Zone, GetAmbientGradient), asCALL_THISCALL);
}

static void RegisterStaticModel(asIScriptEngine* engine)
{
    RegisterDrawable<StaticModel>(engine, "StaticModel");
    engine->RegisterObjectMethod("StaticModel", "void ApplyMaterialList(const String&in fileName = String())", asMETHOD(StaticModel, ApplyMaterialList), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "bool IsInside(const Vector3&in) const", asMETHOD(StaticModel, IsInside), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "bool IsInsideLocal(const Vector3&in) const", asMETHOD(StaticModel, IsInsideLocal), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "void set_model(Model@+)", asMETHOD(StaticModel, SetModel), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "Model@+ get_model() const", asMETHOD(StaticModel, GetModel), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "void set_material(Material@+)", asMETHODPR(StaticModel, SetMaterial, (Material*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "bool set_materials(uint, Material@+)", asMETHODPR(StaticModel, SetMaterial, (unsigned, Material*), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "Material@+ get_materials(uint) const", asMETHOD(StaticModel, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "uint get_numGeometries() const", asMETHOD(StaticModel, GetNumGeometries), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "void set_occlusionLodLevel(uint) const", asMETHOD(StaticModel, SetOcclusionLodLevel), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModel", "uint get_occlusionLodLevel() const", asMETHOD(StaticModel, GetOcclusionLodLevel), asCALL_THISCALL);
}

static void RegisterStaticModelGroup(asIScriptEngine* engine)
{
    RegisterStaticModel<StaticModelGroup>(engine, "StaticModelGroup", true);
    engine->RegisterObjectMethod("StaticModelGroup", "void set_occlusionLodLevel(uint) const", asMETHOD(StaticModelGroup, SetOcclusionLodLevel), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "uint get_occlusionLodLevel() const", asMETHOD(StaticModelGroup, GetOcclusionLodLevel), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "void AddInstanceNode(Node@+)", asMETHOD(StaticModelGroup, AddInstanceNode), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "void RemoveInstanceNode(Node@+)", asMETHOD(StaticModelGroup, RemoveInstanceNode), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "void RemoveAllInstanceNodes()", asMETHOD(StaticModelGroup, RemoveAllInstanceNodes), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "uint get_numInstanceNodes() const", asMETHOD(StaticModelGroup, GetNumInstanceNodes), asCALL_THISCALL);
    engine->RegisterObjectMethod("StaticModelGroup", "Node@+ get_instanceNodes(uint) const", asMETHOD(StaticModelGroup, GetInstanceNode), asCALL_THISCALL);
}

static void RegisterSkybox(asIScriptEngine* engine)
{
    RegisterStaticModel<Skybox>(engine, "Skybox", true);
}

static void AnimatedModelSetModel(Model* model, AnimatedModel* ptr)
{
    ptr->SetModel(model);
}

static const String& AnimatedModelGetMorphName(unsigned index, AnimatedModel* ptr)
{
    const Vector<ModelMorph>& morphs = ptr->GetMorphs();
    return index < morphs.Size() ? morphs[index].name_ : String::EMPTY;
}

static AnimationState* ConstructAnimationState(Node* node, Animation* animation)
{
    return new AnimationState(node, animation);
}

static void AnimationStateSetBoneWeight(const String& name, float weight, AnimationState* ptr)
{
    ptr->SetBoneWeight(name, weight);
}

static void RegisterAnimatedModel(asIScriptEngine* engine)
{
    RegisterRefCounted<AnimationState>(engine, "AnimationState");
    RegisterStaticModel<AnimatedModel>(engine, "AnimatedModel", false);
    
    engine->RegisterObjectBehaviour("AnimationState", asBEHAVE_FACTORY, "AnimationState@+ f(Node@+, Animation@+)", asFUNCTION(ConstructAnimationState), asCALL_CDECL);
    engine->RegisterObjectMethod("AnimationState", "void AddWeight(float)", asMETHOD(AnimationState, AddWeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void AddTime(float)", asMETHOD(AnimationState, AddTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void Apply()", asMETHOD(AnimationState, Apply), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void SetBoneWeight(uint, float, bool recursive = false)", asMETHODPR(AnimationState, SetBoneWeight, (unsigned, float, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void SetBoneWeight(const String&in, float, bool recursive = false)", asMETHODPR(AnimationState, SetBoneWeight, (const String&, float, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void SetBoneWeight(StringHash, float, bool recursive = false)", asMETHODPR(AnimationState, SetBoneWeight, (StringHash, float, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "float GetBoneWeight(uint) const", asMETHODPR(AnimationState, GetBoneWeight, (unsigned) const, float), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "float GetBoneWeight(StringHash) const", asMETHODPR(AnimationState, GetBoneWeight, (StringHash) const, float), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "uint GetTrackIndex(const String&in) const", asMETHODPR(AnimationState, GetTrackIndex, (const String&) const, unsigned), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "uint GetTrackIndex(StringHash) const", asMETHODPR(AnimationState, GetTrackIndex, (StringHash) const, unsigned), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_startBone(Bone@+)", asMETHOD(AnimationState, SetStartBone), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "Bone@+ get_startBone() const", asMETHOD(AnimationState, GetStartBone), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_looped(bool)", asMETHOD(AnimationState, SetLooped), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "bool get_looped() const", asMETHOD(AnimationState, IsLooped), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_weight(float)", asMETHOD(AnimationState, SetWeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "float get_weight() const", asMETHOD(AnimationState, GetWeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_time(float)", asMETHOD(AnimationState, SetTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "float get_time() const", asMETHOD(AnimationState, GetTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_layer(uint8)", asMETHOD(AnimationState, SetLayer), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "uint8 get_layer() const", asMETHOD(AnimationState, GetLayer), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "Animation@+ get_animation() const", asMETHOD(AnimationState, GetAnimation), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "AnimatedModel@+ get_model() const", asMETHOD(AnimationState, GetModel), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "Node@+ get_node() const", asMETHOD(AnimationState, GetNode), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "bool get_enabled() const", asMETHOD(AnimationState, IsEnabled), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "float get_length() const", asMETHOD(AnimationState, GetLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationState", "void set_boneWeights(const String&in, float)", asFUNCTION(AnimationStateSetBoneWeight), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("AnimationState", "float get_boneWeights(const String&in)", asMETHODPR(AnimationState, GetBoneWeight, (const String&) const, float), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "AnimationState@+ AddAnimationState(Animation@+)", asMETHOD(AnimatedModel, AddAnimationState), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void RemoveAnimationState(Animation@+)", asMETHODPR(AnimatedModel, RemoveAnimationState, (Animation*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void RemoveAnimationState(const String&in)", asMETHODPR(AnimatedModel, RemoveAnimationState, (const String&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void RemoveAnimationState(AnimationState@+)", asMETHODPR(AnimatedModel, RemoveAnimationState, (AnimationState*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void RemoveAnimationState(uint)", asMETHODPR(AnimatedModel, RemoveAnimationState, (unsigned), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void RemoveAllAnimationStates()", asMETHOD(AnimatedModel, RemoveAllAnimationStates), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void SetMorphWeight(uint, float)", asMETHODPR(AnimatedModel, SetMorphWeight, (unsigned, float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void ResetMorphWeights()", asMETHOD(AnimatedModel, ResetMorphWeights), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "float GetMorphWeight(uint) const", asMETHODPR(AnimatedModel, GetMorphWeight, (unsigned) const, float), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "AnimationState@+ GetAnimationState(Animation@+) const", asMETHODPR(AnimatedModel, GetAnimationState, (Animation*) const, AnimationState*), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "AnimationState@+ GetAnimationState(uint) const", asMETHODPR(AnimatedModel, GetAnimationState, (unsigned) const, AnimationState*), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void set_model(Model@+)", asFUNCTION(AnimatedModelSetModel), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("AnimatedModel", "void set_animationLodBias(float)", asMETHOD(AnimatedModel, SetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "float get_animationLodBias() const", asMETHOD(AnimatedModel, GetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "void set_updateInvisible(bool)", asMETHOD(AnimatedModel, SetUpdateInvisible), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "bool get_updateInvisible() const", asMETHOD(AnimatedModel, GetUpdateInvisible), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "Skeleton@+ get_skeleton()", asMETHOD(AnimatedModel, GetSkeleton), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "uint get_numAnimationStates() const", asMETHOD(AnimatedModel, GetNumAnimationStates), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "AnimationState@+ get_animationStates(const String&in) const", asMETHODPR(AnimatedModel, GetAnimationState, (const String&) const, AnimationState*), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "uint get_numMorphs() const", asMETHOD(AnimatedModel, GetNumMorphs), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "const String& get_morphNames(uint) const", asFUNCTION(AnimatedModelGetMorphName), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("AnimatedModel", "void set_morphWeights(const String&in, float)", asMETHODPR(AnimatedModel, SetMorphWeight, (const String&, float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimatedModel", "float get_morphWeights(const String&in) const", asMETHODPR(AnimatedModel, GetMorphWeight, (const String&) const, float), asCALL_THISCALL);
}

static void RegisterAnimationController(asIScriptEngine* engine)
{
    RegisterComponent<AnimationController>(engine, "AnimationController");
    engine->RegisterObjectMethod("AnimationController", "bool Play(const String&in, uint8, bool, float fadeTime = 0.0f)", asMETHOD(AnimationController, Play), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool PlayExclusive(const String&in, uint8, bool, float fadeTime = 0.0f)", asMETHOD(AnimationController, PlayExclusive), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "void Stop(const String&in, float fadeTime = 0.0f)", asMETHOD(AnimationController, Stop), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "void StopLayer(uint8, float fadeTime = 0.0f)", asMETHOD(AnimationController, StopLayer), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "void StopAll(float fadeTime = 0.0f)", asMETHOD(AnimationController, StopAll), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool Fade(const String&in, float, float)", asMETHOD(AnimationController, Fade), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool FadeOthers(const String&in, float, float)", asMETHOD(AnimationController, FadeOthers), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetLayer(const String&in, uint8)", asMETHOD(AnimationController, SetLayer), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetStartBone(const String&in, const String&in)", asMETHOD(AnimationController, SetStartBone), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetTime(const String&in, float)", asMETHOD(AnimationController, SetTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetWeight(const String&in, float)", asMETHOD(AnimationController, SetWeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetLooped(const String&in, bool)", asMETHOD(AnimationController, SetLooped), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetSpeed(const String&in, float)", asMETHOD(AnimationController, SetSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool SetAutoFade(const String&in, float)", asMETHOD(AnimationController, SetAutoFade), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool IsPlaying(const String&in) const", asMETHOD(AnimationController, IsPlaying), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool IsFadingIn(const String&in) const", asMETHOD(AnimationController, IsFadingIn), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool IsFadingOut(const String&in) const", asMETHOD(AnimationController, IsFadingOut), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "uint8 GetLayer(const String&in) const", asMETHOD(AnimationController, GetLayer), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "const String& GetStartBone(const String&in) const", asMETHOD(AnimationController, GetStartBoneName), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetTime(const String&in) const", asMETHOD(AnimationController, GetTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetWeight(const String&in) const", asMETHOD(AnimationController, GetWeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "bool GetLooped(const String&in) const", asMETHOD(AnimationController, IsLooped), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetLength(const String&in) const", asMETHOD(AnimationController, GetLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetSpeed(const String&in) const", asMETHOD(AnimationController, GetSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetAutoFade(const String&in) const", asMETHOD(AnimationController, GetAutoFade), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "float GetFadeTarget(const String&in) const", asMETHOD(AnimationController, GetFadeTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "AnimationState@+ GetAnimationState(const String&in) const", asMETHODPR(AnimationController, GetAnimationState, (const String&) const, AnimationState*), asCALL_THISCALL);
    engine->RegisterObjectMethod("AnimationController", "AnimationState@+ GetAnimationState(StringHash) const", asMETHODPR(AnimationController, GetAnimationState, (StringHash) const, AnimationState*), asCALL_THISCALL);
}

static void RegisterBillboardSet(asIScriptEngine* engine)
{
    engine->RegisterObjectType("Billboard", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("Billboard", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Billboard", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("Billboard", "Vector3 position", offsetof(Billboard, position_));
    engine->RegisterObjectProperty("Billboard", "Vector2 size", offsetof(Billboard, size_));
    engine->RegisterObjectProperty("Billboard", "Rect uv", offsetof(Billboard, uv_));
    engine->RegisterObjectProperty("Billboard", "Color color", offsetof(Billboard, color_));
    engine->RegisterObjectProperty("Billboard", "float rotation", offsetof(Billboard, rotation_));
    engine->RegisterObjectProperty("Billboard", "bool enabled", offsetof(Billboard, enabled_));
    
    RegisterDrawable<BillboardSet>(engine, "BillboardSet");
    engine->RegisterObjectMethod("BillboardSet", "void Commit()", asMETHOD(BillboardSet, Commit), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_material(Material@+)", asMETHOD(BillboardSet, SetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "Material@+ get_material() const", asMETHOD(BillboardSet, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_numBillboards(uint)", asMETHOD(BillboardSet, SetNumBillboards), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "uint get_numBillboards() const", asMETHOD(BillboardSet, GetNumBillboards), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_relative(bool)", asMETHOD(BillboardSet, SetRelative), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "bool get_relative() const", asMETHOD(BillboardSet, IsRelative), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_sorted(bool)", asMETHOD(BillboardSet, SetSorted), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "bool get_sorted() const", asMETHOD(BillboardSet, IsSorted), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_scaled(bool)", asMETHOD(BillboardSet, SetScaled), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "bool get_scaled() const", asMETHOD(BillboardSet, IsScaled), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_faceCamera(bool)", asMETHOD(BillboardSet, SetFaceCamera), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "bool get_faceCamera() const", asMETHOD(BillboardSet, GetFaceCamera), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "void set_animationLodBias(float)", asMETHOD(BillboardSet, SetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "float get_animationLodBias() const", asMETHOD(BillboardSet, GetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "Billboard@+ get_billboards(uint)", asMETHOD(BillboardSet, GetBillboard), asCALL_THISCALL);
    engine->RegisterObjectMethod("BillboardSet", "Zone@+ get_zone() const", asMETHOD(BillboardSet, GetZone), asCALL_THISCALL);
}

void ParticleEmitterSetEmitting(bool enable, ParticleEmitter* ptr)
{
    ptr->SetEmitting(enable);
}

static void RegisterParticleEmitter(asIScriptEngine* engine)
{
    engine->RegisterEnum("EmitterType");
    engine->RegisterEnumValue("EmitterType", "EMITTER_SPHERE", EMITTER_SPHERE);
    engine->RegisterEnumValue("EmitterType", "EMITTER_BOX", EMITTER_BOX);
    
    engine->RegisterObjectType("ColorFrame", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("ColorFrame", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("ColorFrame", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("ColorFrame", "Color color", offsetof(ColorFrame, color_));
    engine->RegisterObjectProperty("ColorFrame", "float time", offsetof(ColorFrame, time_));
    
    engine->RegisterObjectType("TextureFrame", 0, asOBJ_REF);
    engine->RegisterObjectBehaviour("TextureFrame", asBEHAVE_ADDREF, "void f()", asFUNCTION(FakeAddRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("TextureFrame", asBEHAVE_RELEASE, "void f()", asFUNCTION(FakeReleaseRef), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("TextureFrame", "Rect uv", offsetof(TextureFrame, uv_));
    engine->RegisterObjectProperty("TextureFrame", "float time", offsetof(TextureFrame, time_));
    
    RegisterDrawable<ParticleEmitter>(engine, "ParticleEmitter");
    engine->RegisterObjectMethod("ParticleEmitter", "bool Load(XMLFile@+)", asMETHODPR(ParticleEmitter, Load, (XMLFile*), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "bool Save(XMLFile@+)", asMETHODPR(ParticleEmitter, Save, (XMLFile*) const, bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void SetEmitting(bool, bool)", asMETHOD(ParticleEmitter, SetEmitting), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void SetColor(const Color&in)", asMETHOD(ParticleEmitter, SetColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_material(Material@+)", asMETHOD(ParticleEmitter, SetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "Material@+ get_material() const", asMETHOD(ParticleEmitter, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_relative(bool)", asMETHOD(ParticleEmitter, SetRelative), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "bool get_relative() const", asMETHOD(ParticleEmitter, IsRelative), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_sorted(bool)", asMETHOD(ParticleEmitter, SetSorted), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "bool get_sorted() const", asMETHOD(ParticleEmitter, IsSorted), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_scaled(bool)", asMETHOD(ParticleEmitter, SetScaled), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "bool get_scaled() const", asMETHOD(ParticleEmitter, IsScaled), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_updateInvisible(bool)", asMETHOD(ParticleEmitter, SetUpdateInvisible), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "bool get_updateInvisible() const", asMETHOD(ParticleEmitter, GetUpdateInvisible), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_animationLodBias(float)", asMETHOD(ParticleEmitter, SetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_animationLodBias() const", asMETHOD(ParticleEmitter, GetAnimationLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_emitting() const", asFUNCTION(ParticleEmitterSetEmitting), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("ParticleEmitter", "bool get_emitting() const", asMETHOD(ParticleEmitter, IsEmitting), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_numParticles(uint) const", asMETHOD(ParticleEmitter, SetNumParticles), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "uint get_numParticles() const", asMETHOD(ParticleEmitter, GetNumParticles), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_emissionRate(float)", asMETHOD(ParticleEmitter, SetEmissionRate), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minEmissionRate(float)", asMETHOD(ParticleEmitter, SetMinEmissionRate), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxEmissionRate(float)", asMETHOD(ParticleEmitter, SetMaxEmissionRate), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_minEmissionRate() const", asMETHOD(ParticleEmitter, GetMinEmissionRate), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_maxEmissionRate() const", asMETHOD(ParticleEmitter, GetMaxEmissionRate), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_emitterType(EmitterType)", asMETHOD(ParticleEmitter, SetEmitterType), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "EmitterType get_emitterType() const", asMETHOD(ParticleEmitter, GetEmitterType), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_emitterSize(const Vector3&in)", asMETHOD(ParticleEmitter, SetEmitterSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector3& get_emitterSize() const", asMETHOD(ParticleEmitter, GetEmitterSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_activeTime(float) const", asMETHOD(ParticleEmitter, SetActiveTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_activeTime() const", asMETHOD(ParticleEmitter, GetActiveTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_inactiveTime(float) const", asMETHOD(ParticleEmitter, SetInactiveTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_inactiveTime() const", asMETHOD(ParticleEmitter, GetInactiveTime), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_timeToLive(float)", asMETHOD(ParticleEmitter, SetTimeToLive), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minTimeToLive(float)", asMETHOD(ParticleEmitter, SetMinTimeToLive), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxTimeToLive(float)", asMETHOD(ParticleEmitter, SetMaxTimeToLive), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_minTimeToLive() const", asMETHOD(ParticleEmitter, GetMinTimeToLive), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_maxTimeToLive() const", asMETHOD(ParticleEmitter, GetMaxTimeToLive), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_particleSize(const Vector2&in)", asMETHOD(ParticleEmitter, SetParticleSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minParticleSize(const Vector2&in)", asMETHOD(ParticleEmitter, SetMinParticleSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxParticleSize(const Vector2&in)", asMETHOD(ParticleEmitter, SetMaxParticleSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector2& get_minParticleSize() const", asMETHOD(ParticleEmitter, GetMinParticleSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector3& get_maxParticleSize() const", asMETHOD(ParticleEmitter, GetMaxParticleSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minDirection(const Vector3&in)", asMETHOD(ParticleEmitter, SetMinDirection), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector3& get_minDirection() const", asMETHOD(ParticleEmitter, GetMinDirection), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxDirection(const Vector3&in)", asMETHOD(ParticleEmitter, SetMaxDirection), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector3& get_maxDirection() const", asMETHOD(ParticleEmitter, GetMaxDirection), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_velocity(float)", asMETHOD(ParticleEmitter, SetVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minVelocity(float)", asMETHOD(ParticleEmitter, SetMinVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxVelocity(float)", asMETHOD(ParticleEmitter, SetMaxVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_minVelocity() const", asMETHOD(ParticleEmitter, GetMinVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_maxVelocity() const", asMETHOD(ParticleEmitter, GetMaxVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_rotation(float)", asMETHOD(ParticleEmitter, SetRotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minRotation(float)", asMETHOD(ParticleEmitter, SetMinRotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxRotation(float)", asMETHOD(ParticleEmitter, SetMaxRotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_minRotation() const", asMETHOD(ParticleEmitter, GetMinRotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_maxRotation() const", asMETHOD(ParticleEmitter, GetMaxRotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_rotationSpeed(float)", asMETHOD(ParticleEmitter, SetRotationSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_minRotationSpeed(float)", asMETHOD(ParticleEmitter, SetMinRotationSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_maxRotationSpeed(float)", asMETHOD(ParticleEmitter, SetMaxRotationSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_minRotationSpeed() const", asMETHOD(ParticleEmitter, GetMinRotationSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_maxRotationSpeed() const", asMETHOD(ParticleEmitter, GetMaxRotationSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_constantForce(const Vector3&in)", asMETHOD(ParticleEmitter, SetConstantForce), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "const Vector3& get_constantForce() const", asMETHOD(ParticleEmitter, GetConstantForce), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_dampingForce(float)", asMETHOD(ParticleEmitter, SetDampingForce), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_dampingForce() const", asMETHOD(ParticleEmitter, GetDampingForce), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_sizeAdd(float)", asMETHOD(ParticleEmitter, SetSizeAdd), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_sizeAdd() const", asMETHOD(ParticleEmitter, GetSizeAdd), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_sizeMul(float)", asMETHOD(ParticleEmitter, SetSizeMul), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "float get_sizeMul() const", asMETHOD(ParticleEmitter, GetSizeMul), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "ColorFrame@+ get_colors(uint)", asMETHODPR(ParticleEmitter, GetColor, (unsigned), ColorFrame*), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_numColors(uint)", asMETHOD(ParticleEmitter, SetNumColors), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "uint get_numColors() const", asMETHOD(ParticleEmitter, GetNumColors), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "TextureFrame@+ get_textureFrames(uint)", asMETHODPR(ParticleEmitter, GetTextureFrame, (unsigned), TextureFrame*), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "void set_numTextureFrames(uint)", asMETHOD(ParticleEmitter, SetNumTextureFrames), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "uint get_numTextureFrames() const", asMETHOD(ParticleEmitter, GetNumTextureFrames), asCALL_THISCALL);
    engine->RegisterObjectMethod("ParticleEmitter", "Zone@+ get_zone() const", asMETHOD(ParticleEmitter, GetZone), asCALL_THISCALL);
}

static void RegisterCustomGeometry(asIScriptEngine* engine)
{
    engine->RegisterEnum("PrimitiveType");
    engine->RegisterEnumValue("PrimitiveType", "TRIANGLE_LIST", TRIANGLE_LIST);
    engine->RegisterEnumValue("PrimitiveType", "LINE_LIST", LINE_LIST);
    
    RegisterDrawable<CustomGeometry>(engine, "CustomGeometry");
    engine->RegisterObjectMethod("CustomGeometry", "void Clear()", asMETHOD(CustomGeometry, Clear), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void BeginGeometry(uint, PrimitiveType)", asMETHOD(CustomGeometry, BeginGeometry), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void DefineVertex(const Vector3&in)", asMETHOD(CustomGeometry, DefineVertex), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void DefineNormal(const Vector3&in)", asMETHOD(CustomGeometry, DefineNormal), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void DefineColor(const Color&in)", asMETHOD(CustomGeometry, DefineColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void DefineTexCoord(const Vector2&in)", asMETHOD(CustomGeometry, DefineTexCoord), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void DefineTangent(const Vector4&in)", asMETHOD(CustomGeometry, DefineTangent), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void Commit()", asMETHOD(CustomGeometry, Commit), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void set_material(Material@+)", asMETHODPR(CustomGeometry, SetMaterial, (Material*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "bool set_materials(uint, Material@+)", asMETHODPR(CustomGeometry, SetMaterial, (unsigned, Material*), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "Material@+ get_materials(uint) const", asMETHOD(CustomGeometry, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "void set_numGeometries(uint)", asMETHOD(CustomGeometry, SetNumGeometries), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "uint get_numGeometries() const", asMETHOD(CustomGeometry, GetNumGeometries), asCALL_THISCALL);
    engine->RegisterObjectMethod("CustomGeometry", "Zone@+ get_zone() const", asMETHOD(CustomGeometry, GetZone), asCALL_THISCALL);
}

static void RegisterDecalSet(asIScriptEngine* engine)
{
    RegisterDrawable<DecalSet>(engine, "DecalSet");
    engine->RegisterObjectMethod("DecalSet", "bool AddDecal(Drawable@+, const Vector3&in, const Quaternion&in, float, float, float, const Vector2&in, const Vector2&in, float timeToLive = 0.0, float normalCutoff = 0.1, uint subGeometry = 0xffffffff)", asMETHOD(DecalSet, AddDecal), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "void RemoveDecals(uint)", asMETHOD(DecalSet, RemoveDecals), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "void RemoveAllDecals()", asMETHOD(DecalSet, RemoveAllDecals), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "void set_material(Material@+)", asMETHOD(DecalSet, SetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "Material@+ get_material() const", asMETHOD(DecalSet, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "uint get_numDecals() const", asMETHOD(DecalSet, GetNumDecals), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "uint get_numVertices() const", asMETHOD(DecalSet, GetNumVertices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "uint get_numIndices() const", asMETHOD(DecalSet, GetNumVertices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "void set_maxVertices(uint)", asMETHOD(DecalSet, SetMaxVertices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "uint get_maxVertices() const", asMETHOD(DecalSet, GetMaxVertices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "void set_maxIndices(uint)", asMETHOD(DecalSet, SetMaxIndices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "uint get_maxIndices() const", asMETHOD(DecalSet, GetMaxIndices), asCALL_THISCALL);
    engine->RegisterObjectMethod("DecalSet", "Zone@+ get_zone() const", asMETHOD(DecalSet, GetZone), asCALL_THISCALL);
}

static void RegisterTerrain(asIScriptEngine* engine)
{
    RegisterDrawable<TerrainPatch>(engine, "TerrainPatch");
    RegisterComponent<Terrain>(engine, "Terrain");
    engine->RegisterObjectMethod("Terrain", "float GetHeight(const Vector3&in) const", asMETHOD(Terrain, GetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "Vector3 GetNormal(const Vector3&in) const", asMETHOD(Terrain, GetNormal), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "TerrainPatch@+ GetPatch(int, int) const", asMETHODPR(Terrain, GetPatch, (int, int) const, TerrainPatch*), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_material(Material@+)", asMETHOD(Terrain, SetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "Material@+ get_material() const", asMETHOD(Terrain, GetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_smoothing(bool)", asMETHOD(Terrain, SetSmoothing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "bool get_smoothing() const", asMETHOD(Terrain, GetSmoothing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_heightMap(Image@+)", asMETHOD(Terrain, SetHeightMap), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "Image@+ get_heightMap() const", asMETHOD(Terrain, GetHeightMap), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_patchSize(int)", asMETHOD(Terrain, SetPatchSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "int get_patchSize() const", asMETHOD(Terrain, GetPatchSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_spacing(const Vector3&in)", asMETHOD(Terrain, SetSpacing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "const Vector3& get_spacing() const", asMETHOD(Terrain, GetSpacing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "const IntVector2& get_numVertices() const", asMETHOD(Terrain, GetNumVertices), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "const IntVector2& get_numPatches() const", asMETHOD(Terrain, GetNumPatches), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "TerrainPatch@+ get_patches(uint) const", asMETHODPR(Terrain, GetPatch, (unsigned) const, TerrainPatch*), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_castShadows(bool)", asMETHOD(Terrain, SetCastShadows), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "bool get_castShadows() const", asMETHOD(Terrain, GetCastShadows), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_occluder(bool)", asMETHOD(Terrain, SetOccluder), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "bool get_occluder() const", asMETHOD(Terrain, IsOccluder), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_occludee(bool)", asMETHOD(Terrain, SetOccludee), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "bool get_occludee() const", asMETHOD(Terrain, IsOccludee), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_drawDistance(float)", asMETHOD(Terrain, SetDrawDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "float get_drawDistance() const", asMETHOD(Terrain, GetDrawDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_shadowDistance(float)", asMETHOD(Terrain, SetShadowDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "float get_shadowDistance() const", asMETHOD(Terrain, GetShadowDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_lodBias(float)", asMETHOD(Terrain, SetLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "float get_lodBias() const", asMETHOD(Terrain, GetLodBias), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_viewMask(uint)", asMETHOD(Terrain, SetViewMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "uint get_viewMask() const", asMETHOD(Terrain, GetViewMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_lightMask(uint)", asMETHOD(Terrain, SetLightMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "uint get_lightMask() const", asMETHOD(Terrain, GetLightMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_shadowMask(uint)", asMETHOD(Terrain, SetShadowMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "uint get_shadowMask() const", asMETHOD(Terrain, GetShadowMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_zoneMask(uint)", asMETHOD(Terrain, SetZoneMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "uint get_zoneMask() const", asMETHOD(Terrain, GetZoneMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "void set_maxLights(uint)", asMETHOD(Terrain, SetMaxLights), asCALL_THISCALL);
    engine->RegisterObjectMethod("Terrain", "uint get_maxLights() const", asMETHOD(Terrain, GetMaxLights), asCALL_THISCALL);
}


static CScriptArray* GraphicsGetResolutions(Graphics* ptr)
{
    return VectorToArray<IntVector2>(ptr->GetResolutions(), "Array<IntVector2>");
}

static CScriptArray* GraphicsGetMultiSampleLevels(Graphics* ptr)
{
    return VectorToArray<int>(ptr->GetMultiSampleLevels(), "Array<int>");
}

static void GraphicsPrecacheShaders(File* file, Graphics* ptr)
{
    if (file)
        ptr->PrecacheShaders(*file);
}

static Graphics* GetGraphics()
{
    return GetScriptContext()->GetSubsystem<Graphics>();
}

static void RegisterGraphics(asIScriptEngine* engine)
{
    RegisterObject<Graphics>(engine, "Graphics");
    engine->RegisterObjectMethod("Graphics", "bool SetMode(int, int, bool, bool, bool, bool, bool, int)", asMETHODPR(Graphics, SetMode, (int, int, bool, bool, bool, bool, bool, int), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool SetMode(int, int)", asMETHODPR(Graphics, SetMode, (int, int), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void SetWindowPosition(int, int)", asMETHODPR(Graphics, SetWindowPosition, (int, int), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool ToggleFullscreen()", asMETHOD(Graphics, ToggleFullscreen), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void Maximize()", asMETHOD(Graphics, Maximize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void Minimize()", asMETHOD(Graphics, Minimize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void Close()", asMETHOD(Graphics, Close), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool TakeScreenShot(Image@+)", asMETHOD(Graphics, TakeScreenShot), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void BeginDumpShaders(const String&in)", asMETHOD(Graphics, BeginDumpShaders), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void EndDumpShaders()", asMETHOD(Graphics, EndDumpShaders), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void PrecacheShaders(File@+)", asFUNCTION(GraphicsPrecacheShaders), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Graphics", "void set_windowTitle(const String&in)", asMETHOD(Graphics, SetWindowTitle), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "const String& get_windowTitle() const", asMETHOD(Graphics, GetWindowTitle), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_windowIcon(Image@+)", asMETHOD(Graphics, SetWindowIcon), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_windowPosition(const IntVector2&in)", asMETHODPR(Graphics, SetWindowPosition, (const IntVector2&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "IntVector2 get_windowPosition() const", asMETHOD(Graphics, GetWindowPosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_sRGB(bool)", asMETHOD(Graphics, SetSRGB), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_sRGB() const", asMETHOD(Graphics, GetSRGB), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_flushGPU(bool)", asMETHOD(Graphics, SetFlushGPU), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_flushGPU() const", asMETHOD(Graphics, GetFlushGPU), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_orientations(const String&in)", asMETHOD(Graphics, SetOrientations), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "const String& get_orientations() const", asMETHOD(Graphics, GetOrientations), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "int get_width() const", asMETHOD(Graphics, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "int get_height() const", asMETHOD(Graphics, GetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "int get_multiSample() const", asMETHOD(Graphics, GetMultiSample), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_fullscreen() const", asMETHOD(Graphics, GetFullscreen), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_resizable() const", asMETHOD(Graphics, GetResizable), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_borderless() const", asMETHOD(Graphics, GetBorderless), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_vsync() const", asMETHOD(Graphics, GetVSync), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_tripleBuffer() const", asMETHOD(Graphics, GetTripleBuffer), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_initialized() const", asMETHOD(Graphics, IsInitialized), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_deviceLost() const", asMETHOD(Graphics, IsDeviceLost), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "uint get_numPrimitives() const", asMETHOD(Graphics, GetNumPrimitives), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "uint get_numBatches() const", asMETHOD(Graphics, GetNumBatches), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_sm3Support() const", asMETHOD(Graphics, GetSM3Support), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_instancingSupport() const", asMETHOD(Graphics, GetInstancingSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_lightPrepassSupport() const", asMETHOD(Graphics, GetLightPrepassSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_deferredSupport() const", asMETHOD(Graphics, GetDeferredSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_hardwareShadowSupport() const", asMETHOD(Graphics, GetHardwareShadowSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_sRGBSupport() const", asMETHOD(Graphics, GetSRGBSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_sRGBWriteSupport() const", asMETHOD(Graphics, GetSRGBWriteSupport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "void set_forceSM2(bool)", asMETHOD(Graphics, SetForceSM2), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "bool get_forceSM2() const", asMETHOD(Graphics, GetForceSM2), asCALL_THISCALL);
    engine->RegisterObjectMethod("Graphics", "Array<IntVector2>@ get_resolutions() const", asFUNCTION(GraphicsGetResolutions), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Graphics", "Array<int>@ get_multiSampleLevels() const", asFUNCTION(GraphicsGetMultiSampleLevels), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Graphics", "IntVector2 get_desktopResolution() const", asMETHOD(Graphics, GetDesktopResolution), asCALL_THISCALL);
    engine->RegisterGlobalFunction("Graphics@+ get_graphics()", asFUNCTION(GetGraphics), asCALL_CDECL);
}

static Renderer* GetRenderer()
{
    return GetScriptContext()->GetSubsystem<Renderer>();
}

static void RegisterRenderer(asIScriptEngine* engine)
{
    engine->RegisterGlobalProperty("const int QUALITY_LOW", (void*)&QUALITY_LOW);
    engine->RegisterGlobalProperty("const int QUALITY_MEDIUM", (void*)&QUALITY_MEDIUM);
    engine->RegisterGlobalProperty("const int QUALITY_HIGH", (void*)&QUALITY_HIGH);
    engine->RegisterGlobalProperty("const int QUALITY_MAX", (void*)&QUALITY_MAX);
    
    engine->RegisterGlobalProperty("const int SHADOWQUALITY_LOW_16BIT", (void*)&SHADOWQUALITY_LOW_16BIT);
    engine->RegisterGlobalProperty("const int SHADOWQUALITY_LOW_24BIT", (void*)&SHADOWQUALITY_LOW_24BIT);
    engine->RegisterGlobalProperty("const int SHADOWQUALITY_HIGH_16BIT", (void*)&SHADOWQUALITY_HIGH_16BIT);
    engine->RegisterGlobalProperty("const int SHADOWQUALITY_HIGH_24BIT", (void*)&SHADOWQUALITY_HIGH_24BIT);
    
    RegisterObject<Renderer>(engine, "Renderer");
    engine->RegisterObjectMethod("Renderer", "void DrawDebugGeometry(bool) const", asMETHOD(Renderer, DrawDebugGeometry), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void ReloadShaders() const", asMETHOD(Renderer, ReloadShaders), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_numViewports(uint)", asMETHOD(Renderer, SetNumViewports), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numViewports() const", asMETHOD(Renderer, GetNumViewports), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_viewports(uint, Viewport@+)", asMETHOD(Renderer, SetViewport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "Viewport@+ get_viewports(uint) const", asMETHOD(Renderer, GetViewport), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void SetDefaultRenderPath(XMLFile@+)", asMETHODPR(Renderer, SetDefaultRenderPath, (XMLFile*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_defaultRenderPath(RenderPath@+)", asMETHODPR(Renderer, SetDefaultRenderPath, (RenderPath*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "RenderPath@+ get_defaultRenderPath() const", asMETHOD(Renderer, GetDefaultRenderPath), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "Zone@+ get_defaultZone() const", asMETHOD(Renderer, GetDefaultZone), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "Material@+ get_defaultMaterial() const", asMETHOD(Renderer, GetDefaultMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "Material@+ get_defaultLightRamp() const", asMETHOD(Renderer, GetDefaultLightRamp), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "Material@+ get_defaultLightSpot() const", asMETHOD(Renderer, GetDefaultLightSpot), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_hdrRendering(bool)", asMETHOD(Renderer, SetHDRRendering), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "bool get_hdrRendering() const", asMETHOD(Renderer, GetHDRRendering), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_specularLighting(bool)", asMETHOD(Renderer, SetSpecularLighting), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "bool get_specularLighting() const", asMETHOD(Renderer, GetSpecularLighting), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_textureAnisotropy(int)", asMETHOD(Renderer, SetTextureAnisotropy), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_textureAnisotropy() const", asMETHOD(Renderer, GetTextureAnisotropy), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_textureFilterMode(TextureFilterMode)", asMETHOD(Renderer, SetTextureFilterMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "TextureFilterMode get_textureFilterMode() const", asMETHOD(Renderer, GetTextureFilterMode), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_textureQuality(int)", asMETHOD(Renderer, SetTextureQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_textureQuality() const", asMETHOD(Renderer, GetTextureQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_materialQuality(int)", asMETHOD(Renderer, SetMaterialQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_materialQuality() const", asMETHOD(Renderer, GetMaterialQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_drawShadows(bool)", asMETHOD(Renderer, SetDrawShadows), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "bool get_drawShadows() const", asMETHOD(Renderer, GetDrawShadows), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_shadowMapSize(int)", asMETHOD(Renderer, SetShadowMapSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_shadowMapSize() const", asMETHOD(Renderer, GetShadowMapSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_shadowQuality(int)", asMETHOD(Renderer, SetShadowQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_shadowQuality() const", asMETHOD(Renderer, GetShadowQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_maxShadowMaps(int)", asMETHOD(Renderer, SetMaxShadowMaps), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_maxShadowMaps() const", asMETHOD(Renderer, GetMaxShadowMaps), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_reuseShadowMaps(bool)", asMETHOD(Renderer, SetReuseShadowMaps), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "bool get_reuseShadowMaps() const", asMETHOD(Renderer, GetReuseShadowMaps), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_dynamicInstancing(bool)", asMETHOD(Renderer, SetDynamicInstancing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "bool get_dynamicInstancing() const", asMETHOD(Renderer, GetDynamicInstancing), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_minInstances(int)", asMETHOD(Renderer, SetMinInstances), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_minInstances() const", asMETHOD(Renderer, GetMinInstances), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_maxInstanceTriangles(int)", asMETHOD(Renderer, SetMaxInstanceTriangles), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_maxInstanceTriangles() const", asMETHOD(Renderer, GetMaxInstanceTriangles), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_maxSortedInstances(int)", asMETHOD(Renderer, SetMaxSortedInstances), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_maxSortedInstances() const", asMETHOD(Renderer, GetMaxSortedInstances), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_maxOccluderTriangles(int)", asMETHOD(Renderer, SetMaxOccluderTriangles), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_maxOccluderTriangles() const", asMETHOD(Renderer, GetMaxOccluderTriangles), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_occlusionBufferSize(int)", asMETHOD(Renderer, SetOcclusionBufferSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "int get_occlusionBufferSize() const", asMETHOD(Renderer, GetOcclusionBufferSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_occluderSizeThreshold(float)", asMETHOD(Renderer, SetOccluderSizeThreshold), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "float get_occluderSizeThreshold() const", asMETHOD(Renderer, GetOccluderSizeThreshold), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_mobileShadowBiasMul(float)", asMETHOD(Renderer, SetMobileShadowBiasMul), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "float get_mobileShadowBiasMul() const", asMETHOD(Renderer, GetMobileShadowBiasMul), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "void set_mobileShadowBiasAdd(float)", asMETHOD(Renderer, SetMobileShadowBiasAdd), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "float get_mobileShadowBiasAdd() const", asMETHOD(Renderer, GetMobileShadowBiasAdd), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numPrimitives() const", asMETHOD(Renderer, GetNumPrimitives), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numBatches() const", asMETHOD(Renderer, GetNumBatches), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numViews() const", asMETHOD(Renderer, GetNumViews), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numGeometries(bool) const", asMETHOD(Renderer, GetNumGeometries), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numLights(bool) const", asMETHOD(Renderer, GetNumLights), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numShadowMaps(bool) const", asMETHOD(Renderer, GetNumShadowMaps), asCALL_THISCALL);
    engine->RegisterObjectMethod("Renderer", "uint get_numOccluders(bool) const", asMETHOD(Renderer, GetNumOccluders), asCALL_THISCALL);
    engine->RegisterGlobalFunction("Renderer@+ get_renderer()", asFUNCTION(GetRenderer), asCALL_CDECL);
}

static DebugRenderer* GetDebugRenderer()
{
    Scene* scene = GetScriptContextScene();
    if (scene)
        return scene->GetComponent<DebugRenderer>();
    else
        return 0;
}

static DebugRenderer* SceneGetDebugRenderer(Scene* ptr)
{
    return ptr->GetComponent<DebugRenderer>();
}

static void RegisterDebugRenderer(asIScriptEngine* engine)
{
    engine->RegisterObjectMethod("DebugRenderer", "void AddLine(const Vector3&in, const Vector3&in, const Color&in, bool depthTest = true)", asMETHODPR(DebugRenderer, AddLine, (const Vector3&, const Vector3&, const Color&, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddTriangle(const Vector3&in, const Vector3&in, const Vector3&in, const Color&in, bool depthTest = true)", asMETHODPR(DebugRenderer, AddTriangle, (const Vector3&, const Vector3&, const Vector3&, const Color&, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddNode(Node@+, float scale = 1.0, bool depthTest = true)", asMETHOD(DebugRenderer, AddNode), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddBoundingBox(const BoundingBox&in, const Color&in, bool depthTest = true)", asMETHODPR(DebugRenderer, AddBoundingBox, (const BoundingBox&, const Color&, bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddFrustum(const Frustum&in, const Color&in, bool depthTest = true)", asMETHOD(DebugRenderer, AddFrustum), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddPolyhedron(const Polyhedron&in, const Color&in, bool depthTest = true)", asMETHOD(DebugRenderer, AddPolyhedron), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddSphere(const Sphere&in, const Color&in, bool depthTest = true)", asMETHOD(DebugRenderer, AddSphere), asCALL_THISCALL);
    engine->RegisterObjectMethod("DebugRenderer", "void AddSkeleton(Skeleton@+, const Color&in, bool depthTest = true)", asMETHOD(DebugRenderer, AddSkeleton), asCALL_THISCALL);
    engine->RegisterObjectMethod("Scene", "DebugRenderer@+ get_debugRenderer() const", asFUNCTION(SceneGetDebugRenderer), asCALL_CDECL_OBJLAST);
    engine->RegisterGlobalFunction("DebugRenderer@+ get_debugRenderer()", asFUNCTION(GetDebugRenderer), asCALL_CDECL);
}

static void ConstructRayQueryResult(RayQueryResult* ptr)
{
    new(ptr) RayQueryResult();
    ptr->position_ = Vector3::ZERO;
    ptr->normal_ = Vector3::ZERO;
    ptr->distance_ = 0.0f;
    ptr->subObject_ = 0;
}

static Drawable* RayQueryResultGetDrawable(RayQueryResult* ptr)
{
    return ptr->drawable_;
}

static Node* RayQueryResultGetNode(RayQueryResult* ptr)
{
    return ptr->node_;
}

static CScriptArray* OctreeRaycast(const Ray& ray, RayQueryLevel level, float maxDistance, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<RayQueryResult> result;
    RayOctreeQuery query(result, ray, level, maxDistance, drawableFlags, viewMask);
    ptr->Raycast(query);
    return VectorToArray<RayQueryResult>(result, "Array<RayQueryResult>");
}

static RayQueryResult OctreeRaycastSingle(const Ray& ray, RayQueryLevel level, float maxDistance, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<RayQueryResult> result;
    RayOctreeQuery query(result, ray, level, maxDistance, drawableFlags, viewMask);
    ptr->RaycastSingle(query);
    if (!query.result_.Empty())
        return query.result_[0];
    else
    {
        RayQueryResult empty;
        empty.position_ = Vector3::ZERO;
        empty.normal_ = Vector3::ZERO;
        empty.distance_ = M_INFINITY;
        empty.subObject_ = 0;
        return empty;
    }
}

static CScriptArray* OctreeGetDrawablesPoint(const Vector3& point, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<Drawable*> result;
    PointOctreeQuery query(result, point, drawableFlags, viewMask);
    ptr->GetDrawables(query);
    return VectorToHandleArray<Drawable>(result, "Array<Drawable@>");
}

static CScriptArray* OctreeGetDrawablesBox(const BoundingBox& box, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<Drawable*> result;
    BoxOctreeQuery query(result, box, drawableFlags, viewMask);
    ptr->GetDrawables(query);
    return VectorToHandleArray<Drawable>(result, "Array<Drawable@>");
}

static CScriptArray* OctreeGetDrawablesFrustum(const Frustum& frustum, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<Drawable*> result;
    FrustumOctreeQuery query(result, frustum, drawableFlags, viewMask);
    ptr->GetDrawables(query);
    return VectorToHandleArray<Drawable>(result, "Array<Node@>");
}

static CScriptArray* OctreeGetDrawablesSphere(const Sphere& sphere, unsigned char drawableFlags, unsigned viewMask, Octree* ptr)
{
    PODVector<Drawable*> result;
    SphereOctreeQuery query(result, sphere, drawableFlags, viewMask);
    ptr->GetDrawables(query);
    return VectorToHandleArray<Drawable>(result, "Array<Node@>");
}

static Octree* SceneGetOctree(Scene* ptr)
{
    return ptr->GetComponent<Octree>();
}

static Octree* GetOctree()
{
    Scene* scene = GetScriptContextScene();
    return scene ? scene->GetComponent<Octree>() : 0;
}

static void RegisterOctree(asIScriptEngine* engine)
{
    engine->RegisterEnum("RayQueryLevel");
    engine->RegisterEnumValue("RayQueryLevel", "RAY_AABB", RAY_AABB);
    engine->RegisterEnumValue("RayQueryLevel", "RAY_OBB", RAY_OBB);
    engine->RegisterEnumValue("RayQueryLevel", "RAY_TRIANGLE", RAY_TRIANGLE);
    
    engine->RegisterObjectType("RayQueryResult", sizeof(RayQueryResult), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
    engine->RegisterObjectBehaviour("RayQueryResult", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructRayQueryResult), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("RayQueryResult", "Vector3 position", offsetof(RayQueryResult, position_));
    engine->RegisterObjectProperty("RayQueryResult", "Vector3 normal", offsetof(RayQueryResult, normal_));
    engine->RegisterObjectProperty("RayQueryResult", "float distance", offsetof(RayQueryResult, distance_));
    engine->RegisterObjectMethod("RayQueryResult", "Drawable@+ get_drawable() const", asFUNCTION(RayQueryResultGetDrawable), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("RayQueryResult", "Node@+ get_node() const", asFUNCTION(RayQueryResultGetNode), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty("RayQueryResult", "uint subObject", offsetof(RayQueryResult, subObject_));
    
    RegisterComponent<Octree>(engine, "Octree");
    engine->RegisterObjectMethod("Octree", "void SetSize(const BoundingBox&in, uint)", asMETHOD(Octree, SetSize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Octree", "void DrawDebugGeometry(bool) const", asMETHODPR(Octree, DrawDebugGeometry, (bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Octree", "void AddManualDrawable(Drawable@+)", asMETHOD(Octree, AddManualDrawable), asCALL_THISCALL);
    engine->RegisterObjectMethod("Octree", "void RemoveManualDrawable(Drawable@+)", asMETHOD(Octree, RemoveManualDrawable), asCALL_THISCALL);
    engine->RegisterObjectMethod("Octree", "Array<RayQueryResult>@ Raycast(const Ray&in, RayQueryLevel level = RAY_TRIANGLE, float maxDistance = M_INFINITY, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK) const", asFUNCTION(OctreeRaycast), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "RayQueryResult RaycastSingle(const Ray&in, RayQueryLevel level = RAY_TRIANGLE, float maxDistance = M_INFINITY, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK) const", asFUNCTION(OctreeRaycastSingle), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "Array<Node@>@ GetDrawables(const Vector3&in, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK)", asFUNCTION(OctreeGetDrawablesPoint), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "Array<Node@>@ GetDrawables(const BoundingBox&in, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK)", asFUNCTION(OctreeGetDrawablesBox), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "Array<Node@>@ GetDrawables(const Frustum&in, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK)", asFUNCTION(OctreeGetDrawablesFrustum), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "Array<Node@>@ GetDrawables(const Sphere&in, uint8 drawableFlags = DRAWABLE_ANY, uint viewMask = DEFAULT_VIEWMASK)", asFUNCTION(OctreeGetDrawablesSphere), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("Octree", "const BoundingBox& get_worldBoundingBox() const", asMETHODPR(Octree, GetWorldBoundingBox, () const, const BoundingBox&), asCALL_THISCALL);
    engine->RegisterObjectMethod("Octree", "uint get_numLevels() const", asMETHOD(Octree, GetNumLevels), asCALL_THISCALL);
    engine->RegisterObjectMethod("Scene", "Octree@+ get_octree() const", asFUNCTION(SceneGetOctree), asCALL_CDECL_OBJLAST);
    engine->RegisterGlobalFunction("Octree@+ get_octree()", asFUNCTION(GetOctree), asCALL_CDECL);
}

void RegisterGraphicsAPI(asIScriptEngine* engine)
{
    RegisterSkeleton(engine);
    RegisterDebugRenderer(engine);
    RegisterCamera(engine);
    RegisterRenderPath(engine);
    RegisterTextures(engine);
    RegisterMaterial(engine);
    RegisterModel(engine);
    RegisterAnimation(engine);
    RegisterDrawable(engine);
    RegisterLight(engine);
    RegisterZone(engine);
    RegisterStaticModel(engine);
    RegisterStaticModelGroup(engine);
    RegisterSkybox(engine);
    RegisterAnimatedModel(engine);
    RegisterAnimationController(engine);
    RegisterBillboardSet(engine);
    RegisterParticleEmitter(engine);
    RegisterCustomGeometry(engine);
    RegisterDecalSet(engine);
    RegisterTerrain(engine);
    RegisterOctree(engine);
    RegisterGraphics(engine);
    RegisterRenderer(engine);
}

}
