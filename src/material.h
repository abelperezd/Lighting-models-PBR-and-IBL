#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"

#include "light.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;

	Texture* txt_albedo = NULL;
	Texture* txt_metalness = NULL;
	Texture* txt_normal = NULL;
	Texture* txt_roughness = NULL;
	Texture* txt_emissive = NULL;

	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class PBR_material : public StandardMaterial
{
public:

	Light* light = NULL;

	Texture* E_text_0 = NULL;
	Texture* E_text_1 = NULL;
	Texture* E_text_2 = NULL;
	Texture* E_text_3 = NULL;
	Texture* E_text_4 = NULL;
	Texture* E_text_5 = NULL;

	Texture* LUTtexture = NULL;

	float shadSelect_;
	bool objectSelect_;

	PBR_material(bool objectSel, float numShader,  Light* light_pbr, Texture* albedo_text, Texture* metalness_text, Texture* normal_text, Texture* roughness_text, Texture* emissive_text, Shader* shad, Texture* textSB_0, Texture* textSB_1, Texture* textSB_2, Texture* textSB_3, Texture* textSB_4, Texture* textSB_5, Texture* LUT_texture);
	~PBR_material();

	void setUniforms(Camera* camera, Matrix44 model);
};

class SBMaterial : public StandardMaterial
{
public:


	SBMaterial(Shader* shad, Texture* texture_sb);
	~SBMaterial();

};



#endif


