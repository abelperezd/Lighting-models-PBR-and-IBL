#include "material.h"
#include "texture.h"
#include "application.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
} StandardMaterial::~StandardMaterial(){}

PBR_material::PBR_material(bool objectSel, float numShader, Light* light_pbr, Texture* albedo_text, Texture* metalness_text, Texture* normal_text, Texture* roughness_text, Texture* emissive_text, Shader* shad, Texture* textSB_0, Texture* textSB_1, Texture* textSB_2, Texture* textSB_3, Texture* textSB_4, Texture* textSB_5, Texture* LUT_texture)
{

	shader = shad;

	txt_albedo = albedo_text;
	txt_metalness = metalness_text;
	txt_normal = normal_text;
	txt_roughness = roughness_text;
	txt_emissive = emissive_text;

    E_text_0 = textSB_0;
	E_text_1 = textSB_1;
	E_text_2 = textSB_2;
	E_text_3 = textSB_3;
	E_text_4 = textSB_4;
	E_text_5 = textSB_5;

	LUTtexture = LUT_texture;

	light = light_pbr;

	shadSelect_ = numShader;
	objectSelect_ = objectSel;

}PBR_material::~PBR_material() {}

void PBR_material::setUniforms(Camera* camera, Matrix44 model) {

	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_texture_albedo", txt_albedo, 0);
	shader->setUniform("u_texture_normal", txt_normal, 1);
	shader->setUniform("u_texture_metalness", txt_metalness, 2);
	shader->setUniform("u_texture_roughness", txt_roughness, 3);
	shader->setUniform("u_texture_emissive", txt_emissive, 4);

	shader->setUniform("u_texture", E_text_0, 5);
	shader->setUniform("u_texture_prem_0", E_text_1, 6);
	shader->setUniform("u_texture_prem_1", E_text_2, 7);
	shader->setUniform("u_texture_prem_2", E_text_3, 8);
	shader->setUniform("u_texture_prem_3", E_text_4, 9);
	shader->setUniform("u_texture_prem_4", E_text_5, 10);

	shader->setUniform("u_LUT_texture", LUTtexture, 11);

	shader->setUniform("shadSelect", shadSelect_);
	shader->setUniform("objectSelect", objectSelect_);

	if (light)
		shader->setUniform("light_pos", light->position);
		shader->setUniform("light_color", light->color);
		shader->setUniform("light_intensity", light->intensity);
}




void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu(){}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
} WireframeMaterial::~WireframeMaterial(){}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

SBMaterial::SBMaterial(Shader* shad, Texture* texture_sb) {

	shader = shad;

	texture = new Texture();
	texture = texture_sb;


} SBMaterial::~SBMaterial() {}


